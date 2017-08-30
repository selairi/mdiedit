/****************************************************************************
**
**   Copyright (C) 2016 P.L. Lucas
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

#ifndef SYNTAXHIGHLIGHTER_H
#define SYNTAXHIGHLIGHTER_H

#include <QSyntaxHighlighter>
#include <QList>
#include <QRegularExpression>
#include "globalconfig.h"
#include "document.h"
#include "textblockdata.h"

struct Syntax;
struct SyntaxStartEnd;

class SyntaxHighlighter:public QSyntaxHighlighter
{
    Q_OBJECT

public:
    SyntaxHighlighter(QObject * parent, const QPalette & palette, GlobalConfig *globalConfig);
    void highlightBlock(const QString & text);
    void matchParenthesis(QTextBlock block, int pos);
    
    GlobalConfig *globalConfig;

public slots:
    void setFileName(QString fileName);

private:
    enum BlockState {None = -1, Other};
    
    QList<Syntax*> loadSyntaxFromFile(QString file);
    QList<Syntax*> loadSyntaxFrom(QJsonDocument &json);
    Syntax *matchSyntaxToFileName(QString fileName, QList<Syntax*> list);

    struct FormatToApply {
        int offset, length;
        QTextCharFormat format;
        int state;
    };
    void hightlightText(const QString & text, const QTextCharFormat & format, 
        const QList<QRegularExpression> & regList, int offset, FormatToApply *formatToApply);
    void hightlightText(const QString & text, const QTextCharFormat & format, 
        const QList<SyntaxStartEnd> & regList, int &state, int offset, FormatToApply *formatToApply);

    QTextCharFormat tabPositionFormat;
    QTextCharFormat wordsFormat;
    QTextCharFormat commentsFormat;
    QTextCharFormat stringsFormat;
    static QList<Syntax*> syntaxList;
    Syntax *syntax;
    
    QRegularExpression parenthesis;
    QString startParenthesisList;
    QTextBlock startParenthesisBlock, endParenthesisBlock;
    Parenthesis startParenthesis, endParenthesis;
    static QHash<QChar,QChar> parenthesisPair;
};

struct SyntaxStartEnd
{
    QRegularExpression start, end;
    bool samePatternOk;
};

struct Syntax 
{
    int referenceCount;
    QString title;
    QList<QRegularExpression> fileType;
    QList<QRegularExpression> words;
    QList<QRegularExpression> comments;
    QList<QRegularExpression> strings;
    QList<SyntaxStartEnd> wordsBlock;
    QList<SyntaxStartEnd> commentsBlock;
    QList<SyntaxStartEnd> stringsBlock;
};

#endif