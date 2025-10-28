/*
 * Copyright (C) 2018 CFS Engineering
 *
 * Created: 2018 Malo Drougard <malo.drougard@protonmail.com>
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

#ifndef TIGL_MODIFICATORMANAGER_H
#define TIGL_MODIFICATORMANAGER_H

#include "TIGLCreatorDocument.h"
#include "CPACSTreeItem.h"
#include "ModificatorContainerWidget.h"
#include "modificators/NewConnectedElementDialog.h"
#include <QUndoStack>
#include "TIGLCreatorContext.h"
#include "CCPACSPositioning.h"
#include "ProfilesDBManager.h"
#include <QAbstractItemModel>
#include "CPACSTreeView.h"
#include "CPACSTree.h"
#include <AIS_InteractiveObject.hxx>
#include <SceneGraph.h>

class TIGLCreatorWindow;

/**
 * @brief Main class to articulate the modification interface and the tree view.
 *
 * This class holds three main components:
 * CCPACSConfiguration: that is the main tigl object to work with.
 * CPACSTreeWidget: that manages the tree interface.
 * ModificatorContainerWidget: that is the interface that holds the specialized modificator for a
 * specific tigl object.
 *
 * To work with this class, first we need to set the CCPACSConfiguration. The
 * CPACSTreeView will be updated and the correct tree will be build and
 * displayed. Once the CPACSConfiguration is set, it will wait for a
 * "newSelectedTreeItem" signal. If a "newSelectedTreeItem" signal is emitted, the
 * "dispatch" function will be called. The dispatch function will look at the
 * new selected element and if the element has an associated ModificatorWidget, it
 * will set this particular ModificatorWidget by a call of the modificatorContainerWidget.
 * The dispatch function will also call highlighting functions to show in the scene with
 * element can be currently edited.
 *
 * @author Malo Drougard
 *
 */
class ModificatorModel : public QAbstractItemModel
{
    Q_OBJECT

signals:
    void configurationEdited();
    void componentVisibilityChanged(const QString& uid, bool visible);

public slots:
    void dispatch(cpcr::CPACSTreeItem* item);
    void createUndoCommand();
    void resetTree();
    // function to update the profiles software db if the settings has changed
    void updateProfilesDB(QString newDBPath);

    // Highlighting functions
    void highlight(std::vector<tigl::CTiglSectionElement*> elements);
    void highlight(tigl::CCPACSPositioning &positioning, const tigl::CTiglTransformation& parentTransformation);
    void unHighlight();

    // delete wing without dialog
    void deleteWing(std::string const& uid);

    // delete fuselage without dialog
    void deleteFuselage(std::string const& uid);

    // delete section without dialog
    void deleteSection(cpcr::CPACSTreeItem* item);

    // add wing airfoil without dialog
    void addProfile(QString const& profileID);

    // Open dialog for deleting a section
    void onDeleteSectionRequested(Ui::ElementModificatorInterface& emi);

    // Opens dialog for adding a section
    void onAddSectionRequested(Ui::ElementModificatorInterface& emi);
    void onAddSectionRequested(CPACSTreeView::Where where, cpcr::CPACSTreeItem* item);

    // Opens dialog for adding a wing
    void onAddWingRequested();

    // Opens dialog for deleting a wing
    void onDeleteWingRequested();

    // Opens a dialog for adding a fuselage
    void onAddFuselageRequested();

    // Opens a dialog for deleting a fuselage
    void onDeleteFuselageRequested();

public:
    ModificatorModel(
            ModificatorContainerWidget* modificatorContainerWidget,
            TIGLCreatorContext* scene,
            QUndoStack* undoStack,
            QObject* parent = 0
    );

    void setCPACSConfiguration(TIGLCreatorDocument* newDoc);

    // Associate a SceneGraph instance with this model. The model does not own
    // the pointer; the caller (TIGLCreatorWindow) manages SceneGraph lifetime.
    void setSceneGraph(SceneGraph* sg) { sceneGraph = sg; }

    /**
     * @brief resets the currently loaded CCPACSConfiguration to the CPACS configuration in a string
     * 
     * This is used e.g. by Undo and Redo commands, where the current configuration needs to be 
     * reset to its previous state.
     * 
     * @param config a CPACS conform string
     */
    void updateCpacsConfigurationFromString(std::string const& config);

    /**
     * @brief gets the currently loaded CCPACSConfiguration as a CPACS conform string
     * 
     * @return std::string 
     */
    std::string getConfigurationAsString();

