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

#pragma once

#include <AIS_InteractiveObject.hxx>
#include <TIGLCreatorDocument.h>
#include <string>
#include <unordered_map>
#include <vector>
#include <CPACSTreeItem.h>

namespace cpcr { class CPACSTreeItem; }

class SceneGraph
{
public:
    // Default constructor
    SceneGraph();
    ~SceneGraph();

    // Associate the SceneGraph with a document. Call this when a configuration is opened/closed so
    // the SceneGraph lifetime is per-configuration.
    void setDocument(TIGLCreatorDocument* d);

    // Clear all internal caches. This is called when the active configuration
    // is closed to ensure no stale state remains.
    void clear();

    // Check if a component with the given uid is drawable and store the result in the drawable map
    bool isDrawable(const std::string& uid) const;

    // Check if a CPACSTreeItem has any drawable children
    bool hasDrawableChildren(cpcr::CPACSTreeItem* item) const;

    // Get the visibility of a component with the given uid
    bool getVisibility(const std::string& uid) const;
    // Update the visibility of a component in the map
    void updateVisibility(const std::string& uid, bool visible);
    // Check if visibility is stored for a given uid
    bool hasVisibilityStored(const std::string& uid) const;
    
    // Register an interactive object in the visibility map
    void registerInteractiveObject(const std::string& uid, Handle(AIS_InteractiveObject) obj);
    // Check if uid has interactive objects registered
    bool hasInteractiveObjects(const std::string& uid) const;
    // Get interactive objects for a given uid
    std::vector<Handle(AIS_InteractiveObject)> getInteractiveObjects(const std::string& uid) const;
    // Clear all registered interactive objects. Used when reloading documents.
    void clearInteractiveObjects();
    
    // Reload the scene graph. Used when reloading documents.
    void reloadSceneGraph(TIGLCreatorContext* myScene);
    
    // read(QString filename);
    // write(QString filename);
private:
    //std::unordered_map<std::string, VisibleMap> opened_files; // unter der Annahme, dass wir mehrere configurations gleichzeitig erlauben wollen
    // cameraPosition pos;
    // cameraLookatPosition lookat;
    // wireframe_settings settings;
    // std::vector<lightsources> lightsources;
    TIGLCreatorDocument* doc;

    mutable std::unordered_map<std::string, bool> drawableMap;

    struct VisibilityInfo {
        bool visible = true;
        std::vector<Handle(AIS_InteractiveObject)> objects;
    };

    std::unordered_map<std::string, VisibilityInfo> visibilityMap;
};