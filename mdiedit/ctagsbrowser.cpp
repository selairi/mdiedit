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

#include "ctagsbrowser.h"

#include <QProcess>
#include <QDebug>

static QTreeWidgetItem *_parse_ctag_ouput(QTreeWidget *treeWidget, const char *buff);

CTAGSBrowser::CTAGSBrowser(QWidget * parent, QStringList files):QDialog(parent) 
{
	ui.setupUi(this);
    
    QProcess *ctags = new QProcess();
    QString program("ctags");
    QStringList arguments;
    arguments << "-Rf" << "-" << "--fields=+n" << files;
    ctags->start(program, arguments);
    
    if (ctags->waitForStarted())
    {
        char buf[1024];
        QList<QTreeWidgetItem *> items;
        ctags->waitForReadyRead();
        while(ctags->readLine(buf, sizeof(buf)) > 0) {
            QStringList columns = QString(buf).split('\t');
            QTreeWidgetItem *item = _parse_ctag_ouput(ui.treeWidget, buf);
            if(item != nullptr)
                items.append(item);
            ctags->waitForReadyRead();
        }
        ui.treeWidget->insertTopLevelItems(0, items);
        ctags->waitForFinished();
    }
    delete ctags;
    ui.treeWidget->sortItems(0, Qt::AscendingOrder);
    ui.treeWidget->resizeColumnToContents(0);
    ui.treeWidget->resizeColumnToContents(1);
    ui.treeWidget->resizeColumnToContents(2);
    connect(ui.treeWidget, SIGNAL(itemDoubleClicked(QTreeWidgetItem *, int)), this, SLOT(itemDoubleClicked(QTreeWidgetItem *, int)));
}

void CTAGSBrowser::itemDoubleClicked(QTreeWidgetItem *item, int column)
{
    line = item->data(0, Qt::UserRole).toInt();
    file = item->data(0, Qt::ToolTipRole).toString();
    accept();
}

enum CTAGSColumns {
    Column_TAG=0,
    Column_FILENAME=1,
    Column_CODE=2,
    Column_TYPE=3,
    Column_LINE=4,
    Column_CLASS=5,
    Column_FILE=6
};

QTreeWidgetItem *_parse_ctag_ouput(QTreeWidget *treeWidget, const char *buff)
{
    if(buff[0] == '!') // It is a comment line
        return nullptr;
    QStringList columns = QString(buff).split('\t');
    QTreeWidgetItem *item = new QTreeWidgetItem(treeWidget);
    if(columns.size() > Column_CLASS)
        item->setText(0, columns[Column_CLASS].trimmed());
    if(columns.size() > Column_FILENAME)
        item->setToolTip(0, columns[Column_FILENAME].trimmed());
    if(columns.size() > Column_TAG)
        item->setText(1, columns[Column_TAG].trimmed());
    if(columns.size() > Column_CODE) {
        QString code = columns[Column_CODE].trimmed();
        code = code.left(code.size()-4);
        code.replace(QRegularExpression("^/\\^"), "");
        code = code.trimmed();
        item->setToolTip(1, code);
        item->setText(2, code);
    }
    if(columns.size() > Column_LINE) {
        int line = columns[Column_LINE].trimmed().section(':', 1, 1).toInt();
        item->setData(0, Qt::UserRole, line);
    }
    return item;
}