    /**
     * @brief writeCPACS writes the currently loaded CCPACSConfiguration to the underlying tixi handle.
     * 
     */
    void writeCPACS();

    // standardization functions
    // standardize the wing or the fuselage given by the uid and call createUndoCommand
    void standardize(QString uid, bool useSimpleDecomposition);
    // standardize the aircraft uid and call createUndoCommand
    void standardize(bool useSimpleDecomposition);

    // QAbstractItemModel interface

    QVariant data(const QModelIndex& index, int role) const override;

    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex()) const override;

    QModelIndex parent(const QModelIndex& index) const override;

    // count the number of child of a index
    int rowCount(const QModelIndex& parent = QModelIndex()) const override;

    // count the number of data a index hold
    int columnCount(const QModelIndex& parent = QModelIndex()) const override;

    Qt::ItemFlags flags(const QModelIndex &index) const override;

    // Return true if there is a valid root
    bool isValid() const;

    bool isVisible(const std::string& uid) const;

    void setVisibility(const std::string& uid, bool visible);

    void loadVisibilityFromSettings();

    void saveVisibilityToSettings();

    // Register interactive AIS objects with a UID so the model can manage
    // appearing/disappearing without querying external managers.
 

    bool setData(const QModelIndex& index, const QVariant& value, int role);

    QModelIndex getIdxForUID(std::string uid) const;

    std::string getUidForIdx(QModelIndex idx) const;

    cpcr::CPACSTreeItem* getItemFromSelection(const QItemSelection& newSelection);

    // Return the index of the first cpacs element that is of the "model" type
    QModelIndex getAircraftModelIndex() const;

    // return the item for the given index
    // empty index is considered as the root index!
    cpcr::CPACSTreeItem* getItem(QModelIndex index) const;

    // return a index for the item
    QModelIndex getIndex(cpcr::CPACSTreeItem* item, int column) const;

protected:

    inline bool configurationIsSet() const
    {
        return (doc != nullptr && doc->getCpacsHandle() > 0);
    }

private:

    /**
     * @brief resolve resolves the ElementModificatorInterface based on the uid
     * The ElementModificatorInterface can handle either wings or fuselages
     * @param uid of a wing or a fuselage
     * @return the interface
     */
    Ui::ElementModificatorInterface resolve(std::string const& uid) const;

    /**
     * @brief ModificatorModel::addSection adds a wing or fuselage section
     * @param element An interface class to handle both wing and fuselage sections
     * @param where enum, can be Before or After
     * @param startUID a reference section
     * @param sectionName the name of the new section
     * @param eta an eta value for the new section. Only applies for internal sections
     */
    void addSection(
            Ui::ElementModificatorInterface& element,
            NewConnectedElementDialog::Where where,
            std::string startUID,
            std::string sectionName,
            std::optional<double> eta
    );

    std::string sectionUidToElementUid(std::string const& uid) const;
    std::string elementUidToSectionUid(std::string const& uid) const;

    //convenience getters for some specific cpacs node
    cpcr::CPACSTreeItem *getWings() const;
    cpcr::CPACSTreeItem *getFuselages() const;
    cpcr::CPACSTreeItem *getAirfoils() const;
    cpcr::CPACSTreeItem *getFuselageProfiles() const;

    ModificatorContainerWidget* modificatorContainerWidget;

    // these need to be in sync at all times:
    cpcr::CPACSTree tree;
    TIGLCreatorDocument* doc;
    TIGLCreatorContext* scene;

    QUndoStack* myUndoStack;
    QList<Handle(AIS_InteractiveObject)> highligthteds;
    ProfilesDBManager profilesDB;
    SceneGraph* sceneGraph;

    // helper for hierarchical visibility
    // NOTE: recursive visibility was removed; visibility changes are applied only to the clicked item
    // and its immediate children (non-recursively).

    // Register/unregister interactive AIS objects with a UID so the model can manage
    // appearing/disappearing without querying external managers.
    // Note: these methods are declared in the public section above. Keep only the
    // non-duplicated helper declaration here.
    Qt::CheckState aggregateChildrenState(cpcr::CPACSTreeItem* item) const;

    // Return whether the given UID corresponds to a drawable geometric component.
    // Drawable here means the UID maps to an ITiglGeometricComponent that provides a loft
    // (PNamedShape) which can be displayed in the viewer.
    bool isDrawableUID(const std::string& uid) const;


    // Populate the drawableMap cache by iterating geometric components
    // in the current configuration. This avoids expensive calls to
    // GetLoft() during paint/data operations.
    void populateDrawableCache();


};

#endif // TIGL_MODIFICATORMANAGER_H
