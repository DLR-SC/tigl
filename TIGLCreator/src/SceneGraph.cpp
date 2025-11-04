/*
 * Copyright (C) 2025 German Aerospace Center (DLR/SC)
 *
 * Created: 2025 Ole Albers <ole.albers@dlr.de>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

 #include <SceneGraph.h>
 #include <TIGLCreatorContext.h>


SceneGraph::SceneGraph()
    : doc(nullptr)
    , drawableMap()
    , visibilityMap()
{
}

SceneGraph::~SceneGraph()
{
    clear();
}

void SceneGraph::setDocument(TIGLCreatorDocument* d)
{
    if (doc != d) {
        // Clear previous state when switching documents
        clear();
        doc = d;
    }
}

void SceneGraph::clear()
{
    for (auto &kv : visibilityMap) {
        kv.second.objects.clear();
    }
    visibilityMap.clear();
    drawableMap.clear();
    doc = nullptr;
}



bool SceneGraph::isDrawable(const std::string& uid) const
{
    if (uid.empty() || doc == nullptr) {
        return false;
    }
    
    // Check if uid is already in the map
    auto it = drawableMap.find(uid);
    if (it != drawableMap.end()) {
        return it->second;
    }
    
    bool drawable = false;
    try {
        tigl::ITiglGeometricComponent& comp = doc->GetConfiguration().GetUIDManager().GetGeometricComponent(uid);
        PNamedShape loft = comp.GetLoft();
        if (loft) {
        drawable = true; 
    }
}
catch (...) {

}

// write the uid in the map
drawableMap[uid] = drawable;
return drawable;
}

void SceneGraph::registerInteractiveObject(const std::string& uid, Handle(AIS_InteractiveObject) obj)
{
    if (uid.empty() || obj.IsNull()) return;
    auto &info = visibilityMap[uid];
    // avoid duplicates
    auto it = std::find(info.objects.begin(), info.objects.end(), obj);
    if (it == info.objects.end()) {
        info.objects.push_back(obj);
    }
}

bool SceneGraph::hasInteractiveObjects(const std::string& uid) const
{
    const auto it = visibilityMap.find(uid);
    if (it == visibilityMap.end()) return false;
    return !it->second.objects.empty();
}

std::vector<Handle(AIS_InteractiveObject)> SceneGraph::getInteractiveObjects(const std::string& uid) const
{
    auto it = visibilityMap.find(uid);
    if (it == visibilityMap.end()) return {};
    return it->second.objects;
}

void SceneGraph::clearInteractiveObjects()
{
    for (auto &it : visibilityMap) {
        auto &objs = it.second.objects;
        for (auto &obj : objs) {
            if (!obj.IsNull())
                obj.Nullify();
        }
        objs.clear();
    }
}

bool SceneGraph::hasDrawableChildren(cpcr::CPACSTreeItem* item) const

{
    if (!item) return false;

    for (auto child : item->getChildren()) {
        if (!child) continue;

        std::string cuid = child->getUid();
        if (!cuid.empty() && isDrawable(cuid))
            return true;
    }

    return false;
}

bool SceneGraph::getVisibility(const std::string& uid ) const
{
    const auto it = visibilityMap.find(uid);
    return (it == visibilityMap.end()) ? false : it->second.visible;
}

void SceneGraph::updateVisibility(const std::string& uid, bool visible)
{
    visibilityMap[uid].visible = visible;
}

void SceneGraph::reloadSceneGraph(TIGLCreatorContext* myScene)
{
    for (auto& uid : visibilityMap) {
        if (uid.second.visible) {
            if (hasInteractiveObjects(uid.first)) {
                auto objs = getInteractiveObjects(uid.first);
                for (auto& obj : objs) {
                    myScene->getContext()->Display(obj, Standard_False);
                }
            }
            else {
                doc->drawComponentByUID(QString::fromStdString(uid.first));
            }
        }
    }
}
bool SceneGraph::hasVisibilityStored(const std::string& uid) const
{
    return visibilityMap.find(uid) != visibilityMap.end();
}
