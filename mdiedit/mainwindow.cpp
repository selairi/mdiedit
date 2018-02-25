/****************************************************************************
**
**   Copyright (C) 2017 P.L. Lucas
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

#include <QtGlobal>

#if QT_VERSION >= 0x050000
#include <QtWidgets>
#else
#include <QtGui>
#endif
#include <QFontDialog>
#include <QInputDialog> 
#include <QFileInfo>

#include "mainwindow.h"
#include "mdichild.h"
#include "snippesdialog.h"
#include "completiondialog.h"
#include "ctagsbrowser.h"

MainWindow::MainWindow()
{
    globalConfig = new GlobalConfig(this);
    lineNumberLabel = NULL;
    popupMenu = NULL;
    mdiArea = new QMdiArea(this);
    mdiArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    mdiArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    setCentralWidget(mdiArea);
    connect(mdiArea, SIGNAL(subWindowActivated(QMdiSubWindow*)),
            this, SLOT(updateMdiChild(QMdiSubWindow*)));
    connect(mdiArea, SIGNAL(subWindowActivated(QMdiSubWindow*)),
            this, SLOT(updateMenus()));
    windowMapper = new QSignalMapper(this);
    connect(windowMapper, SIGNAL(mapped(QWidget*)),
            this, SLOT(setActiveSubWindow(QWidget*)));
    actionsMapper = new QSignalMapper(this);
    tabsGroupAct = new QActionGroup(this);
    syntaxGroupAct = new QActionGroup(this);
    spellCheckDictsActGroup = new QActionGroup(this);
    spellCheckMapper = new QSignalMapper(this);
    
    fileBrowserWidget = NULL;

    createActions();
    createMenus();
    createToolBars();
    createStatusBar();
    createDockWidgets();
    updateMenus();

    readSettings();

    setWindowTitle(tr("MDI Edit"));
    setUnifiedTitleAndToolBarOnMac(true);
    
    findDialog = NULL;
}

MainWindow::~MainWindow()
{
    delete [] tabsSpacesAct;
}

void MainWindow::showFileBrowser()
{
    if(fileBrowserWidget == NULL) {
        //showFileBrowser(true);
        fileBrowserDockWidget->show();
    }
    else
        fileBrowserDockWidget->setVisible(!fileBrowserDockWidget->isVisible());
}

void MainWindow::showFileBrowser(bool visibility)
{
    if(visibility && fileBrowserWidget == NULL) {
        fileBrowserWidget = new FileBrowser(this);
        fileBrowserDockWidget->setWidget(fileBrowserWidget);
        fileBrowserWidget->show();
        connect(fileBrowserWidget, SIGNAL(fileActivated(QString)), this, SLOT(open(QString)));
        if(!fileBrowserPath.isEmpty())
            fileBrowserWidget->setPath(fileBrowserPath);
    }
    else if(!visibility && fileBrowserWidget != NULL) {
        fileBrowserPath = fileBrowserWidget->getPath();
        delete fileBrowserWidget;
        fileBrowserWidget = NULL;
        fileBrowserDockWidget->setWidget(fileBrowserWidget);
    }
}

void MainWindow::createDockWidgets()
{
    fileBrowserDockWidget = new QDockWidget(tr("File Browser"), this);
    fileBrowserDockWidget->setObjectName("File Browser");
    addDockWidget(Qt::RightDockWidgetArea, fileBrowserDockWidget);
    fileBrowserDockWidget->hide();
    connect(fileBrowserDockWidget, SIGNAL(visibilityChanged(bool)), this, SLOT(showFileBrowser(bool)));
    
    documentListDockWidget = new DocumentList(globalConfig, mdiArea, this);
    documentListDockWidget->setObjectName("Document List");
    addDockWidget(Qt::RightDockWidgetArea, documentListDockWidget);
}

#include <QTime>
void MainWindow::showLineNumber() {
    MdiChild *child = activeMdiChild();
    showLineNumber(child);
}

void MainWindow::showLineNumber(MdiChild *child) {
    if (child && lineNumberLabel) {
        int lineno = child->textCursor().blockNumber();
        int columnno = child->textCursor().positionInBlock();
        QString str = QString(tr("%1,%2")).arg(++lineno).arg(++columnno);
        lineNumberLabel->setText(str);
    }
}

void MainWindow::updateMdiChild(QMdiSubWindow *) {
    MdiChild *mdichild = activeMdiChild();
    if(mdichild) {
        // Update line number in MainWindow
        {
            QList<QMdiSubWindow *> windows = mdiArea->subWindowList();
            for (int i = 0; i < windows.size(); ++i) {
                MdiChild *child = qobject_cast<MdiChild *>(windows.at(i)->widget());
                disconnect(child, SIGNAL(cursorPositionChanged()), this, SLOT(showLineNumber()));
            }
            connect(activeMdiChild(), SIGNAL(cursorPositionChanged()), this, SLOT(showLineNumber()));
            showLineNumber(mdichild);
        }
        {
            // Set active syntax
            Syntax *syntax = mdichild->getSyntaxHightlighter()->getSyntax();
            bool syntaxFoundOk = false;
            if(syntax != nullptr) {
                if(syntaxsAct.contains(syntax->title)) {
                    QAction *action = syntaxsAct[syntax->title];
                    if(action->text() == syntax->title) {
                        disconnect(actionsMapper, SIGNAL(mapped(QString)), this, SLOT(setSyntax(QString)));
                        action->setChecked(true);
                        connect(actionsMapper, SIGNAL(mapped(QString)), this, SLOT(setSyntax(QString)));
                        syntaxFoundOk = true;
                    }
                }
            }
            if(!syntaxFoundOk) {
                disconnect(actionsMapper, SIGNAL(mapped(QString)), this, SLOT(setSyntax(QString)));
                syntaxsAct["None"]->setChecked(true);
                connect(actionsMapper, SIGNAL(mapped(QString)), this, SLOT(setSyntax(QString)));
            }
        }
    }
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    mdiArea->closeAllSubWindows();
    if (mdiArea->currentSubWindow()) {
        event->ignore();
    } else {
        writeSettings();
        event->accept();
    }
}

void MainWindow::newFile()
{
    MdiChild *child = createMdiChild();
    child->newFile();
    child->show();
}

void MainWindow::open(QString fileName)
{
    if (!fileName.isEmpty()) {
        QMdiSubWindow *existing = findMdiChild(fileName);
        if (existing) {
            mdiArea->setActiveSubWindow(existing);
            return;
        }

        MdiChild *child = createMdiChild();
        QFileInfo fileInfo(fileName);
        if(fileInfo.exists()) {
            if (child->loadFile(fileName)) {
                statusBar()->showMessage(tr("File loaded"), 2000);
                child->show();
            } else {
                child->close();
            }
        }
        else {
            child->setCurrentFile(fileName);
            child->show();
            statusBar()->showMessage(tr("File doesn't exist"), 2000);
        }
    }
}

void MainWindow::open()
{
    QString _path;
     if (activeMdiChild()) {
         _path = activeMdiChild()->currentFile();
     }
    QStringList fileNames = QFileDialog::getOpenFileNames(this, tr("Open"), _path);
    QString fileName;
    foreach(fileName, fileNames)
        open(fileName);
}

void MainWindow::save()
{
    if (activeMdiChild() && activeMdiChild()->save())
        statusBar()->showMessage(tr("File saved"), 2000);
}

void MainWindow::saveAs()
{
    if (activeMdiChild() && activeMdiChild()->saveAs())
        statusBar()->showMessage(tr("File saved"), 2000);
}


void MainWindow::cut()
{
    if (activeMdiChild())
        activeMdiChild()->cut();
}

void MainWindow::copy()
{
    if (activeMdiChild())
        activeMdiChild()->copy();
}

void MainWindow::paste()
{
    if (activeMdiChild())
        activeMdiChild()->paste();
}

void MainWindow::selectAll()
{
    if (activeMdiChild())
        activeMdiChild()->selectAll();
}

void MainWindow::undo()
{
    if (activeMdiChild())
        activeMdiChild()->undo();
}

void MainWindow::redo()
{
    if (activeMdiChild())
        activeMdiChild()->redo();
}

void MainWindow::completion()
{
    if (activeMdiChild()) {
        MdiChild *activeChild = activeMdiChild();
        
        CompletionDialog *completionDialog = new CompletionDialog(this);        

        QStringList completerWordList;
        
        QList<QMdiSubWindow *> windows = mdiArea->subWindowList();
        for (int i = 0; i < windows.size(); ++i) {
            MdiChild *child = qobject_cast<MdiChild *>(windows.at(i)->widget());
            QString str = child->toPlainText();
            completerWordList = completerWordList + str.split(QRegExp("\\W"), QString::SkipEmptyParts);
        }
                
        completerWordList.removeDuplicates();
        completionDialog->setWordList(completerWordList);
    
        QTextCursor cursor = activeChild->textCursor();
        QTextCursor cursorOriginal = activeChild->textCursor();
        cursor.movePosition(QTextCursor::PreviousCharacter, QTextCursor::KeepAnchor);
        QString text = cursor.selectedText();
        if(QRegExp("\\w").exactMatch(text)) {
            cursor = activeChild->textCursor();
            cursor.movePosition(QTextCursor::PreviousWord, QTextCursor::KeepAnchor);
            text = cursor.selectedText();
            activeChild->setTextCursor(cursor);
        } else
            text = QString();
        
        completionDialog->setCompletionPrefix(text);
        
        int ok = completionDialog->exec();
        if(ok == QDialog::Accepted) {
            activeChild->setFocus(Qt::ActiveWindowFocusReason);
            activeChild->insertPlainText(completionDialog->getText());
        }
        else
            activeChild->setTextCursor(cursorOriginal);
        
        completionDialog->clear();
        delete completionDialog;
    }
}

void MainWindow::putCursorInNotFound(QTextDocument::FindFlags flags)
{
    if (activeMdiChild()) {
        if(flags & QTextDocument::FindBackward)
            activeMdiChild()->moveCursor(QTextCursor::End);
        else
            activeMdiChild()->moveCursor(QTextCursor::Start);
    }
}

bool MainWindow::findNext()
{
    if (activeMdiChild() && findDialog) {
        QTextDocument::FindFlags flags = findDialog->findFlags();
        QString str = findDialog->text();
        bool regExpOk = findDialog->regExpChecked();
        bool ok;
        if(regExpOk) {
             QTextCursor cursor = activeMdiChild()->textCursor();
             QTextCursor cursorAux = activeMdiChild()->document()->find(QRegExp(str), cursor, flags);
             ok = !cursorAux.isNull();
             if(ok)
                 activeMdiChild()->setTextCursor(cursorAux);
        }
        else
            ok = activeMdiChild()->find(str, flags);
        if(!ok) {
            QMessageBox msgBox;
            msgBox.setText(tr("Not found."));
            msgBox.exec();
            putCursorInNotFound(flags);
        } else {
            activeMdiChild()->ensureCursorVisible();
        }
        return ok;
    }
    return false;
}

void MainWindow::showFindDialog()
{
    if (activeMdiChild()) {
        if(findDialog == NULL) {
            findDialog = new FindDialog(this);
            connect(findDialog, SIGNAL(find()), this, SLOT(findNext()));
            connect(findDialog, SIGNAL(replace()), this, SLOT(replace()));
            connect(findDialog, SIGNAL(replaceAll()), this, SLOT(replaceAll()));
        }
        findDialog->showDialog();
    }
}

void MainWindow::setFont(MdiChild *child)
{
    if(child==NULL) {
        QList<QMdiSubWindow *> windows = mdiArea->subWindowList();
        for (int i = 0; i < windows.size(); ++i) {
            MdiChild *child = qobject_cast<MdiChild *>(windows.at(i)->widget());
            child->setFont(font);
        }
    }
    else {
        if(child)
            child->setFont(font);
    }
}

void MainWindow::showFontDialog()
{
    bool ok;
    QFont selectedFont = QFontDialog::getFont(&ok, font, this);
    if (ok) {
        font = selectedFont;
        setFont();
    }
}

void MainWindow::showSnippesDialog()
{
    SnippesDialog *dialog = new SnippesDialog(&snippes, globalConfig->snippesActivateOk, this);
    int ok = dialog->exec();
    if(QDialog::Rejected==ok)
        return;
    snippes = dialog->snippes;
    globalConfig->snippesActivateOk = dialog->getActivateSnippes();
    delete dialog;
}

void MainWindow::replace()
{
    
    if (activeMdiChild() && findDialog) {
        //        QTextDocument::FindFlags flags = findDialog->findFlags();
        //        QString str = findDialog->text();
        QString replace_str = findDialog->replaceText();
            bool ok = true;
        if(activeMdiChild()->textCursor().hasSelection()) {
            if( activeMdiChild()->textCursor().selectedText() == replace_str )
                 ok = findNext();
        }
        else
             ok = findNext();
        if(!ok)
            return; 
        else
                activeMdiChild()->insertPlainText(replace_str);
        ok = findNext();
    }
}

void MainWindow::replaceAll()
{
    if (activeMdiChild() && findDialog) {
       QTextDocument::FindFlags flags = findDialog->findFlags();
       QString str = findDialog->text();
       QString replace_str = findDialog->replaceText();
       bool regExpOk = findDialog->regExpChecked();
       bool ok;
           QTextCursor cursor = activeMdiChild()->textCursor();
       cursor.movePosition(QTextCursor::Start);
       cursor.beginEditBlock();
        if(regExpOk)
                    cursor = activeMdiChild()->document()->find(QRegExp(str), cursor, flags);
                else
                    cursor = activeMdiChild()->document()->find(str, cursor, flags);
        ok = !cursor.isNull();
        while(ok) {
                cursor.insertText(replace_str);
                QTextCursor cursorAux;
                if(regExpOk)
                    cursorAux = activeMdiChild()->document()->find(QRegExp(str), cursor, flags);
                else
                    cursorAux = activeMdiChild()->document()->find(str, cursor, flags);
                ok = !cursorAux.isNull();
                if(ok) //cursor.swap(cursorAux);
                    cursor=cursorAux;
        }
        cursor.endEditBlock();
    }
}

void MainWindow::goToLine()
{
    MdiChild *child = activeMdiChild();
    if (child) {
        QTextCursor cursor = child->textCursor();
        bool ok;
        int lineno = QInputDialog::getInt(this, tr("Go to line"), tr("Line number"), cursor.blockNumber()+1, 1, 2147483647, 1, &ok);
        if(ok) {
            cursor.movePosition(QTextCursor::Start);
            cursor.movePosition(QTextCursor::NextBlock, QTextCursor::MoveAnchor, lineno-1);
            child->setTextCursor(cursor);
            child->ensureCursorVisible();
        }
    }
}

void MainWindow::wordwrapMode(MdiChild *child)
{
    if(child==NULL) {
        QList<QMdiSubWindow *> windows = mdiArea->subWindowList();
        for (int i = 0; i < windows.size(); ++i) {
            MdiChild *child = qobject_cast<MdiChild *>(windows.at(i)->widget());
            if(wordwrapAct->isChecked())
                child->setLineWrapMode(QPlainTextEdit::WidgetWidth);
            else
                child->setLineWrapMode(QPlainTextEdit::NoWrap);
        }
    }
    else {
        if(child) {
            if(wordwrapAct->isChecked())
                child->setLineWrapMode(QPlainTextEdit::WidgetWidth);
            else
                child->setLineWrapMode(QPlainTextEdit::NoWrap);
        }
    }
}

void MainWindow::newView()
{
    if (activeMdiChild()) {
        MdiChild *activeChild = activeMdiChild();
        MdiChild *child = createMdiChild();
        child->setView(activeChild);
        child->show();
    }
}

void MainWindow::saveAll()
{
    QList<QMdiSubWindow *> windows = mdiArea->subWindowList();
    
    for (int i = 0; i < windows.size(); ++i) {
        MdiChild *child = qobject_cast<MdiChild *>(windows.at(i)->widget());
        child->save();
    }
}

void MainWindow::minimizeAllSubWindows()
{
    QList<QMdiSubWindow *> windows = mdiArea->subWindowList();
    
    for (int i = 0; i < windows.size(); ++i) {
        MdiChild *child = qobject_cast<MdiChild *>(windows.at(i)->widget());
        child->showMinimized();
    }
}

void MainWindow::toggleTabbedViewMode()
{
    if(tabbedViewAct->isChecked())
        mdiArea->setViewMode(QMdiArea::TabbedView);
    else
        mdiArea->setViewMode(QMdiArea::SubWindowView);
}

void MainWindow::reparentDocument(Document *doc)
{
    QList<QMdiSubWindow *> windows = mdiArea->subWindowList();
    
    for (int i = 0; i < windows.size(); ++i) {
        MdiChild *child = qobject_cast<MdiChild *>(windows.at(i)->widget());
        if( child->view() == doc ) {
            doc->setParent(child);
            break;
        }
    }
}

void MainWindow::about()
{
   QMessageBox::about(this, tr("About MDI Edit"),
            tr("Simple text editor based in the Qt MDI example.\n\n"
                "Copyright (C) 2017 P.L. Lucas\n"
                "License: 3-clause BSD"
            ));
}

void MainWindow::updateMenus()
{
    bool hasMdiChild = (activeMdiChild() != 0);
    saveAct->setEnabled(hasMdiChild);
    saveAsAct->setEnabled(hasMdiChild);
    pasteAct->setEnabled(hasMdiChild);
    selectAllAct->setEnabled(hasMdiChild);
    completionAct->setEnabled(hasMdiChild);
    closeAct->setEnabled(hasMdiChild);
    closeAllAct->setEnabled(hasMdiChild);
    tileAct->setEnabled(hasMdiChild);
    cascadeAct->setEnabled(hasMdiChild);
    nextAct->setEnabled(hasMdiChild);
    previousAct->setEnabled(hasMdiChild);
    separatorAct->setVisible(hasMdiChild);
    
    undoAct->setVisible(hasMdiChild);
    redoAct->setVisible(hasMdiChild);
    
    findAct->setVisible(hasMdiChild);
    findNextAct->setVisible(hasMdiChild);
    goToLineAct->setVisible(hasMdiChild);

    bool hasSelection = (activeMdiChild() &&
                         activeMdiChild()->textCursor().hasSelection());
    cutAct->setEnabled(hasSelection);
    copyAct->setEnabled(hasSelection);

}

void MainWindow::updateWindowMenu()
{
    windowMenu->clear();
    windowMenu->addAction(newViewAct);
    windowMenu->addAction(minimizeAllAct);
    windowMenu->addAction(closeAct);
    windowMenu->addAction(closeAllAct);
    windowMenu->addSeparator();
    windowMenu->addAction(tileAct);
    windowMenu->addAction(cascadeAct);
    windowMenu->addAction(tabbedViewAct);
    windowMenu->addSeparator();
    windowMenu->addAction(nextAct);
    windowMenu->addAction(previousAct);
    windowMenu->addAction(separatorAct);

    QList<QMdiSubWindow *> windows = mdiArea->subWindowList();
    separatorAct->setVisible(!windows.isEmpty());

    for (int i = 0; i < windows.size(); ++i) {
        MdiChild *child = qobject_cast<MdiChild *>(windows.at(i)->widget());

        QString text;
        QString windowTitle = child->userFriendlyCurrentFile();
        if(child->view()->isModified())
            windowTitle+=" *";
        if (i < 9) {
            text = tr("&%1 %2").arg(i + 1)
                               .arg(windowTitle);
        } else {
            text = tr("%1 %2").arg(i + 1)
                              .arg(windowTitle);
        }
        QAction *action  = windowMenu->addAction(text);
        action->setCheckable(true);
        action ->setChecked(child == activeMdiChild());
        connect(action, SIGNAL(triggered()), windowMapper, SLOT(map()));
        windowMapper->setMapping(action, windows.at(i));
    }
}

MdiChild *MainWindow::createMdiChild()
{
    MdiChild *child = new MdiChild(globalConfig, mdiArea);
    child->snippes = &snippes;
    child->globalConfig = globalConfig;
    mdiArea->addSubWindow(child);

    connect(child, SIGNAL(copyAvailable(bool)), cutAct, SLOT(setEnabled(bool)));
    connect(child, SIGNAL(copyAvailable(bool)), copyAct, SLOT(setEnabled(bool)));
    connect(child, SIGNAL(reparentDocument(Document *)), this, SLOT(reparentDocument(Document *)));
    connect(child, SIGNAL(showMessage(QString)), this, SLOT(showMessage(QString)));
    connect(child, SIGNAL(updateWindowName(MdiChild*)), documentListDockWidget, SLOT(updateDocument(MdiChild*)));
    connect(child, SIGNAL(deleteDocument(MdiChild*)), documentListDockWidget, SLOT(deleteDocument(MdiChild*)));

    wordwrapMode(child);
    setFont(child);
    
    return child;
}

void MainWindow::createActions()
{
    newAct = new QAction(QIcon::fromTheme("document-new"), tr("&New"), this);
    newAct->setShortcuts(QKeySequence::New);
    newAct->setStatusTip(tr("Create a new file"));
    connect(newAct, SIGNAL(triggered()), this, SLOT(newFile()));

    openAct = new QAction(QIcon::fromTheme("document-open"), tr("&Open..."), this);
    openAct->setShortcuts(QKeySequence::Open);
    openAct->setStatusTip(tr("Open an existing file"));
    connect(openAct, SIGNAL(triggered()), this, SLOT(open()));

    saveAct = new QAction(QIcon::fromTheme("document-save"), tr("&Save"), this);
    saveAct->setShortcuts(QKeySequence::Save);
    saveAct->setStatusTip(tr("Save the document to disk"));
    connect(saveAct, SIGNAL(triggered()), this, SLOT(save()));

    saveAsAct = new QAction(tr("Save &As..."), this);
    saveAsAct->setShortcuts(QKeySequence::SaveAs);
    saveAsAct->setStatusTip(tr("Save the document under a new name"));
    connect(saveAsAct, SIGNAL(triggered()), this, SLOT(saveAs()));
    
    saveAllAct = new QAction(tr("Save A&ll..."), this);
    saveAllAct->setStatusTip(tr("Save all documents"));
    connect(saveAllAct, SIGNAL(triggered()), this, SLOT(saveAll()));

    exitAct = new QAction(tr("E&xit"), this);
    exitAct->setShortcuts(QKeySequence::Quit);
    exitAct->setStatusTip(tr("Exit the application"));
    connect(exitAct, SIGNAL(triggered()), qApp, SLOT(closeAllWindows()));

    cutAct = new QAction(QIcon::fromTheme("edit-cut"), tr("Cu&t"), this);
    cutAct->setShortcuts(QKeySequence::Cut);
    cutAct->setStatusTip(tr("Cut the current selection's contents to the "
                            "clipboard"));
    connect(cutAct, SIGNAL(triggered()), this, SLOT(cut()));

    copyAct = new QAction(QIcon::fromTheme("edit-copy"), tr("&Copy"), this);
    copyAct->setShortcuts(QKeySequence::Copy);
    copyAct->setStatusTip(tr("Copy the current selection's contents to the "
                             "clipboard"));
    connect(copyAct, SIGNAL(triggered()), this, SLOT(copy()));

    undoAct = new QAction( QIcon::fromTheme("edit-undo"), tr("&Undo"), this);
    undoAct->setShortcuts(QKeySequence::Undo);
    connect(undoAct, SIGNAL(triggered()), this, SLOT(undo()));

    blockModeAct = new QAction( tr("&Block mode"), this);
    QList<QKeySequence> keysBlockModeAct;
    keysBlockModeAct << QKeySequence(Qt::CTRL+Qt::Key_B);
    blockModeAct->setShortcuts(keysBlockModeAct);
    connect(blockModeAct, SIGNAL(triggered()), this, SLOT(blockMode()));
        
    findAct = new QAction(QIcon::fromTheme("edit-find"), tr("&Find"), this);
    findAct->setShortcuts(QKeySequence::Find);
    connect(findAct, SIGNAL(triggered()), this, SLOT(showFindDialog()));
    
    findNextAct = new QAction( tr("Find &next"), this);
    QList<QKeySequence> keysfindNextAct;
    keysfindNextAct << QKeySequence(Qt::Key_F3);
    findNextAct->setShortcuts(keysfindNextAct);
    connect(findNextAct, SIGNAL(triggered()), this, SLOT(findNext()));
    
    goToLineAct = new QAction(tr("&Go to line"), this);
    QList<QKeySequence> keysGoToLineAct;
    keysGoToLineAct << QKeySequence(Qt::CTRL+Qt::Key_G);
    goToLineAct->setShortcuts(keysGoToLineAct);
    connect(goToLineAct, SIGNAL(triggered()), this, SLOT(goToLine()));
    
    redoAct = new QAction(QIcon::fromTheme("edit-redo"), tr("&Redo"), this);
    redoAct->setShortcuts(QKeySequence::Redo);
    connect(redoAct, SIGNAL(triggered()), this, SLOT(redo()));
    
    snippesAct = new QAction( tr("Text shortcuts"), this);
    connect(snippesAct, SIGNAL(triggered()), this, SLOT(showSnippesDialog()));
    
    replaceTabsBySpacesAct = new QAction( tr("Replace tabs by spaces"), this);
    replaceTabsBySpacesAct->setCheckable(true);
    connect(replaceTabsBySpacesAct, SIGNAL(changed()), this, SLOT(replaceTabsBySpaces()));
    
    tabsSpacesAct = new QAction*[N_TABS_SPACES];
    for(int i=0; i<N_TABS_SPACES; i++) {
        int spaces = i==0?1:i*2;
        QString arg=QString("%1").arg(spaces);
        tabsSpacesAct[i] = new QAction(arg, this);
        tabsSpacesAct[i]->setCheckable(true);
        if(spaces == globalConfig->tabsSpacesSize)
            tabsSpacesAct[i]->setChecked(true);
        tabsGroupAct->addAction(tabsSpacesAct[i]);
        connect(tabsSpacesAct[i], SIGNAL(changed()), actionsMapper, SLOT(map()));
        actionsMapper->setMapping(tabsSpacesAct[i], spaces);
    }
    connect(actionsMapper, SIGNAL(mapped(int)), this, SLOT(setTabsSize(int)));

    syntaxHighlightAct = new QAction( tr("Syntax highlight"), this);
    syntaxHighlightAct->setCheckable(true);
    connect(syntaxHighlightAct, SIGNAL(changed()), this, SLOT(syntaxHighlight()));
    
    highlightParenthesisMatchAct = new QAction( tr("Highlight parenthesis match"), this);
    highlightParenthesisMatchAct->setCheckable(true);
    connect(highlightParenthesisMatchAct, SIGNAL(changed()), this, SLOT(highlightParenthesisMatch()));
    
    {
        QList<Syntax*> list = SyntaxHighlighter::syntaxsList();
        int syntaxsActLength = list.size()+1;
        for(int i=0; i<syntaxsActLength; i++) {
            QString syntaxName = (i == 0) ? "None" : list[i-1]->title;
            QAction *action = new QAction(syntaxName, this);
            syntaxsAct[syntaxName] = action;
            action->setCheckable(true);
            syntaxGroupAct->addAction(action);
            connect(action, SIGNAL(changed()), actionsMapper, SLOT(map()));
            actionsMapper->setMapping(action, syntaxName);
        }
        connect(actionsMapper, SIGNAL(mapped(QString)), this, SLOT(setSyntax(QString)));
    }
    
    wordwrapAct = new QAction( tr("&Wordwrap"), this);
    wordwrapAct->setCheckable(true);
    connect(wordwrapAct, SIGNAL(triggered()), this, SLOT(wordwrapMode()));
    
    fontAct = new QAction( tr("&Font"), this);
    connect(fontAct, SIGNAL(triggered()), this, SLOT(showFontDialog()));

    pasteAct = new QAction(QIcon::fromTheme("edit-paste"), tr("&Paste"), this);
    pasteAct->setShortcuts(QKeySequence::Paste);
    pasteAct->setStatusTip(tr("Paste the clipboard's contents into the current "
                              "selection"));
    connect(pasteAct, SIGNAL(triggered()), this, SLOT(paste()));
    
    selectAllAct = new QAction(QIcon::fromTheme("edit-select-all"), tr("Select All"), this);
    selectAllAct->setShortcuts(QKeySequence::SelectAll);
    connect(selectAllAct, SIGNAL(triggered()), this, SLOT(selectAll()));
    
    completionAct = new QAction( tr("Completer"), this);
    QList<QKeySequence> keysCompletionAct;
    keysCompletionAct << QKeySequence(Qt::Key_F2);
    completionAct->setShortcuts(keysCompletionAct);
    connect(completionAct, SIGNAL(triggered()), this, SLOT(completion()));
    
    showFileBrowserAct = new QAction( tr("Show/Hide file browser"), this);
    QList<QKeySequence> keysShowFileBrowserAct;
    keysShowFileBrowserAct << QKeySequence(Qt::Key_F5);
    showFileBrowserAct->setShortcuts(keysShowFileBrowserAct);
    connect(showFileBrowserAct, SIGNAL(triggered()), this, SLOT(showFileBrowser()));
    
    ctagsBrowserAct = new QAction( tr("CTAGS browser"), this);
    QList<QKeySequence> keysCTAGSBrowserAct;
    keysCTAGSBrowserAct << QKeySequence(Qt::Key_F6);
    ctagsBrowserAct->setShortcuts(keysCTAGSBrowserAct);
    connect(ctagsBrowserAct, SIGNAL(triggered()), this, SLOT(showCTAGSBrowser()));
    
    spellDictNoneAct = new QAction(tr("None"), this);
    spellDictNoneAct->setCheckable(true);
    spellCheckDictsActGroup->addAction(spellDictNoneAct);
    connect(spellDictNoneAct, SIGNAL(changed()), spellCheckMapper, SLOT(map()));
    spellCheckMapper->setMapping(spellDictNoneAct, 0);
    connect(spellCheckMapper, SIGNAL(mapped(int)), this, SLOT(setSpellDict(int)));
    spellCheckDictsAct.append(spellDictNoneAct);

    newViewAct = new QAction( tr("&New view"), this);
    connect(newViewAct, SIGNAL(triggered()), this, SLOT(newView()));
    
    closeAct = new QAction(tr("Cl&ose"), this);
    closeAct->setStatusTip(tr("Close the active window"));
    connect(closeAct, SIGNAL(triggered()),
            mdiArea, SLOT(closeActiveSubWindow()));

    closeAllAct = new QAction(tr("Close &All"), this);
    closeAllAct->setStatusTip(tr("Close all the windows"));
    connect(closeAllAct, SIGNAL(triggered()),
            mdiArea, SLOT(closeAllSubWindows()));

    minimizeAllAct = new QAction(tr("&Minimize All"), this);
    minimizeAllAct->setStatusTip(tr("Minimize all the windows"));
    connect(minimizeAllAct, SIGNAL(triggered()),
            this, SLOT(minimizeAllSubWindows()));

    tileAct = new QAction(tr("&Tile"), this);
    tileAct->setStatusTip(tr("Tile the windows"));
    QList<QKeySequence> keysTileAct;
    keysTileAct << QKeySequence(Qt::Key_F10);
    tileAct->setShortcuts(keysTileAct);
    connect(tileAct, SIGNAL(triggered()), mdiArea, SLOT(tileSubWindows()));

    cascadeAct = new QAction(tr("&Cascade"), this);
    cascadeAct->setStatusTip(tr("Cascade the windows"));
    connect(cascadeAct, SIGNAL(triggered()), mdiArea, SLOT(cascadeSubWindows()));
    
    tabbedViewAct = new QAction(tr("T&abbed view"), this);
    tabbedViewAct->setStatusTip(tr("Display sub-windows with tabs in a tab bar."));
    tabbedViewAct->setCheckable(true);
    connect(tabbedViewAct, SIGNAL(triggered()), this, SLOT(toggleTabbedViewMode()));

    nextAct = new QAction(tr("Ne&xt"), this);
    nextAct->setShortcuts(QKeySequence::NextChild);
    nextAct->setStatusTip(tr("Move the focus to the next window"));
    connect(nextAct, SIGNAL(triggered()),
            mdiArea, SLOT(activateNextSubWindow()));

    previousAct = new QAction(tr("Pre&vious"), this);
    previousAct->setShortcuts(QKeySequence::PreviousChild);
    previousAct->setStatusTip(tr("Move the focus to the previous "
                                 "window"));
    connect(previousAct, SIGNAL(triggered()),
            mdiArea, SLOT(activatePreviousSubWindow()));

    separatorAct = new QAction(this);
    separatorAct->setSeparator(true);

    aboutAct = new QAction(tr("&About"), this);
    aboutAct->setStatusTip(tr("Show the application's About box"));
    connect(aboutAct, SIGNAL(triggered()), this, SLOT(about()));

    aboutQtAct = new QAction(tr("About &Qt"), this);
    aboutQtAct->setStatusTip(tr("Show the Qt library's About box"));
    connect(aboutQtAct, SIGNAL(triggered()), qApp, SLOT(aboutQt()));
}

void MainWindow::createMenus()
{
    fileMenu = menuBar()->addMenu(tr("&File"));
    fileMenu->addAction(newAct);
    fileMenu->addAction(openAct);
    fileMenu->addAction(saveAct);
    fileMenu->addAction(saveAsAct);
    fileMenu->addAction(saveAllAct);
    fileMenu->addSeparator();
    QAction *action = fileMenu->addAction(tr("Switch layout direction"));
    connect(action, SIGNAL(triggered()), this, SLOT(switchLayoutDirection()));
    fileMenu->addAction(exitAct);

    editMenu = menuBar()->addMenu(tr("&Edit"));
    editMenu->addAction(undoAct);
    editMenu->addAction(redoAct);
    editMenu->addSeparator();
    editMenu->addAction(cutAct);
    editMenu->addAction(copyAct);
    editMenu->addAction(pasteAct);
    editMenu->addAction(selectAllAct);
    editMenu->addAction(blockModeAct);
    editMenu->addSeparator();
    editMenu->addAction(completionAct);
    editMenu->addSeparator();
    editMenu->addAction(findAct);
    editMenu->addAction(findNextAct);
    editMenu->addAction(goToLineAct);
    editMenu->addSeparator();
    editMenu->addAction(snippesAct);
    editMenu->addAction(wordwrapAct);
    editMenu->addAction(fontAct);
    tabsMenu = editMenu->addMenu(tr("Tabs"));
    tabsMenu->addAction(replaceTabsBySpacesAct);
    for(int i=0; i<N_TABS_SPACES; i++) {
        tabsMenu->addAction(tabsSpacesAct[i]);
    }
    syntaxHighlightMenu = editMenu->addMenu(tr("Syntax"));
    syntaxHighlightMenu->addAction(syntaxHighlightAct);
    syntaxHighlightMenu->addAction(highlightParenthesisMatchAct);
    syntaxHighlightMenu->addSeparator();
    {
        QStringList list(syntaxsAct.keys());
        QString none("None");
        list.removeOne(none);
        list.sort();
        list.prepend(none);
        for(QString key : list) {
            syntaxHighlightMenu->addAction(syntaxsAct[key]);
        }
    }
    
    toolsMenu = menuBar()->addMenu(tr("&Tools"));
    toolsMenu->addAction(showFileBrowserAct);
    toolsMenu->addAction(ctagsBrowserAct);
    spellCheckMenu = toolsMenu->addMenu(tr("Spell"));
    for(int i=0; i<spellCheckDictsAct.size(); i++)
        spellCheckMenu->addAction(spellCheckDictsAct[i]);
    

    windowMenu = menuBar()->addMenu(tr("&Window"));
    updateWindowMenu();
    connect(windowMenu, SIGNAL(aboutToShow()), this, SLOT(updateWindowMenu()));

    menuBar()->addSeparator();

    helpMenu = menuBar()->addMenu(tr("&Help"));
    helpMenu->addAction(aboutAct);
    helpMenu->addAction(aboutQtAct);
}

void MainWindow::createToolBars()
{
    fileToolBar = addToolBar(tr("File"));
    fileToolBar->addAction(newAct);
    fileToolBar->addAction(openAct);
    fileToolBar->addAction(saveAct);
    fileToolBar->setObjectName("FileToolBar");

    editToolBar = addToolBar(tr("Edit"));
    editToolBar->addAction(cutAct);
    editToolBar->addAction(copyAct);
    editToolBar->addAction(pasteAct);
    editToolBar->addAction(findAct);
    editToolBar->setObjectName("EditToolBar");
}

void MainWindow::createStatusBar()
{
    statusBar()->showMessage(tr("Ready"));
    lineNumberLabel = new QLabel(this);
    lineNumberLabel->setTextFormat(Qt::PlainText);
    statusBar()->addPermanentWidget(lineNumberLabel);
}

void MainWindow::readSettings()
{
    QSettings settings("QtProject", "MDI Edit");
    QPoint pos = settings.value("pos", QPoint(200, 200)).toPoint();
    QSize size = settings.value("size", QSize(400, 400)).toSize();
    move(pos);
    resize(size);
    if(settings.value("maximized", false).toBool())
        showMaximized();
    restoreState(settings.value("windowState").toByteArray());
    tabbedViewAct->setChecked(settings.value("tabbedViewMode", QVariant(false)).toBool());
    toggleTabbedViewMode();
    settings.beginGroup("format");
    wordwrapAct->setChecked(settings.value("wordwrap", true).toBool());
    wordwrapMode();
    font.fromString(settings.value("font").toString());
    setFont();
    settings.endGroup();
    globalConfig->replaceTabsBySpacesOk = settings.value("replaceTabsBySpacesOk", false).toBool();
    replaceTabsBySpacesAct->setChecked(globalConfig->replaceTabsBySpacesOk);
    globalConfig->tabsSpacesSize = settings.value("tabsSpacesSize").toInt();
    for(int i=0;i<N_TABS_SPACES;i++) {
        int spaces = i==0?1:i*2;
        tabsSpacesAct[i]->setChecked(spaces == globalConfig->tabsSpacesSize);
    }
    globalConfig->snippesActivateOk = settings.value("snippesActivateOk", false).toBool();
    globalConfig->setSyntaxHighlight(settings.value("syntaxHighlightOk", true).toBool());
    syntaxHighlightAct->setChecked(globalConfig->isSyntaxHighlight());
    globalConfig->setHighlightParenthesisMatch(settings.value("highlightParenthesisMatch", true).toBool());
    highlightParenthesisMatchAct->setChecked(globalConfig->isHighlightParenthesisMatch());
    settings.beginGroup("snippes");
    QStringList keys = settings.childKeys();
     foreach(QString key, keys) {
         snippes[key]=settings.value(key).toString();
    }
    settings.endGroup();
}

void MainWindow::writeSettings()
{
    QSettings settings("QtProject", "MDI Edit");
    settings.setValue("pos", pos());
    settings.setValue("size", size());
    settings.setValue("maximized", isMaximized());
    settings.setValue("tabbedViewMode", tabbedViewAct->isChecked());
    settings.setValue("windowState", saveState());
    settings.beginGroup("format");
    settings.setValue("wordwrap", wordwrapAct->isChecked());
    settings.setValue("font", font.toString());
    settings.endGroup();
    settings.setValue("replaceTabsBySpacesOk", globalConfig->replaceTabsBySpacesOk);
    settings.setValue("tabsSpacesSize", globalConfig->tabsSpacesSize);
    settings.setValue("snippesActivateOk", globalConfig->snippesActivateOk);
    settings.setValue("syntaxHighlightOk", globalConfig->isSyntaxHighlight());
    settings.setValue("highlightParenthesisMatch", globalConfig->isHighlightParenthesisMatch());
    settings.beginGroup("snippes");
    QHash<QString, QString>::const_iterator i = snippes.constBegin();
    while (i != snippes.constEnd()) {
        settings.setValue(i.key(), i.value());
        ++i;
    }
    settings.endGroup();
}

MdiChild *MainWindow::activeMdiChild()
{
    QMdiSubWindow *activeSubWindow = mdiArea->activeSubWindow();
    if (activeSubWindow)
        return qobject_cast<MdiChild *>(activeSubWindow->widget());
    return 0;
}

QMdiSubWindow *MainWindow::findMdiChild(const QString &fileName)
{
    QString canonicalFilePath = QFileInfo(fileName).canonicalFilePath();

    foreach (QMdiSubWindow *window, mdiArea->subWindowList()) {
        MdiChild *mdiChild = qobject_cast<MdiChild *>(window->widget());
        if (mdiChild->currentFile() == canonicalFilePath)
            return window;
    }
    return 0;
}

void MainWindow::switchLayoutDirection()
{
    if (layoutDirection() == Qt::LeftToRight)
        qApp->setLayoutDirection(Qt::RightToLeft);
    else
        qApp->setLayoutDirection(Qt::LeftToRight);
}

void MainWindow::setActiveSubWindow(QWidget *window)
{
    if (!window)
        return;
    mdiArea->setActiveSubWindow(qobject_cast<QMdiSubWindow *>(window));
}

void MainWindow::replaceTabsBySpaces()
{
    globalConfig->replaceTabsBySpacesOk = replaceTabsBySpacesAct->isChecked();
}

void MainWindow::syntaxHighlight()
{
    globalConfig->setSyntaxHighlight(syntaxHighlightAct->isChecked());
}

void MainWindow::highlightParenthesisMatch()
{
        globalConfig->setHighlightParenthesisMatch(highlightParenthesisMatchAct->isChecked());
}

void MainWindow::setTabsSize(int spaces)
{
    globalConfig->tabsSpacesSize = spaces;
    foreach (QMdiSubWindow *window, mdiArea->subWindowList()) {
        MdiChild *mdiChild = qobject_cast<MdiChild *>(window->widget());
        mdiChild->updateTabsSize();
    }
}

void MainWindow::blockMode()
{
    MdiChild *child = activeMdiChild();
    if (child)
        child->blockMode();
}

void MainWindow::showMessage(QString text)
{
    statusBar()->showMessage(text, 10000);
}

void MainWindow::showEvent(QShowEvent *event)
{
    QMainWindow::showEvent(event);
    if(popupMenu == NULL)
    {
        toolsMenu->addMenu(popupMenu = createPopupMenu());
        popupMenu->setTitle(tr("Show/Hide Tools"));
    }
}

void MainWindow::setSyntax(QString syntaxName)
{
    MdiChild *mdichild = activeMdiChild();
    if(mdichild != nullptr)
        mdichild->getSyntaxHightlighter()->setSyntax(syntaxName);
}

void MainWindow::showCTAGSBrowser()
{
    QStringList files;
    foreach (QMdiSubWindow *window, mdiArea->subWindowList()) {
        MdiChild *mdiChild = qobject_cast<MdiChild *>(window->widget());
        files << mdiChild->currentFile();
    }
    CTAGSBrowser *browser = new CTAGSBrowser(this, files);
    if(browser->exec() == QDialog::Accepted) {
        open(browser->file);
        MdiChild *child = activeMdiChild();
        if (child) {
            QTextCursor cursor = child->textCursor();
            cursor.movePosition(QTextCursor::Start);
            cursor.movePosition(QTextCursor::NextBlock, QTextCursor::MoveAnchor, browser->line-1);
            child->setTextCursor(cursor);
            child->ensureCursorVisible();
        }
    }
    delete browser;
}

void MainWindow::setSpellChecker(SpellCheck *spellChecker)
{
    globalConfig->setSpellCheck(spellChecker);
    
    // Update spell check menu
    QStringList langs = spellChecker->getDicts();
    for(int i=0; i<langs.size(); i++) {
        QAction *spellDictAct = new QAction(langs[i], this);
        spellDictAct->setCheckable(true);
        spellCheckDictsActGroup->addAction(spellDictAct);
        connect(spellDictAct, SIGNAL(changed()), spellCheckMapper, SLOT(map()));
        spellCheckMapper->setMapping(spellDictAct, i+1);
        spellCheckDictsAct.append(spellDictAct);
    }
    for(int i=1; i<spellCheckDictsAct.size(); i++)
        spellCheckMenu->addAction(spellCheckDictsAct[i]);
    //setSpellDict(0);
    spellDictNoneAct->setChecked(true);
}

void MainWindow::setSpellDict(int i)
{
    SpellCheck *spellCheck = globalConfig->getSpellCheck();
    if(spellCheck == nullptr)
        return;
    // If i==0, it is disabled
    spellCheck->setEnable(i!=0);
    spellCheck->setLang(spellCheckDictsAct[i]->text());
    foreach (QMdiSubWindow *window, mdiArea->subWindowList()) {
        MdiChild *mdiChild = qobject_cast<MdiChild *>(window->widget());
        mdiChild->getSyntaxHightlighter()->rehighlight();
    }
}