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

#include <QtGlobal>

#if QT_VERSION >= 0x050000
#include <QtWidgets>
#else
#include <QtGui>
#endif

#include "mdichild.h"
#include "textblockdata.h"

MdiChild::MdiChild()
{
	_document = new Document(this);
	docLayout = new PlainTextDocumentLayout(_document);
	_document->setDocumentLayout(docLayout);
	setDocument(_document);
	setAttribute(Qt::WA_DeleteOnClose);
	setWindowIcon(QIcon::fromTheme("text-x-generic"));
	isUntitled = true;
	autoindent = true;
	snipples=NULL;
	snipplesActivateOk=NULL;
	replaceTabsBySpacesOk = NULL;
	//setTabStopWidth(20);
	setCursorWidth(3);
	connect(document(), SIGNAL(contentsChanged()),
		this, SLOT(documentContentsChanged()));
	connect(document(), SIGNAL(modificationChanged(bool)),
		this, SLOT(documentWasModified()));
	connect(document(), SIGNAL(fileNameChanged(QString)),
		this, SLOT(setCurrentFile(QString)));
	connect(docLayout, SIGNAL(docChanged(int, int, int)), this, SLOT(documentChanged(int, int, int)));
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
    
     if(snipplesActivateOk!=NULL && *snipplesActivateOk && snipples!=NULL && e->key() == Qt::Key_Tab && e->modifiers() == Qt::NoModifier &&  ! textCursor().hasSelection()) {
         QTextCursor cursor = textCursor();
         QTextCursor cursorOriginal = textCursor();
         cursor.movePosition(QTextCursor::PreviousWord, QTextCursor::KeepAnchor);
         QString text = cursor.selectedText();
         if(snipples->contains(text)) {
             setTextCursor(cursor);
         	insertPlainText(snipples->value(text));
         	return;
         }
     }
     else if(e->key() == Qt::Key_Tab && e->modifiers() == Qt::NoModifier && textCursor().hasSelection()) {
    	QTextCursor cursor = textCursor();
    	int start = cursor.selectionStart() ;
    	int end = cursor.selectionEnd() ;
    	cursor.setPosition(end);
    	cursor.setPosition(start, QTextCursor::KeepAnchor);
    	cursor.movePosition(QTextCursor::StartOfBlock, QTextCursor::KeepAnchor);
    	QString text = cursor.selectedText();
    	text.replace(QChar(QChar::ParagraphSeparator), "\n");
    	if(replaceTabsBySpacesOk!=NULL && *replaceTabsBySpacesOk) {
    	    text = "    " + text;
    	    text.replace(QChar('\n'), "\n    ");
    	} else {
    	    text = "\t" + text;
    	    text.replace(QChar('\n'), "\n\t");
    	}
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
    } else if(e->key() == Qt::Key_Tab && replaceTabsBySpacesOk!=NULL && *replaceTabsBySpacesOk) {
    	QTextCursor cursor = textCursor();
    	int spaces = 4 - cursor.columnNumber() % 4;
    	for(;spaces>0;spaces--)
    	    cursor.insertText(" ");
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

void MdiChild::documentChanged(int position, int charsRemoved, int charsAdded)
{
    if(blockModeData.enabled)
    {
        if(blockModeData.lineNumber == textCursor().blockNumber())
        {
            QTextCursor cursor = textCursor();
            cursor.setPosition(position);
            cursor.movePosition(QTextCursor::Right, QTextCursor::KeepAnchor, charsAdded);
            QString text = cursor.selectedText();
            int pos = position - cursor.block().position();
            disconnect(docLayout, SIGNAL(docChanged(int, int, int)), this, SLOT(documentChanged(int, int, int)));
         	  disconnect(document(), SIGNAL(contentsChanged()), this, SLOT(documentContentsChanged()));
            for(int i = blockModeData.startLine+1; i<=blockModeData.endLine; i++)
            {
                // Set cursor position
                int line = cursor.blockNumber();
                while(line < _document->lastBlock().blockNumber() && line < i)
                {
                    cursor.setPosition(cursor.block().next().position());
                    line = cursor.blockNumber();
                }
                while(line > 0 && line > i)
                {
                    cursor.setPosition(cursor.block().previous().position());
                    line = cursor.blockNumber();
                }
                int startPosition = pos;
                if( pos >= cursor.block().length() )
                    //startPosition = cursor.block().length() - 1;
                    continue;
                int charsCount = charsRemoved;
                if( (startPosition + charsRemoved) >= cursor.block().length() )
                    //charsCount = cursor.block().length() - startPosition - 1;
                    continue;
                cursor.setPosition(cursor.block().position() + startPosition);
                cursor.movePosition(QTextCursor::Right, QTextCursor::KeepAnchor, charsCount);
                cursor.insertText(text);
            }
            connect(docLayout, SIGNAL(docChanged(int, int, int)), this, SLOT(documentChanged(int, int, int)));
            connect(document(), SIGNAL(contentsChanged()), this, SLOT(documentContentsChanged()));
        }
        else
              blockMode(false);
    }
}

void MdiChild::documentContentsChanged()
{
    //setWindowModified(document()->isModified());
    if(hasFocus())
    {
        ensureCursorVisible();
        
    }
    else
    {
        // Ensure first line visible if focus has been lost (multiview mode).
        int moveToPosition = firstLine.position() - firstVisibleBlock().position();
        QTextCursor actual = textCursor();
        QTextCursor cursor = textCursor();
        cursor.setPosition(firstLine.position());
        setTextCursor(cursor);
        if(moveToPosition>0)
        {
            while(firstLine.block() != firstVisibleBlock() && textCursor().block().blockNumber() < blockCount()-1 )
                moveCursor(QTextCursor::Down);
        }
        setTextCursor(actual);
    }
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
	QFileInfo fileInfo(fileName);
	if(! fileInfo.exists() && _document->fileName() != fileName )
		_document->setFileName(fileName);
	else if(fileInfo.canonicalFilePath() != QFileInfo(_document->fileName()).canonicalFilePath())
		_document->setFileName(QFileInfo(fileName).canonicalFilePath());
	isUntitled = false;
	_document->setModified(false);
	setWindowModified(false);
	if(fileInfo.exists())
		setWindowTitle(userFriendlyCurrentFile() + "[*]");
	else
		setWindowTitle(fileName + "[*]");
}

QString MdiChild::strippedName(const QString &fullFileName)
{
    return QFileInfo(fullFileName).fileName();
}

void MdiChild::focusOutEvent(QFocusEvent * event)
{
    if(event->lostFocus())
    {
        QTextCursor cursor = textCursor();
        cursor.setPosition(firstVisibleBlock().position());
        firstLine = cursor;
    }
    QPlainTextEdit::focusOutEvent(event);
}

void MdiChild::blockMode(bool enableOk)
{
    blockModeData.enabled = enableOk;
    blockModeData.lineNumber = -1;
    if(enableOk)
    {
        QTextCursor cursor = textCursor();
        if( cursor.hasSelection() )
        {
            cursor.setPosition(cursor.selectionEnd());
            blockModeData.endLine = cursor.blockNumber();
            cursor.setPosition(textCursor().selectionStart());
            blockModeData.lineNumber = blockModeData.startLine = cursor.blockNumber();
            setTextCursor(cursor);
            emit showMessage(tr("Block mode: Enabled."));
        }
        else
        {
            blockModeData.enabled = false;
            emit showMessage(tr("Block mode: Disabled. No selected text."));
        }
    }
}

PlainTextDocumentLayout::PlainTextDocumentLayout(QTextDocument *parent):QPlainTextDocumentLayout(parent)
{

}

void PlainTextDocumentLayout::documentChanged(int position, int charsRemoved, int charsAdded)
{
    QPlainTextDocumentLayout::documentChanged(position, charsRemoved, charsAdded);
    emit docChanged(position, charsRemoved, charsAdded);
}
