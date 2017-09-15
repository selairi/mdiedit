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
**   * Neither the name of developers or companies in the above copyright, Digia Plc and its 
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

#include "documentlist.h"
#include <QFileInfo>
#include <QDir>
#include <QAction>
#include <QKeySequence>
#include <QMdiSubWindow>
#include <QBoxLayout>
#include <QDebug>

DocumentList::DocumentList(GlobalConfig *globalConfig,  QMdiArea *mdiArea, QWidget *parent)
{
    this->globalConfig = globalConfig;
    this->mdiArea = mdiArea;
    QAction *action = toggleViewAction();
    QList<QKeySequence> keysAct;
    keysAct << QKeySequence(Qt::Key_F9);
    action->setShortcuts(keysAct);
    setWindowTitle(tr("Document List"));
    container = new QWidget(this);
    QBoxLayout *layout = new QBoxLayout(QBoxLayout::TopToBottom, container);
    //container->setLayout(layout);
    container->setObjectName("Container");
    setWidget(container);
    treeWidget = new QTreeWidget(container);
    //treeWidget->setIndentation(8);
    layout->addWidget(treeWidget);
    treeWidget->setColumnCount(2);
    QStringList labels;
    labels << tr("Name") << tr("Path");
    treeWidget->setHeaderLabels(labels);
    treeWidget->setSortingEnabled(true);
    treeWidget->setColumnWidth(0, 200);
    connect(treeWidget, SIGNAL(itemClicked(QTreeWidgetItem*, int)), this, SLOT(focusMdiChild(QTreeWidgetItem*, int)));
    QBoxLayout *boxLayout = new QBoxLayout(QBoxLayout::LeftToRight);
    layout->addLayout(boxLayout);
    tileModeCheckBox = new QCheckBox(tr("Tile on click"), container);
    tileModeCheckBox->setCheckState(Qt::Unchecked);
    boxLayout->addWidget(tileModeCheckBox);
    maxWindowsShownInTiledSpinBox = new QSpinBox(container);
    boxLayout->addWidget(maxWindowsShownInTiledSpinBox);
    maxWindowsShownInTiledSpinBox->setMinimum(1);
    maxWindowsShownInTiledSpinBox->setValue(3);
    maxWindowsShownInTiledSpinBox->setEnabled(false);
    connect(tileModeCheckBox, SIGNAL(stateChanged(int)), this, SLOT(tileModeCheckBoxStateChanged(int)));
}

QTreeWidgetItem* DocumentList::findTreeNode(QString path, MdiChild *mdichild)
{
    QList<QTreeWidgetItem *> items = treeWidget->findItems(path, Qt::MatchExactly, 1);
    for(QTreeWidgetItem *item : items) {
        return item;
    }
    QString parentPath = QFileInfo(path).dir().absolutePath();
    items = treeWidget->findItems(parentPath, Qt::MatchExactly, 1);
    for(const QTreeWidgetItem *item : items) {
        for(int i = 0; i < item->childCount(); i++) {
            QTreeWidgetItem *child = item->child(i);
            if(mdichild == child->data(2, Qt::DisplayRole).value<MdiChild*>())
                return child;
        }
    }
    return nullptr;
}

QTreeWidgetItem* DocumentList::findTreeNode(MdiChild *mdichild)
{
    QList<QTreeWidgetItem *> items = treeWidget->findItems("*", Qt::MatchWildcard, 1);
    for(QTreeWidgetItem *item : items) {
        for(int i = 0; i < item->childCount(); i++) {
            QTreeWidgetItem *child = item->child(i);
            if(mdichild == child->data(2, Qt::DisplayRole).value<MdiChild*>())
                return child;
        }
    }
    return nullptr;
}

void DocumentList::updateDocument(MdiChild *mdichild)
{
    deleteDocument(mdichild);
    QString path = mdichild->currentFile();
    QString parentPath = QFileInfo(path).dir().absolutePath();
    QTreeWidgetItem * item = findTreeNode(path, mdichild);
    if(item == nullptr) {
        QString parentPath = QFileInfo(path).dir().absolutePath();
        QTreeWidgetItem * parentItem = findTreeNode(parentPath, nullptr);
        if(parentItem == nullptr) {
            parentItem = new QTreeWidgetItem();
            parentItem->setText(0, QFileInfo(path).dir().dirName());
            parentItem->setText(1, parentPath);
            parentItem->setData(0, Qt::ToolTipRole, QVariant(parentPath));
            treeWidget->addTopLevelItem(parentItem);
            treeWidget->expandItem(parentItem);
        } 
        item = new QTreeWidgetItem(parentItem);
        item->setText(1, path);
        item->setData(0, Qt::ToolTipRole, QVariant(path));
        QVariant v;
        v.setValue(mdichild);
        item->setData(2, Qt::DisplayRole, v);
        parentItem->addChild(item);
    }
    if(item != nullptr) {
        QString name = mdichild->userFriendlyCurrentFile() + (mdichild->isWindowModified()?"*":"");
        item->setText(0, name);
        treeWidget->setCurrentItem(item);
    }
}

void DocumentList::deleteDocument(MdiChild *mdichild)
{
    QString path = mdichild->currentFile();
    QTreeWidgetItem * item = findTreeNode(mdichild);
    if(item == nullptr)
        return;
    QTreeWidgetItem * parentItem = item->parent();
    parentItem->removeChild(item);
    delete item;
    if(parentItem->childCount() == 0) {
        int index = treeWidget->indexOfTopLevelItem(parentItem);
        if(treeWidget->takeTopLevelItem(index) != 0)
            delete parentItem;
    }
}

void DocumentList::focusMdiChild(QTreeWidgetItem* item, int column)
{
    MdiChild *child = item->data(2, Qt::DisplayRole).value<MdiChild*>();
    if(child != nullptr) {
        child->setFocus();
    }
    if(tileModeCheckBox->checkState() == Qt::Checked) {
        if(child != nullptr) {
            child->showNormal();
        }
        int maxWindowsShown = maxWindowsShownInTiledSpinBox->value();
        QList<QMdiSubWindow*> windows = mdiArea->subWindowList(QMdiArea::ActivationHistoryOrder);
        int i, count = 0;
        // Count number of not minimized windows
        for(i = 0; i < windows.size(); i++) {
            if(! windows[i]->isMinimized()) count++;
        }
        if(maxWindowsShown == count) {
            mdiArea->tileSubWindows();
            return;
        }
        // Only maxWindowsShown windows are shown.
        // Minimize windows.size() - maxWindowsShown windows and maximize the others
        count = windows.size() - maxWindowsShown;
        for(i = 0; i < windows.size(); i++) {
            if(count > 0 && windows[i] != (QMdiSubWindow*)child) {
                if(! windows[i]->isMinimized()) {
                    windows[i]->showMinimized();
                }
                count--;
            } else {
                if(windows[i]->isMinimized()) windows[i]->showNormal();
            }
        }
        // Restore windows order
        for(i = 0; i < windows.size(); i++) {
            mdiArea->setActiveSubWindow(windows[i]);
        }
        mdiArea->tileSubWindows();
    }
}

void DocumentList::tileModeCheckBoxStateChanged(int state)
{
    maxWindowsShownInTiledSpinBox->setEnabled(state == Qt::Checked);
}