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

#include "CPACSFilterModel.h"

CPACSFilterModel::CPACSFilterModel(cpcr::CPACSTree* tree, QObject* parent)
{
    basicTreeRegExp.setPatternSyntax(QRegExp::RegExp2);
    basicTreeRegExp.setCaseSensitivity(Qt::CaseInsensitive);
    basicTreeRegExp.setPattern("^(aircraft|rotorcraft|model|wings|wing|sections|section|positionings|positioning|fuselages|fuselage|rotors|rotor|rotorHub|rotorBladeAttachments|rotorBladeAttachment|rotorBlades|rotorBlade|rotorAirfoils|rotorAirfoil)$");

    searchPattern.setPatternSyntax(QRegExp::FixedString);
    searchPattern.setCaseSensitivity(Qt::CaseInsensitive);
    searchPattern.setPattern("");

    expertView = false;
    geometryNodes = false;
}

void CPACSFilterModel::setModel(ModificatorModel *model)
{
    cpacsModel = model;
    setSourceModel(model);
}

cpcr::CPACSTreeItem* CPACSFilterModel::getItemFromSelection(const QItemSelection& newSelection)
{
    return cpacsModel->getItemFromSelection(mapSelectionToSource(newSelection));
}

cpcr::CPACSTreeItem *CPACSFilterModel::getItem(QModelIndex index) const
{
    return cpacsModel->getItem(mapToSource(index));
}

bool CPACSFilterModel::filterAcceptsRow(int sourceRow, const QModelIndex& sourceParent) const
{

    QModelIndex typeIndex = sourceModel()->index(sourceRow, 1, sourceParent);
    QModelIndex uidIndex  = sourceModel()->index(sourceRow, 2, sourceParent);

    QModelIndex idx0 = sourceModel()->index(sourceRow, 0, sourceParent);

    // check if we should considerate this tree branch
    if ((expertView == false) && (sourceModel()->data(typeIndex).toString().contains(basicTreeRegExp) == false)) {
        return false;
    }

    if (geometryNodes) {
        if (idx0.isValid()) {
            if (sourceModel()->flags(idx0) & Qt::ItemIsUserCheckable) {
                return true;
            }

            // show parent nodes if any child matches
            int childNumber = sourceModel()->rowCount(idx0);
            for (int i = 0; i < childNumber; i++) {
                if (filterAcceptsRow(i, idx0)) {
                    return true;
                }
            }
        }
        return false;
    }

    if (searchPattern.isEmpty()) {
        return true;
    }
    // the case where search is activate
    else {
        // if the row itself match the pattern
        if (sourceModel()->data(typeIndex).toString().contains(searchPattern) ||
            (matchingOnUID && sourceModel()->data(uidIndex).toString().contains(searchPattern))) {
                return true;
        }
        // check if one of its child match the pattern (recursive call)
        else {
            int childNumber = sourceModel()->rowCount(typeIndex);
            for (int i = 0; i < childNumber; i++) {
                if (filterAcceptsRow(i, typeIndex)) {
                    return true;
                }
            }
            return false;
        }
    }
}

void CPACSFilterModel::setExpertView(bool value)
{
    expertView = value;
    invalidateFilter();
}

void CPACSFilterModel::setgeometryNodes(bool value)
{
    geometryNodes = value;
    invalidateFilter();
}

void CPACSFilterModel::setSearchPattern(const QString newText)
{
    searchPattern.setPattern(newText);
    invalidateFilter();
}

void CPACSFilterModel::enableMatchOnUID(bool value)
{
    matchingOnUID = value;
    invalidateFilter();
}

QModelIndex CPACSFilterModel::getAircraftModelRoot()
{
    return mapFromSource(cpacsModel->getAircraftModelIndex());
}

QModelIndex CPACSFilterModel::getIdxForUID(const QString& uid)
{
    QModelIndex sourceIdx = cpacsModel->getIdxForUID(uid.toStdString());
    QModelIndex idx       = mapFromSource(sourceIdx);
    return idx;
}

QString CPACSFilterModel::getUidForIdx(QModelIndex idx)
{
    return QString(cpacsModel->getUidForIdx(mapToSource(idx)).c_str());
}
