/****************************************************************************
**
**   Copyright (C) 2016 P.L. Lucas
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
#include <QCompleter>
#include <QFileInfo>

#include "mainwindow.h"
#include "mdichild.h"
#include "snipplesdialog.h"

MainWindow::MainWindow()
{
    snipplesActivateOk = true;
    replaceTabsBySpacesOk = false;
    lineNumberLabel = NULL;
    mdiArea = new QMdiArea;
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

    createActions();
    createMenus();
    createToolBars();
    createStatusBar();
    updateMenus();

    readSettings();

    setWindowTitle(tr("MDI Edit"));
    setUnifiedTitleAndToolBarOnMac(true);
    
    findDialog = NULL;
}

#include <QTime>
void MainWindow::showLineNumber() {
	// static QTime time = QTime::currentTime();
	// if(time.restart()<100)
	// 	return;
	MdiChild *child = activeMdiChild();
	if (child && lineNumberLabel) {
		int lineno = child->textCursor().blockNumber();
		int columnno = child->textCursor().positionInBlock();
		QString str = QString(tr("%1,%2")).arg(++lineno).arg(++columnno);
		lineNumberLabel->setText(str);
		}
}

void MainWindow::updateMdiChild(QMdiSubWindow *) {
	if(activeMdiChild()) {
		QList<QMdiSubWindow *> windows = mdiArea->subWindowList();
		for (int i = 0; i < windows.size(); ++i) {
			MdiChild *child = qobject_cast<MdiChild *>(windows.at(i)->widget());
			disconnect(child, SIGNAL(cursorPositionChanged()), this, SLOT(showLineNumber()));
		}
		connect(activeMdiChild(), SIGNAL(cursorPositionChanged()), this, SLOT(showLineNumber()));
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
	
		QDialog dialog(this);
		dialog.setWindowTitle(tr("Completer"));
		QVBoxLayout *layout = new QVBoxLayout(&dialog);
		dialog.setLayout(layout);
		
		QCompleter *completer = new QCompleter(&dialog);
		QStringListModel *completerWordListModel = new QStringListModel(&dialog);;
		QStringList completerWordList;
		
		QList<QMdiSubWindow *> windows = mdiArea->subWindowList();
		for (int i = 0; i < windows.size(); ++i) {
			MdiChild *child = qobject_cast<MdiChild *>(windows.at(i)->widget());
			QString str = child->toPlainText();
			completerWordList = completerWordList + str.split(QRegExp("\\W"), QString::SkipEmptyParts);
		}
		
		completerWordList.removeDuplicates();
		completerWordListModel->setStringList(completerWordList);
		completer->setModel(completerWordListModel);
		
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
		
		
		
		QLineEdit *line = new QLineEdit(&dialog);
		line->setText(text);
		line->setCompleter(completer);
		layout->addWidget(line);
		connect(line, SIGNAL(returnPressed()), &dialog, SLOT(accept()));
		completer->setCompletionPrefix(text);
		QTimer::singleShot(500, completer, SLOT(complete()));
		int ok = dialog.exec();
		if(ok == QDialog::Accepted)
			activeChild->insertPlainText(line->text());
		else
			activeChild->setTextCursor(cursorOriginal);
		disconnect(line, SIGNAL(returnPressed()), &dialog, SLOT(accept()));
		
		completerWordList.clear();
		completerWordListModel->setStringList(completerWordList);
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

void MainWindow::showSnipplesDialog()
{
	SnipplesDialog *dialog = new SnipplesDialog(&snipples, snipplesActivateOk, this);
	int ok = dialog->exec();
	if(QDialog::Rejected==ok)
		return;
	snipples = dialog->snipples;
	snipplesActivateOk = dialog->getActivateSnipples();
	delete dialog;
}

void MainWindow::replace()
{
    
    if (activeMdiChild() && findDialog) {
		//		QTextDocument::FindFlags flags = findDialog->findFlags();
		//		QString str = findDialog->text();
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
            	"Copyright (C) 2016 P.L. Lucas\n"
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
    MdiChild *child = new MdiChild;
    child->snipples = &snipples;
    child->snipplesActivateOk = &snipplesActivateOk;
    child->replaceTabsBySpacesOk = &replaceTabsBySpacesOk;
    mdiArea->addSubWindow(child);

    connect(child, SIGNAL(copyAvailable(bool)), cutAct, SLOT(setEnabled(bool)));
    connect(child, SIGNAL(copyAvailable(bool)), copyAct, SLOT(setEnabled(bool)));
    connect(child, SIGNAL(reparentDocument(Document *)), this, SLOT(reparentDocument(Document *)));
    connect(child, SIGNAL(showMessage(QString)), this, SLOT(showMessage(QString)));

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
    
    snipplesAct = new QAction( tr("Text shortcuts"), this);
    connect(snipplesAct, SIGNAL(triggered()), this, SLOT(showSnipplesDialog()));
    
    replaceTabsBySpacesAct = new QAction( tr("Replace tabs by spaces"), this);
    replaceTabsBySpacesAct->setCheckable(true);
    connect(replaceTabsBySpacesAct, SIGNAL(changed()), this, SLOT(replaceTabsBySpaces()));
    
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
    editMenu->addAction(snipplesAct);
    editMenu->addAction(replaceTabsBySpacesAct);
    editMenu->addAction(wordwrapAct);
    editMenu->addAction(fontAct);

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

    editToolBar = addToolBar(tr("Edit"));
    editToolBar->addAction(cutAct);
    editToolBar->addAction(copyAct);
    editToolBar->addAction(pasteAct);
    editToolBar->addAction(findAct);
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
    fileToolBar->setVisible(settings.value("fileToolBar", QVariant(true)).toBool());
    editToolBar->setVisible(settings.value("editToolBar", QVariant(true)).toBool());
    tabbedViewAct->setChecked(settings.value("tabbedViewMode", QVariant(false)).toBool());
    toggleTabbedViewMode();
    settings.beginGroup("format");
    wordwrapAct->setChecked(settings.value("wordwrap").toBool());
    wordwrapMode();
    font.fromString(settings.value("font").toString());
    setFont();
    settings.endGroup();
    replaceTabsBySpacesOk = settings.value("replaceTabsBySpacesOk").toBool();
    replaceTabsBySpacesAct->setChecked(replaceTabsBySpacesOk);
    snipplesActivateOk = settings.value("snipplesActivateOk").toBool();
    settings.beginGroup("snipples");
    QStringList keys = settings.childKeys();
     foreach(QString key, keys) {
         snipples[key]=settings.value(key).toString();
    }
    settings.endGroup();
}

void MainWindow::writeSettings()
{
    QSettings settings("QtProject", "MDI Edit");
    settings.setValue("pos", pos());
    settings.setValue("size", size());
    settings.setValue("maximized", isMaximized());
    settings.setValue("fileToolBar", fileToolBar->isVisible());
    settings.setValue("editToolBar", editToolBar->isVisible());
    settings.setValue("tabbedViewMode", tabbedViewAct->isChecked());
    settings.beginGroup("format");
    settings.setValue("wordwrap", wordwrapAct->isChecked());
    settings.setValue("font", font.toString());
    settings.endGroup();
    settings.setValue("replaceTabsBySpacesOk", replaceTabsBySpacesOk);
    settings.setValue("snipplesActivateOk", snipplesActivateOk);
    settings.beginGroup("snipples");
    QHash<QString, QString>::const_iterator i = snipples.constBegin();
    while (i != snipples.constEnd()) {
        settings.setValue(i.key(), i.value());
        ++i;
    }
    settings.endGroup();
}

MdiChild *MainWindow::activeMdiChild()
{
    if (QMdiSubWindow *activeSubWindow = mdiArea->activeSubWindow())
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
    replaceTabsBySpacesOk = replaceTabsBySpacesAct->isChecked();
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