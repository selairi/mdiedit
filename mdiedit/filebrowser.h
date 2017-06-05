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

#ifndef __FILE_BROWSER_H__
#define __FILE_BROWSER_H__

#include "ui_filebrowser.h"
#include <QFileSystemModel>
#include <QComboBox>

class ComboSignal : public QComboBox
{
    Q_OBJECT
public:
    ComboSignal(QWidget* parent=nullptr);
    void hidePopup();
    void showPopup();
signals:
    void menuPopped();
    void menuHidden();
};


class FileBrowser : public QWidget {
    Q_OBJECT

public:
    FileBrowser(QWidget * parent = 0);
    QString getPath();
    void setPath(QString path);

public slots:
    void fileDoubleClicked(const QModelIndex &index);
    void fileClicked(const QModelIndex &index);
    void up();
    void directoryLoaded(const QString &path);
    void pathComboTextChanged();
    void comboItemActivated(int index);
    void updateStorageDevices();  
    
signals:
    void fileActivated(QString file);
    
private:
    Ui::FileBrowser ui;
    QFileSystemModel *fileModel;
    void setRootIndex(const QModelIndex &index);
    ComboSignal *pathComboBox;
};

#endif
