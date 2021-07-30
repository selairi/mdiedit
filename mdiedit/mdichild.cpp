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

#include <QPalette>
#include <QRegularExpression>

#include "mdichild.h"
#include "textblockdata.h"
#include "textthemes.h"

MdiChild::MdiChild(GlobalConfig *globalConfig, QWidget *parent):QPlainTextEdit(parent)
{
	_document = new Document(this);
	docLayout = new PlainTextDocumentLayout(_document);
	_document->setDocumentLayout(docLayout);
	setDocument(_document);
	blockMode(false);
	syntaxHightlighter = new SyntaxHighlighter(_document, palette(), globalConfig);
	syntaxHightlighter->setDocument(_document);
	setAttribute(Qt::WA_DeleteOnClose);
	setWindowIcon(QIcon::fromTheme("text-x-generic"));
	isUntitled = true;
	autoindent = true;
	snippes = NULL;
	snippesMode.isEnabled = false;
	snippesMode.lastPostionIndex = -1;
	this->globalConfig = globalConfig;
	updateTabsSize();
	setCursorWidth(3);
	updateTextTheme();
	connect(document(), SIGNAL(contentsChanged()),
		this, SLOT(documentContentsChanged()));
	connect(document(), SIGNAL(modificationChanged(bool)),
		this, SLOT(documentWasModified()));
	connect(document(), SIGNAL(fileNameChanged(QString)),
		this, SLOT(setCurrentFile(QString)));
	connect(document(), SIGNAL(contentsChange(int, int, int)), this, SLOT(documentChanged(int, int, int)));
	connect(this, SIGNAL(cursorPositionChanged()), this, SLOT(matchParenthesisPair()));
}

void MdiChild::keyPressEvent(QKeyEvent * e)
{
    static QRegExp regExp("[^ \t]");
    if(autoindent) {
        if(e->key() == Qt::Key_Return || e->key() == Qt::Key_Enter) {
            QTextCursor cursor = textCursor();
            QString line = cursor.block().text().left(cursor.positionInBlock());
            int pos = line.indexOf(regExp);
            QString spaces = line.left(pos);
            QPlainTextEdit::keyPressEvent(e);
            insertPlainText(spaces);
            return;
        }
    }
    
    if(globalConfig->snippesActivateOk && snippes!=NULL && e->key() == Qt::Key_Tab && e->modifiers() == Qt::NoModifier &&  ! textCursor().hasSelection()) {
        QTextCursor cursor = textCursor();
        if( ! snippesMode.isEnabled || 
                (
                    snippesMode.isEnabled && snippesMode.cursorStartSnippe < snippesMode.cursorEndSnippe &&
                    (snippesMode.cursorStartSnippe > cursor || snippesMode.cursorEndSnippe < cursor) 
                )
            ) {
            if(snippesMode.isEnabled) {
                snippesMode.reset();
            }
            QTextCursor cursorOriginal = textCursor();
            cursor.movePosition(QTextCursor::PreviousWord, QTextCursor::KeepAnchor);
            QString text = cursor.selectedText();
            if(snippes->contains(text)) {
               setTextCursor(cursor);
               enableSnippesMode(snippes->value(text), cursor);
               return;
            } else if(globalConfig->replaceTabsBySpacesOk) {
               insertSpacesAsTab(cursorOriginal);
               return;
            }
        } else {
            if(! execNextSnippe(cursor))
                insertSpacesAsTab(cursor);
            return;
        }
    } else if(e->key() == Qt::Key_Tab && e->modifiers() == Qt::NoModifier && textCursor().hasSelection()) {
        	QTextCursor cursor = textCursor();
        	int start = cursor.selectionStart() ;
        	int end = cursor.selectionEnd() ;
        	cursor.setPosition(end);
        	cursor.setPosition(start, QTextCursor::KeepAnchor);
        	cursor.movePosition(QTextCursor::StartOfBlock, QTextCursor::KeepAnchor);
        	QString text = cursor.selectedText();
        	text.replace(QChar(QChar::ParagraphSeparator), "\n");
        	if(globalConfig->replaceTabsBySpacesOk) {
        	    QString spaces(globalConfig->tabsSpacesSize, ' ');
        	    text = spaces + text;
        	    spaces ="\n" + spaces;
        	    text.replace(QChar('\n'), spaces);
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
    } else if(e->key() == Qt::Key_Backtab && e->modifiers() == Qt::ShiftModifier) {
        	QTextCursor cursor = textCursor();
        	if( globalConfig->replaceTabsBySpacesOk ) {   
            	   removeSpacesAsTab(cursor);
        	} else {
        	   cursor.movePosition(QTextCursor::Left, QTextCursor::KeepAnchor, 1);
        	   if( cursor.selectedText() == "\t" ) {
        	       cursor.removeSelectedText();
        	       setTextCursor(cursor);
        	   }
        	}
        	return;
    } else if(e->key() == Qt::Key_Tab && globalConfig->replaceTabsBySpacesOk) {
        	QTextCursor cursor = textCursor();
        	insertSpacesAsTab(cursor);
        	return;
    }
    
    QPlainTextEdit::keyPressEvent(e);
}

void MdiChild::insertSpacesAsTab(QTextCursor &cursor)
{
    int spaces = globalConfig->tabsSpacesSize - cursor.columnNumber() % globalConfig->tabsSpacesSize;
    for(;spaces>0;spaces--)
        cursor.insertText(" ");
    setTextCursor(cursor);
}


void MdiChild::removeSpacesAsTab(QTextCursor &cursor)
{
    QRegExp regex(" +");
    int spaces = cursor.columnNumber() % globalConfig->tabsSpacesSize;
    if( spaces == 0 )
        spaces = globalConfig->tabsSpacesSize;    
    if( cursor.columnNumber() < globalConfig->tabsSpacesSize )
        spaces = cursor.columnNumber();
    cursor.movePosition(QTextCursor::Left, QTextCursor::KeepAnchor, spaces);
    if( regex.exactMatch(cursor.selectedText()) ) {
        cursor.removeSelectedText();
        setTextCursor(cursor);
    }
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

    blockMode(false);
    QTextStream in(&file);
    in.setCodec(globalConfig->getEncoding().toLatin1().constData());
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
    out.setCodec(globalConfig->getEncoding().toLatin1().constData());
    QApplication::setOverrideCursor(Qt::WaitCursor);
    QString text = toPlainText();
    if(globalConfig->isTrailingSpacesWhenSave())
        text = removeTrailingSpaces(text);
    out << text;
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
        emit deleteDocument(this);
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
            //disconnect(docLayout, SIGNAL(docChanged(int, int, int)), this, SLOT(documentChanged(int, int, int)));
            disconnect(document(), SIGNAL(contentsChange(int, int, int)), this, SLOT(documentChanged(int, int, int)));
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
            connect(document(), SIGNAL(contentsChange(int, int, int)), this, SLOT(documentChanged(int, int, int)));
            connect(document(), SIGNAL(contentsChanged()), this, SLOT(documentContentsChanged()));
            if(syntaxHightlighter)
                syntaxHightlighter->rehighlight();
        }
        else
              blockMode(false);
    }
}

