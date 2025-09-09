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

#ifndef TIGL_CPACSFILTERMODEL_H
#define TIGL_CPACSFILTERMODEL_H

#include <QSortFilterProxyModel>
#include "CPACSAbstractModel.h"
#include "CPACSTreeItem.h"

/**
 * This class acts as a proxy on CPACSAbstractModel.
 * Its goal is to allow the model to be filtered and to enable search on the model.
 * Basically this class contains two QRegExp.
 * One to control the default filtering on the model and one to control the search pattern.
 * This two regex are used by the filterAcceptsRow function to determine which row of the
 * model should be display.
 * The default filtering on the mode can be avoided by setting the expertView mode using
 * the setExpertView() function.
 * We can also enable or disable the match on uid using the enableMatchOnUID() function.
 *
 * @author Malo Drougard
 */
class CPACSFilterModel : public QSortFilterProxyModel
{

    Q_OBJECT

public slots:

    void setExpertView(bool value);

    /**
     * enable the match on uid for the search pattern
     * @param value
     */
    void enableMatchOnUID(bool value);

    void setSearchPattern(const QString newText);

public:
    explicit CPACSFilterModel(cpcr::CPACSTree* tree, QObject* parent = 0);

    // The following functions are used to access the none standard functions of the underlying model.

    inline bool isValid() const
    {
        return cpacsModel->isValid();
    };

    inline void disconnectInternalTree()
    {
        cpacsModel->disconnectInternalTree();
    };

    inline void resetInternalTree(cpcr::CPACSTree* tree)
    {
        cpacsModel->resetInternalTree(tree);
    };

    cpcr::CPACSTreeItem* getItemFromSelection(const QItemSelection& newSelection);

    /**
     * @return Return the index of the first cpacs element that is of the "model" type
     */
    QModelIndex getAircraftModelRoot();

    QModelIndex getIdxForUID(const QString& uid);

    QString getUidForIdx(QModelIndex idx);


protected:
    bool filterAcceptsRow(int sourceRow, const QModelIndex& sourceParent) const override;

private:
    CPACSAbstractModel* cpacsModel;
    QRegExp basicTreeRegExp;
    QRegExp searchPattern;
    bool matchingOnUID;
    bool expertView;
};

#endif //TIGL_CPACSFILTERMODEL_H
