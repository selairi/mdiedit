/****************************************************************************
**
**   Copyright (C) 2014 P.L. Lucas
**   Copyright (C) 2013 Digia Plc and/or its subsidiary(-ies).
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

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTextDocument>
#include <QHash>

#include "finddialog.h"

class MdiChild;
class Document;
QT_BEGIN_NAMESPACE
class QAction;
class QMenu;
class QMdiArea;
class QMdiSubWindow;
class QSignalMapper;
class QLabel;
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow();

public slots:
	void open(QString fileName);

protected:
    void closeEvent(QCloseEvent *event);

private slots:
    void newFile();
    void open();
    void save();
    void saveAs();
    void cut();
    void copy();
    void paste();
    void selectAll();
    void undo();
    void redo();
    void completion();
    bool findNext();
    void replace();
    void replaceAll();
    void showFindDialog();
    void goToLine();
    void wordwrapMode(MdiChild *child = NULL);
    void showFontDialog();
    void showSnipplesDialog();
    void replaceTabsBySpaces();
    void about();
    void updateMenus();
    void updateWindowMenu();
    MdiChild *createMdiChild();
    void switchLayoutDirection();
    void setActiveSubWindow(QWidget *window);
    void showLineNumber() ;
    void updateMdiChild(QMdiSubWindow * child);
    void newView();
    void saveAll();
    void minimizeAllSubWindows();
    void reparentDocument(Document *doc);
    void toggleTabbedViewMode();

private:
    FindDialog *findDialog;
    void createActions();
    void createMenus();
    void createToolBars();
    void createStatusBar();
    void readSettings();
    void writeSettings();
    MdiChild *activeMdiChild();
    QMdiSubWindow *findMdiChild(const QString &fileName);
    void setFont(MdiChild *child = NULL);
    void putCursorInNotFound(QTextDocument::FindFlags flags);

    QMdiArea *mdiArea;
    QSignalMapper *windowMapper;
    
    QFont font;

    QMenu *fileMenu;
    QMenu *editMenu;
    QMenu *windowMenu;
    QMenu *helpMenu;
    QToolBar *fileToolBar;
    QToolBar *editToolBar;
    QAction *newAct;
    QAction *openAct;
    QAction *saveAct;
    QAction *saveAsAct;
    QAction *saveAllAct;
    QAction *exitAct;
    
    QAction *cutAct;
    QAction *copyAct;
    QAction *pasteAct;
    QAction *undoAct;
    QAction *redoAct;
    QAction *selectAllAct;
    QAction *completionAct;
    QAction *findAct;
    QAction *findNextAct;
    QAction *goToLineAct;
    QAction *wordwrapAct;
    QAction *fontAct;
    QAction *snipplesAct;
    QAction *replaceTabsBySpacesAct;
    
    QAction *newViewAct;
    QAction *closeAct;
    QAction *closeAllAct;
    QAction *minimizeAllAct;
    QAction *tileAct;
    QAction *cascadeAct;
    QAction *tabbedViewAct;
    QAction *nextAct;
    QAction *previousAct;
    QAction *separatorAct;
    QAction *aboutAct;
    QAction *aboutQtAct;
    
    QLabel *lineNumberLabel;
    
    QHash<QString,QString> snipples;
    bool snipplesActivateOk;
    bool replaceTabsBySpacesOk;
};

#endif
