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

#ifndef MDICHILD_H
#define MDICHILD_H

#include <QPlainTextEdit>
#include <QStringListModel>
#include <QHash>
#include "document.h"
#include "syntaxhighlighter.h"
#include "globalconfig.h"

class PlainTextDocumentLayout;

class MdiChild : public QPlainTextEdit
{
    Q_OBJECT

public:
    MdiChild(GlobalConfig *globalConfig, QWidget *parent);

    void newFile();
    bool loadFile(const QString &fileName);
    bool save();
    bool saveAs();
    bool saveFile(const QString &fileName);
    QString userFriendlyCurrentFile();
    QString currentFile() { return _document->fileName(); }
    void setView(MdiChild *mdiChild);
    Document *view();
    void completion();
    void blockMode(bool enableOk = true);
    void setWindowTitle(const QString &name);
    void setWindowModified(bool modifiedOk);
    inline SyntaxHighlighter *getSyntaxHightlighter() {return syntaxHightlighter;};

    QHash<QString,QString> *snippes;
    GlobalConfig *globalConfig;

signals:
    void reparentDocument(Document *);
    void showMessage(QString);
    void updateWindowName(MdiChild *mdichild);
    void deleteDocument(MdiChild *mdichild);

public slots:
    void setCurrentFile(QString fileName);
    void updateTabsSize();
    void updateTextTheme();

protected:
    void closeEvent(QCloseEvent *event);
    void keyPressEvent(QKeyEvent * e);
    void focusOutEvent(QFocusEvent * event);
    void focusInEvent(QFocusEvent * event);

private slots:
    void documentWasModified();
    void documentContentsChanged();
    void documentChanged(int position, int charsRemoved, int charsAdded);
    void matchParenthesisPair();

private:
    bool maybeSave();
    QString strippedName(const QString &fullFileName);
    QString removeTrailingSpaces(QString text);
    
    bool isUntitled;
    bool autoindent;
    Document *_document;
    QTextCursor firstLine; // First line show in view mode
    PlainTextDocumentLayout *docLayout;
    void insertSpacesAsTab(QTextCursor &cursor);
    void removeSpacesAsTab(QTextCursor &cursor);
    SyntaxHighlighter *syntaxHightlighter;
    
    struct BlockMode {
        bool enabled;
        int lineNumber;
        int startLine;
        int endLine;
    } blockModeData;
    
    struct SnippesMode {
        QList<QTextCursor> cursorMarks[10];
        bool isEnabled;
        int lastPostionIndex;
        QTextCursor cursorStartContent, cursorEndContent;
        QTextCursor cursorStartSnippe, cursorEndSnippe;
        void reset();
    } snippesMode;
    void enableSnippesMode(QString snippeText, QTextCursor cursor);
    bool execNextSnippe(QTextCursor cursor);
};

class PlainTextDocumentLayout: public QPlainTextDocumentLayout
{
Q_OBJECT
public:
    PlainTextDocumentLayout(QTextDocument *parent);

signals:
    void docChanged(int position, int charsRemoved, int charsAdded);

protected:
    virtual void documentChanged(int position, int charsRemoved, int charsAdded);
};

#endif