void MdiChild::documentContentsChanged()
{
    //setWindowModified(document()->isModified());
    // Keep view position when multiview mode is used.
    if(hasFocus())
    {
        ensureCursorVisible();
    }
    else if(this != _document->getLastView() || firstLine.block() != firstVisibleBlock())
    {
        // Ensure the first line will be visible if the focus has been lost (multiview mode).
        QTextBlock firstLineBlock = firstLine.block();
        verticalScrollBar()->setSliderPosition(firstLineBlock.firstLineNumber());
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
        QMessageBox::StandardButton ret = QMessageBox::warning(this, tr("MDI"),
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
    syntaxHightlighter->setFileName(fileName);
}

QString MdiChild::strippedName(const QString &fullFileName)
{
    return QFileInfo(fullFileName).fileName();
}


void MdiChild::setWindowTitle(const QString &name)
{
    QWidget::setWindowTitle(name);
    emit updateWindowName(this);
}


void MdiChild::setWindowModified(bool modifiedOk)
{
    if(isWindowModified() == modifiedOk)
        return;
    QWidget::setWindowModified(modifiedOk);
    emit updateWindowName(this);
}


void MdiChild::focusOutEvent(QFocusEvent * event)
{
    if(event->lostFocus())
    {
        // Set values in order to control multi-view
        QTextCursor cursor = textCursor();
        cursor.setPosition(firstVisibleBlock().position());
        firstLine = cursor;
        
        // If focus is lost, block mode will be be deactivated.
        // Blockmode could be out of control in multi-view mode. 
        blockMode(false);
    }
    QPlainTextEdit::focusOutEvent(event);
}

void MdiChild::focusInEvent(QFocusEvent * event)
{
    if(event->gotFocus())
    {
        _document->setLastView(this);
    }
    QPlainTextEdit::focusInEvent(event);
    if(event->gotFocus())
    {
        emit updateWindowName(this);
    }
}

void MdiChild::updateTabsSize()
{
    int charWidth = fontMetrics().horizontalAdvance("0");
    setTabStopDistance( charWidth*globalConfig->tabsSpacesSize );
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
    else
        emit showMessage(tr("Block mode: Disabled."));
}

void MdiChild::matchParenthesisPair()
{
    if(syntaxHightlighter != nullptr)
        syntaxHightlighter->matchParenthesis(textCursor().block(),
            textCursor().positionInBlock());
}

void MdiChild::enableSnippesMode(QString snippeText, QTextCursor cursor)
{
    QRegularExpression re("\\$(?<position>[0-9])");
    QRegularExpressionMatch match;
    int index;
    snippesMode.reset();
    snippesMode.cursorStartSnippe = cursor;
    snippesMode.cursorEndSnippe = cursor;
    insertPlainText(snippeText);
    snippesMode.cursorStartSnippe.movePosition(QTextCursor::Left, QTextCursor::MoveAnchor, snippeText.size()+1);
    snippesMode.cursorEndSnippe.movePosition(QTextCursor::Left, QTextCursor::MoveAnchor, 1);
    index = 0;
    while( (match=re.match(snippeText, index)).hasMatch() ) {
        index = match.capturedStart("position");
        if(index < 0)
            break;
        bool ok;
        int position = match.captured("position").toInt(&ok);
        if(! ok)
            continue;
        QTextCursor cursorMark = cursor;
        cursorMark.movePosition(QTextCursor::Left, QTextCursor::MoveAnchor, snippeText.length() - index + 1);
        snippesMode.cursorMarks[position].append(cursorMark);
    }
    if(! execNextSnippe(cursor))
        setTextCursor(cursor);
}

bool MdiChild::execNextSnippe(QTextCursor cursor)
{
    for(int index = 0; index < 10; index++) {
        int i = (index+1)%10;
        if(snippesMode.cursorMarks[i].isEmpty())
            continue;
        // Jump to the next position
        QTextCursor cursorFirst, cursorMark;
        cursorMark = cursorFirst = snippesMode.cursorMarks[i].takeFirst();
        cursorMark.movePosition(QTextCursor::Right, QTextCursor::KeepAnchor, 2);
        setTextCursor(cursorMark);
        if(snippesMode.lastPostionIndex == i) {
            snippesMode.cursorStartContent.movePosition(QTextCursor::Right, QTextCursor::MoveAnchor, 1);
            int start = snippesMode.cursorStartContent.position();
            int end = snippesMode.cursorEndContent.position();
            snippesMode.cursorStartContent.movePosition(QTextCursor::Right, QTextCursor::KeepAnchor, end-start);
            QString text = snippesMode.cursorStartContent.selectedText();
            insertPlainText(text);
            while(! snippesMode.cursorMarks[i].isEmpty()) {
                // Copy the contents of last position
                cursorMark = snippesMode.cursorMarks[i].takeFirst();
                cursorMark.movePosition(QTextCursor::Right, QTextCursor::KeepAnchor, 2);
                setTextCursor(cursorMark);
                insertPlainText(text);
            }
            continue;
        }
        if(! snippesMode.cursorMarks[i].isEmpty()) {
            snippesMode.cursorEndContent = cursorMark;
            snippesMode.cursorStartContent = cursorFirst;
            snippesMode.cursorStartContent.movePosition(QTextCursor::Left, QTextCursor::MoveAnchor, 1);
        }
        // Are more positions left?
        snippesMode.lastPostionIndex = i;
        snippesMode.isEnabled = false;
        for(i=0; i<10; i++)
            if(! snippesMode.cursorMarks[i].isEmpty()) {
                snippesMode.isEnabled = true;
                break;
            }
        if(! snippesMode.isEnabled)
            snippesMode.lastPostionIndex = -1;
        
        return true;
    }
    snippesMode.isEnabled = false;
    snippesMode.lastPostionIndex = -1;
    return false;
}

void MdiChild::SnippesMode::reset()
{
    isEnabled = false;
    lastPostionIndex = -1;
    for(int index = 0; index < 10; index++)
        cursorMarks[index].clear();
}

void MdiChild::updateTextTheme()
{
    TextThemes themes;
    TextTheme theme = themes.getTextTheme(globalConfig->getTextTheme(), QApplication::palette(this));
    QPalette pPalette = palette();
    pPalette.setColor(QPalette::Base, theme.background);
    pPalette.setColor(QPalette::Text, theme.foreground);
    pPalette.setColor(QPalette::HighlightedText, theme.selectionForeground);
    pPalette.setColor(QPalette::Highlight, theme.selectionBackground);
    setPalette(pPalette);
    syntaxHightlighter->updateTextTheme(&theme);
    
}

QString MdiChild::removeTrailingSpaces(QString text)
{
    QString buffer;
    QRegularExpression re("[ \\t\\n]+$");
    bool firstLine = true;
    for(QString line : text.split("\n")) {
        if(!firstLine) {
            buffer.append("\n");
        } else
            firstLine = false;
        buffer.append(line.remove(re));
    }
    return buffer;
}

PlainTextDocumentLayout::PlainTextDocumentLayout(QTextDocument *parent):QPlainTextDocumentLayout(parent)
{

}

void PlainTextDocumentLayout::documentChanged(int position, int charsRemoved, int charsAdded)
{
    QPlainTextDocumentLayout::documentChanged(position, charsRemoved, charsAdded);
    emit docChanged(position, charsRemoved, charsAdded);
}

