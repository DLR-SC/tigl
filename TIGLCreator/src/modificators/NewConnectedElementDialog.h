/*
 * Copyright (C) 2019 CFS Engineering
 *
 * Created: 2019 Malo Drougard <malo.drougard@protonmail.com>
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

#ifndef NEWCONNECTEDELEMENTDIALOG_H
#define NEWCONNECTEDELEMENTDIALOG_H

#include <QDialog>
#include <QString>
#include <optional>
#include "ElementModificatorInterface.h"
#include "CTiglUIDManager.h"

namespace Ui
{
class NewConnectedElementDialog;
}

class NewConnectedElementDialog : public QDialog
{
    Q_OBJECT

public:
    enum Where
    {
        Before,
        After
    };

    explicit NewConnectedElementDialog(QStringList connectedElements, tigl::CTiglUIDManager const& uid_mgr, QWidget* parent = nullptr);
    ~NewConnectedElementDialog();

    /**
     * Return the reference element uid to which the new connected element will be connected.
     * @return
     */
    QString getStartUID() const;
    void setStartUID(QString const& text);

    /**
     * Return if the new connected element should be created before or after the reference element.
     * @return
     */
    Where getWhere() const;
    void setWhere(Where where);

    std::optional<double> getEta() const;
    QString getSectionName() const;

private slots:
    void activate_eta();
    void updateDefaultName();

private:
    Ui::NewConnectedElementDialog* ui;
    std::function<QString(QString)> make_unique;
    bool between_sections;
};

#endif // NEWCONNECTEDELEMENTDIALOG_H
