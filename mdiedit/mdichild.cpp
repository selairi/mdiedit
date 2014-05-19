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

#include <QtWidgets>

#include "mdichild.h"

MdiChild::MdiChild()
{
	_document = new Document(this);
	setDocument(_document);
	setAttribute(Qt::WA_DeleteOnClose);
	setWindowIcon(QIcon::fromTheme("text-x-generic"));
	isUntitled = true;
	autoindent = true;
	setTabStopWidth(20);
	connect(document(), SIGNAL(contentsChanged()),
		this, SLOT(documentContentsChanged()));
	connect(document(), SIGNAL(modificationChanged(bool)),
		this, SLOT(documentWasModified()));
	connect(document(), SIGNAL(fileNameChanged(QString)),
		this, SLOT(setCurrentFile(QString)));
}

void MdiChild::keyPressEvent(QKeyEvent * e)
{
    static QRegExp regExp("[^ \t]");
    if(autoindent) {
        if(e->key() == Qt::Key_Return || e->key() == Qt::Key_Enter) {
            QTextCursor cursor = textCursor();
            QString line = cursor.block().text();
            int pos = line.indexOf(regExp);
            QString spaces = line.left(pos);
            QPlainTextEdit::keyPressEvent(e);
            insertPlainText(spaces);
            return;
        }
    }
    
    if(e->key() == Qt::Key_Tab && e->modifiers() == Qt::NoModifier && textCursor().hasSelection()) {
    	QTextCursor cursor = textCursor();
    	int start = cursor.selectionStart() ;
    	int end = cursor.selectionEnd() ;
    	cursor.setPosition(end);
    	cursor.setPosition(start, QTextCursor::KeepAnchor);
    	cursor.movePosition(QTextCursor::StartOfBlock, QTextCursor::KeepAnchor);
    	QString text = "\t" + cursor.selectedText();
    	text.replace(QChar(QChar::ParagraphSeparator), "\n");
    	text.replace(QChar('\n'), "\n\t");
    	start = cursor.selectionStart() ;
    	cursor.insertText(text);
    	cursor.setPosition(start, QTextCursor::KeepAnchor);
    	setTextCursor(cursor);
    	return;
    } else if(e->key() == Qt::Key_Backtab && e->modifiers() == Qt::ShiftModifier && textCursor().hasSelection()) {
    	QTextCursor cursor = textCursor();
    	int start = cursor.selectionStart() ;
    	int end = cursor.selectionEnd() ;
    	cursor.setPosition(end);
    	cursor.setPosition(start, QTextCursor::KeepAnchor);
    	cursor.movePosition(QTextCursor::StartOfBlock, QTextCursor::KeepAnchor);
    	QString text = cursor.selectedText();
    	text.replace(QChar(QChar::ParagraphSeparator), "\n");
    	text.replace(QRegExp("^[ \t]"), "");
    	text.replace(QRegExp("\n[ \t]"), "\n");
    	start = cursor.selectionStart() ;
    	cursor.insertText(text);
    	cursor.setPosition(start, QTextCursor::KeepAnchor);
    	setTextCursor(cursor);
    	return;
    }
    
    QPlainTextEdit::keyPressEvent(e);
}

void MdiChild::newFile()
{
    static int sequenceNumber = 1;
    
    _document->setFileName(tr("document%1.txt").arg(sequenceNumber++));
    isUntitled = true;
    setWindowTitle(_document->fileName() + "[*]");
}

Document *MdiChild::view()
{
	return _document;
}

void MdiChild::setView(MdiChild *mdiChild)
{
	_document = mdiChild->view();
	setDocument(_document);
	bool modified = document()->isModified();
	setCurrentFile(_document->fileName());
	document()->setModified(modified);
	isUntitled = mdiChild->isUntitled;
	setWindowModified(modified);
	setWindowTitle(userFriendlyCurrentFile() + "[*]");

	connect(_document, SIGNAL(contentsChanged()),
		this, SLOT(documentContentsChanged()));
	connect(document(), SIGNAL(modificationChanged(bool)),
		this, SLOT(documentWasModified()));
	connect(_document, SIGNAL(fileNameChanged(QString)),
		this, SLOT(setCurrentFile(QString)));
}

bool MdiChild::loadFile(const QString &fileName)
{
    QFile file(fileName);
    if (!file.open(QFile::ReadOnly | QFile::Text)) {
        QMessageBox::warning(this, tr("MDI"),
                             tr("Cannot read file %1:\n%2.")
                             .arg(fileName)
                             .arg(file.errorString()));
        return false;
    }

    QTextStream in(&file);
    QApplication::setOverrideCursor(Qt::WaitCursor);
    setPlainText(in.readAll());
    QApplication::restoreOverrideCursor();

    setCurrentFile(fileName);

    return true;
}

bool MdiChild::save()
{
    if (isUntitled) {
        return saveAs();
    } else {
        return saveFile(_document->fileName());
    }
}

bool MdiChild::saveAs()
{
    QString fileName = QFileDialog::getSaveFileName(this, tr("Save As"),
                                                    _document->fileName());
    if (fileName.isEmpty())
        return false;

    return saveFile(fileName);
}

bool MdiChild::saveFile(const QString &fileName)
{
    QFile file(fileName);
    if (!file.open(QFile::WriteOnly | QFile::Text)) {
        QMessageBox::warning(this, tr("MDI"),
                             tr("Cannot write file %1:\n%2.")
                             .arg(fileName)
                             .arg(file.errorString()));
        return false;
    }

    QTextStream out(&file);
    QApplication::setOverrideCursor(Qt::WaitCursor);
    out << toPlainText();
    QApplication::restoreOverrideCursor();

    setCurrentFile(fileName);
    return true;
}

QString MdiChild::userFriendlyCurrentFile()
{
    return strippedName(_document->fileName());
}

void MdiChild::closeEvent(QCloseEvent *event)
{
    if (maybeSave()) {
        if(_document->parent() == this) { // Reparent _document to other view
	        Document *doc=_document;
	        _document = NULL;
	        emit reparentDocument(doc);
        }
        event->accept();
    } else {
        event->ignore();
    }
}

void MdiChild::documentContentsChanged()
{
    //setWindowModified(document()->isModified());
    ensureCursorVisible();
}

void MdiChild::documentWasModified()
{
    setWindowModified(document()->isModified());
    //ensureCursorVisible();
}

bool MdiChild::maybeSave()
{
    if (document()->isModified()) {
	QMessageBox::StandardButton ret;
        ret = QMessageBox::warning(this, tr("MDI"),
                     tr("'%1' has been modified.\n"
                        "Do you want to save your changes?")
                     .arg(userFriendlyCurrentFile()),
                     QMessageBox::Save | QMessageBox::Discard
		     | QMessageBox::Cancel);
        if (ret == QMessageBox::Save)
            return save();
        else if (ret == QMessageBox::Cancel)
            return false;
    }
    return true;
}

void MdiChild::setCurrentFile(QString fileName)
{
	if(QFileInfo(fileName).canonicalFilePath() != QFileInfo(_document->fileName()).canonicalFilePath())
		_document->setFileName(QFileInfo(fileName).canonicalFilePath());
	isUntitled = false;
	document()->setModified(false);
	setWindowModified(false);
	setWindowTitle(userFriendlyCurrentFile() + "[*]");
}

QString MdiChild::strippedName(const QString &fullFileName)
{
    return QFileInfo(fullFileName).fileName();
}

