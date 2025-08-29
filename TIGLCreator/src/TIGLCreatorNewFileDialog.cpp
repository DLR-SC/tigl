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


#include "TIGLCreatorNewFileDialog.h"
#include "ui_TIGLCreatorNewFileDialog.h"
#include "TIGLCreatorSettings.h"
#include <QPushButton>
#include <QRegExp>
#include "CTiglLogging.h"
#include "TIGLCreatorErrorDialog.h"

TIGLCreatorNewFileDialog::TIGLCreatorNewFileDialog(QWidget* parent)
    : QDialog(parent)
    , ui(new Ui::TIGLCreatorNewFileDialog)
{
    ui->setupUi(this);
    populate();

    newCPACSFileName = "";
}

TIGLCreatorNewFileDialog::~TIGLCreatorNewFileDialog()
{
    delete ui;
}

void TIGLCreatorNewFileDialog::populate()
{
    // get the settings for this application
    TIGLCreatorSettings& settings = TIGLCreatorSettings::Instance();
    // retrieve the files in the templates dir
    QRegExp filesFilter("^.*\\.(xml|cpacs|cpacs3)$", Qt::CaseInsensitive, QRegExp::RegExp);
    QStringList files = settings.templateDir().entryList(QDir::Files);
    files             = files.filter(filesFilter);
    // set the model and the view
    templateListModel.setStringList(files);
    ui->templatesListView->setModel(&templateListModel);
}

void TIGLCreatorNewFileDialog::accept()
{

    QModelIndex index        = ui->templatesListView->currentIndex();
    QString selectedTemplate = index.data(Qt::DisplayRole).toString();

    TIGLCreatorSettings& settings = TIGLCreatorSettings::Instance();

    QString originalFile = settings.templateDir().absolutePath() + "/" + selectedTemplate;

    if(QFile::exists(originalFile)) {
        newCPACSFileName = originalFile;
        QDialog::accept();
    }
    else {
        QString errorMsg = "An error occurs when reading the file \"" + originalFile + "\".";

        LOG(WARNING) << "TIGLCreatorNewFileDialog::templateIsSelected: " + errorMsg.toStdString() << std::endl;

        // Display the
        TIGLCreatorErrorDialog dialog(this);
        dialog.setMessage(QString("<b>%1</b><br /><br />%2").arg("Template Error").arg(errorMsg));
        dialog.setWindowTitle("Error");
        dialog.exec();
        reject();
    }
}

QString TIGLCreatorNewFileDialog::getNewFileName()
{
    return newCPACSFileName;
}
