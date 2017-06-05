/****************************************************************************
**
**   Copyright (C) 2017 P.L. Lucas
**
**
** LICENSE: BSD
** You may use this file under the terms of the BSD license as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of developers or companies in the above copyright and its 
**     Subsidiary(-ies) nor the names of its contributors may be used to 
**     endorse or promote products derived from this software without 
**     specific prior written permission.
**
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
**
****************************************************************************/

#include "filebrowser.h"
#include <QFileInfo>
#include <QDebug>
#include <QLineEdit>
#include <QStorageInfo>

ComboSignal::ComboSignal(QWidget* parent):QComboBox(parent)
{
}

void ComboSignal::hidePopup()
{
    emit menuHidden();
    QComboBox::hidePopup();
}
void ComboSignal::showPopup()
{
    emit menuPopped();
    QComboBox::showPopup();
}

FileBrowser::FileBrowser(QWidget * parent):QWidget(parent)
{
    	ui.setupUi(this);
    	pathComboBox = new ComboSignal(this);
    	ui.horizontalLayout->addWidget(pathComboBox);
    	pathComboBox->setEditable(true);
    	pathComboBox->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
    	pathComboBox->show();
    fileModel = new QFileSystemModel(this);
    fileModel->setRootPath(QDir::rootPath());
    //fileModel->sort(0, Qt::AscendingOrder);
    ui.fileView->setModel(fileModel);
    ui.fileView->setRootIndex(fileModel->index(QDir::currentPath()));
    ui.fileView->hideColumn(1);
    ui.fileView->hideColumn(2);
    ui.fileView->hideColumn(3);
    
    
    setRootIndex(fileModel->index(QDir::currentPath()));
    
    connect(pathComboBox->lineEdit(), SIGNAL(returnPressed()), this, SLOT(pathComboTextChanged()));
    connect(ui.fileView, SIGNAL(doubleClicked(const QModelIndex &)), this, SLOT(fileDoubleClicked(const QModelIndex &)));
    connect(ui.fileView, SIGNAL(clicked(const QModelIndex &)), this, SLOT(fileClicked(const QModelIndex &)));
    connect(ui.upButton, SIGNAL(clicked()), this, SLOT(up()));
    connect(fileModel, SIGNAL(directoryLoaded(const QString &)), this, SLOT(directoryLoaded(const QString &)));
    connect(pathComboBox, SIGNAL(activated(int)), this, SLOT(comboItemActivated(int)));
    connect(pathComboBox, SIGNAL(menuPopped()), this, SLOT(updateStorageDevices()));
}

void FileBrowser::fileDoubleClicked(const QModelIndex &index)
{
    if(!fileModel->isDir(index))
        emit fileActivated(fileModel->filePath(index));
}

void FileBrowser::fileClicked(const QModelIndex &index)
{
    //QFileInfo file(fileModel->filePath(index));
    //qDebug() << index.data().toString() << fileModel->filePath(index) << file.absoluteFilePath() << file.isDir();
    if(fileModel->isDir(index))
    {
        setRootIndex(index);
        //fileModel->sort(0, Qt::AscendingOrder	);
    }
}

void FileBrowser::up()
{
    QModelIndex index = ui.fileView->rootIndex();
    setRootIndex(index.parent());
}

void FileBrowser::setRootIndex(const QModelIndex &index)
{
    ui.fileView->setRootIndex(index);
    pathComboBox->setCurrentText(fileModel->filePath(index));

}

void FileBrowser::directoryLoaded(const QString &path)
{
    //fileModel->sort(1, Qt::AscendingOrder	);
    //qDebug() << "Ordenando";
}

void FileBrowser::pathComboTextChanged()
{
    ui.fileView->setRootIndex(fileModel->index(pathComboBox->currentText()));
}

QString FileBrowser::getPath()
{
    return pathComboBox->currentText();
}

void FileBrowser::setPath(QString path)
{
    setRootIndex(fileModel->index(path));
}

void FileBrowser::comboItemActivated(int index)
{
    setPath(pathComboBox->itemData(index).toString());
}

void FileBrowser::updateStorageDevices()
{
    pathComboBox->clear();
    QStorageInfo storageInfo;
    foreach(const QStorageInfo &storage, storageInfo.mountedVolumes()) {
        if(!storage.name().isEmpty())
            pathComboBox->addItem(storage.name(), QVariant(storage.rootPath()));
    }
    pathComboBox->insertItem(0, storageInfo.root().rootPath(), QVariant(storageInfo.root().rootPath()));
}