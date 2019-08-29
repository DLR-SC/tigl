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


#include "TIGLViewerNewFileDialog.h"
#include "ui_TIGLViewerNewFileDialog.h"
#include "TIGLViewerSettings.h"
#include <QPushButton>
#include <QRegExp>
#include "CTiglLogging.h"
#include "TIGLViewerErrorDialog.h"

TIGLViewerNewFileDialog::TIGLViewerNewFileDialog(QWidget* parent)
    : QDialog(parent)
    , ui(new Ui::TIGLViewerNewFileDialog)
{
    ui->setupUi(this);
    populate();

    newCPACSFileName = "";
}

TIGLViewerNewFileDialog::~TIGLViewerNewFileDialog()
{
    delete ui;
}

void TIGLViewerNewFileDialog::populate()
{
    // get the settings for this application
    TIGLViewerSettings& settings = TIGLViewerSettings::Instance();
    // retrieve the files in the templates dir
    QRegExp filesFilter("^.*\\.(xml|cpacs|cpacs3)$", Qt::CaseInsensitive, QRegExp::RegExp);
    QStringList files = settings.templateDir().entryList(QDir::Files);
    files             = files.filter(filesFilter);
    // set the model and the view
    templateListModel.setStringList(files);
    ui->templatesListView->setModel(&templateListModel);
}

void TIGLViewerNewFileDialog::accept()
{

    QModelIndex index        = ui->templatesListView->currentIndex();
    QString selectedTemplate = index.data(Qt::DisplayRole).toString();

    TIGLViewerSettings& settings = TIGLViewerSettings::Instance();

    QString originalFile = settings.templateDir().absolutePath() + "/" + selectedTemplate;
    QString newFilePath  = originalFile + ".temp";
    int prefix           = 1;
    while (QFile::exists(newFilePath)) {
        newFilePath =
            settings.templateDir().absolutePath() + "/" + QString::number(prefix) + "_" + selectedTemplate + ".temp";
        prefix = prefix + 1;
    }

    // Copy the file
    if (QFile::copy(originalFile, newFilePath)) {
        newCPACSFileName = newFilePath;
        LOG(INFO) << "TIGLViewerNewFileDialog::templateIsSelected: new file " + newFilePath.toStdString() +
                         " created based on the template."
                  << std::endl;
        QDialog::accept();
    }
    else {
        QString errorMsg = "An error occurs during the creation of the file \"" + newFilePath +
                           "\". Make shure the application has the permission to write into \"" +
                           settings.templateDir().absolutePath() + "\"";

        LOG(WARNING) << "TIGLViewerNewFileDialog::templateIsSelected: " + errorMsg.toStdString() << std::endl;

        // Display the
        TIGLViewerErrorDialog dialog(this);
        dialog.setMessage(QString("<b>%1</b><br /><br />%2").arg("Template Error").arg(errorMsg));
        dialog.setWindowTitle("Error");
        dialog.exec();
        reject();
    }
}

QString TIGLViewerNewFileDialog::getNewFileName()
{
    return newCPACSFileName;
}
