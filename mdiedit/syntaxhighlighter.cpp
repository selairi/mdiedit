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

#include <QFile>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QDebug>
#include "syntaxhighlighter.h"
#include "config.h"

// Static variables
QList<Syntax*> SyntaxHighlighter::syntaxList;
QHash<QChar,QChar> SyntaxHighlighter::parenthesisPair;

SyntaxHighlighter::SyntaxHighlighter(QObject *parent, const QPalette & palette, GlobalConfig *globalConfig):QSyntaxHighlighter(parent)
{
    this->globalConfig = globalConfig;
    tabPositionFormat.setBackground(QBrush(palette.color(QPalette::Disabled, QPalette::Highlight), Qt::Dense7Pattern));
    wordsFormat.setForeground(QBrush(palette.color(QPalette::Inactive, QPalette::Highlight)));
    wordsFormat.setFontWeight(QFont::Bold);
    commentsFormat.setForeground(QBrush(palette.color(QPalette::Disabled, QPalette::Highlight)));
    stringsFormat.setForeground(QBrush(palette.color(QPalette::Active, QPalette::Highlight)));
    syntax = nullptr;
    parenthesis = QRegularExpression("[\\(\\)\\[\\]\\{\\}]");
    startParenthesisList = QString("({[");
    if(parenthesisPair.isEmpty()) {
        parenthesisPair['('] = ')';
        parenthesisPair[')'] = '(';
        parenthesisPair['{'] = '}';
        parenthesisPair['}'] = '{';
        parenthesisPair['['] = ']';
        parenthesisPair[']'] = '[';
    }
    connect(globalConfig, SIGNAL(syntaxHighlightChanged(bool)), this, SLOT(rehighlight()));
}

QList<QRegularExpression> jsonToList(QJsonArray arrayJson)
{
    QList<QRegularExpression> list;
    for(QJsonValue item : arrayJson) {
        QRegularExpression regExp(item.toString());
        if(regExp.isValid()) {
            list.append(regExp);
        }
        else
            qWarning() << "Error in RegExp syntax" << item.toString() << regExp.errorString();
    }
    return list;
}

QList<SyntaxStartEnd> jsonToStartEndList(QJsonArray array)
{
    QList<SyntaxStartEnd> list;
    for(QJsonValue item : array) {
        SyntaxStartEnd se;
        QJsonObject obj = item.toObject();
        QRegularExpression regExpStart(obj.value("start").toString());
        if(! regExpStart.isValid()) {
            qWarning() << "Error in RegExp syntax" << obj.value("start").toString() << regExpStart.errorString();
            continue;
        }
        QRegularExpression regExpEnd(obj.value("end").toString());
        if(! regExpEnd.isValid()) {
            qWarning() << "Error in RegExp syntax" << obj.value("end").toString() << regExpEnd.errorString();
            continue;
        }
        se.start = regExpStart;
        se.end = regExpEnd;
        se.samePatternOk = (obj.value("start").toString() == obj.value("end").toString());
        if(obj.contains("words"))
            se.words = jsonToList(obj.value("words").toArray());
        if(obj.contains("comments"))
            se.comments = jsonToList(obj.value("comments").toArray());
        if(obj.contains("strings"))
            se.strings = jsonToList(obj.value("strings").toArray());
        list.append(se);
    }
    return list;
}

QList<Syntax*> SyntaxHighlighter::loadSyntaxFrom(QJsonDocument &json)
{
    QList<Syntax*> syntaxItems;
    
    if(!json.isArray()) {
        qWarning("Error in syntax.json. No item array.");
        return syntaxItems;
    }
    QJsonArray arrayJson = json.array();
    for(QJsonValue arrayItem : arrayJson) {
        QJsonObject obj = arrayItem.toObject();
        Syntax *s = new Syntax;;
        s->referenceCount = 0;
        s->title = obj.value("title").toString();
        s->fileType = jsonToList(obj.value("fileType").toArray());
        s->words = jsonToList(obj.value("words").toArray());
        s->comments = jsonToList(obj.value("comments").toArray());
        s->strings = jsonToList(obj.value("strings").toArray());
        s->wordsBlock = jsonToStartEndList(obj.value("wordsBlock").toArray());
        s->commentsBlock = jsonToStartEndList(obj.value("commentsBlock").toArray());
        s->stringsBlock = jsonToStartEndList(obj.value("stringsBlock").toArray());
        syntaxItems.append(s);
    }
    
    return syntaxItems;
}

QList<Syntax*> SyntaxHighlighter::loadSyntaxFromFile(QString fileName)
{
    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning("Couldn't open syntax file.");
        QList<Syntax*> syntaxItems;
        return syntaxItems;
    }
    QByteArray jsonFile = file.readAll();
    QJsonParseError jsonError;
    QJsonDocument doc = QJsonDocument::fromJson(jsonFile, &jsonError);
    if(jsonError.error != QJsonParseError::NoError) {
        int line = 1, i = 0;
        for(;i<jsonFile.length() && i<jsonError.offset;i++) {
            if(jsonFile.at(i) == '\n')
                line++;
        }
        qWarning() << "Json error in " << fileName
            << line << ":" << (jsonError.offset-i) << jsonError.errorString();
    }
    return loadSyntaxFrom(doc);
}

Syntax *SyntaxHighlighter::matchSyntaxToFileName(QString fileName, QList<Syntax*> list)
{
    for(Syntax *item: list) {
        for(const QRegularExpression fileType : item->fileType) {
            if(fileType.match(fileName).hasMatch()) {
                item->referenceCount++;
                return item;
            }
        }
    }
    return nullptr;
}

void SyntaxHighlighter::setFileName(QString fileName)
{
    if(syntax != nullptr)
        syntax->referenceCount--;
    syntax = matchSyntaxToFileName(fileName, syntaxList);
    if(syntax == nullptr) {
        // Syntax must be loaded from config file
        QList<Syntax*> list = loadSyntaxFromFile(QString(SYNTAX_PATH "/syntax.json"));
        //QList<Syntax*> list = loadSyntaxFromFile(QString("../syntax/syntax.json"));
        syntax = matchSyntaxToFileName(fileName, list);
        for(Syntax *item: list) {
            if(item->referenceCount == 0) {
                delete item;
            }
        }
        if(syntax != nullptr)
            syntaxList.append(syntax);
    }
    // Delete disabled syntax
    for(Syntax *item: syntaxList) {
        if(item->referenceCount <= 0) {
            syntaxList.removeOne(item);
            delete item;
            break;
        }
    }
    rehighlight();
}

void SyntaxHighlighter::setSyntax(QString syntaxName)
{
    if(syntax != nullptr) {
        if(syntax->title == syntaxName)
            return;
        else {
            syntax->referenceCount--;
            syntax = nullptr;
        }
    }
    bool foundSyntaxOk = false;
    for(Syntax *item: syntaxList) {
        if(item->title == syntaxName) {
            item->referenceCount++;
            syntax = item;
            foundSyntaxOk = true;
        }
    }
    if(!foundSyntaxOk) {
        QList<Syntax*> list = loadSyntaxFromFile(QString(SYNTAX_PATH "/syntax.json"));
        for(Syntax *item: list) {
            if(item->title == syntaxName) {
                item->referenceCount++;
                syntax = item;
                syntaxList.append(syntax);
                foundSyntaxOk = true;
            }
        }
        // Delete syntaxs which haven't been used
        for(Syntax *item: list) {
            if(item->referenceCount == 0) {
                delete item;
            }
        }
    }
    // Delete syntaxs which haven't been used
    for(Syntax *item: syntaxList) {
        if(item->referenceCount <= 0) {
            syntaxList.removeOne(item);
            delete item;
            break;
        }
    }
    rehighlight();
}


void SyntaxHighlighter::hightlightText(const QString & text, const QTextCharFormat & format, 
    const QList<QRegularExpression> & regList, int offset, FormatToApply *formatToApply)
{
    for(const QRegularExpression & re : regList) {
        QRegularExpressionMatch match = re.match(text, offset);
        if (match.hasMatch()) {
            bool isCapturedMainGroup;
            int capturedStart = match.capturedStart("main");
            if(capturedStart < 0) {
                isCapturedMainGroup = false;
                capturedStart = match.capturedStart();
            } else {
                isCapturedMainGroup = true;
            }
            if(formatToApply->offset == -1 || capturedStart < formatToApply->offset) { 
                formatToApply->offset = capturedStart;
                formatToApply->length = isCapturedMainGroup ? match.capturedLength("main") : match.capturedLength();
                formatToApply->format = format;
            }
        }
    }
}

void SyntaxHighlighter::hightlightText(const QString & text, const QTextCharFormat & format,
    const QList<SyntaxStartEnd> & regList, int &state, int offset, FormatToApply *formatToApply)
{
    for(const SyntaxStartEnd & re : regList) {
        state++; 
        
        if(offset == 0 && previousBlockState() != state  && previousBlockState() != BlockState::None)
            continue;
        
        int startIndex = 0;
        int endIndex = 0;
        
        QRegularExpressionMatchIterator startMatchIt = re.start.globalMatch(text, offset);
        QRegularExpressionMatchIterator endMatchIt = re.end.globalMatch(text, offset);
        
        if (previousBlockState() != state || offset > 0) {
            if(startMatchIt.hasNext()) {
                QRegularExpressionMatch match = startMatchIt.next();
                startIndex = match.capturedStart();
            } else {
                startIndex = -1;
            }
        }
        
        if(startIndex >= 0) {
            int endMatchedLength = 0;
            if(endMatchIt.hasNext()) {
                QRegularExpressionMatch match = endMatchIt.next();
                endIndex = match.capturedStart();
                endMatchedLength = match.capturedLength();
            } else {
                endIndex = -1;
            }
            if(re.samePatternOk) {
                if(previousBlockState() != state && endIndex == startIndex) {
                    if(endMatchIt.hasNext()) {
                        QRegularExpressionMatch match = endMatchIt.next();
                        endIndex = match.capturedStart();
                        endMatchedLength = match.capturedLength();
                    } else {
                        endIndex = -1;
                    }
                } else if(previousBlockState() == state && endIndex == startIndex) {
                    if(startIndex != 0) {
                        if(endMatchIt.hasNext()) {
                            QRegularExpressionMatch match = endMatchIt.next();
                            endIndex = match.capturedStart();
                            endMatchedLength = match.capturedLength();
                        } else {
                            endIndex = -1;
                        }
                    }
                }
            }
            int commentLength;
            if(formatToApply->offset == -1 || startIndex < formatToApply->offset) {
                formatToApply->offset = startIndex;
                if (endIndex == -1) {
                    formatToApply->state = state;
                    commentLength = text.length() - startIndex;
                } else {
                    formatToApply->state = BlockState::None;
                    commentLength = endIndex - startIndex + endMatchedLength;
                }
                formatToApply->length = commentLength;
                formatToApply->format = format;
                formatToApply->syntaxInside = re;
                formatToApply->syntaxInsideOk = true;
            }
        }
    }
}

void SyntaxHighlighter::matchParenthesis(QTextBlock block, int pos)
{
    // Remove previos highlighted parenthesis
    if(startParenthesis.pos > -1 || endParenthesis.pos > -1) {
        startParenthesis.pos = endParenthesis.pos = -1;
        rehighlightBlock(startParenthesisBlock);
        rehighlightBlock(endParenthesisBlock);
    } else
        startParenthesis.pos = endParenthesis.pos = -1;
    
    if(! globalConfig->isHighlightParenthesisMatch())
        return;
    TextBlockData *userData = (TextBlockData *)block.userData();
    if(userData == nullptr) {
        rehighlight();
        return;
    }
    userData = (TextBlockData *)block.userData();
    if(userData == nullptr) {
        return;
    }
    int i = 0;
    while(i < userData->parenthesis.length()) {
        Parenthesis p = userData->parenthesis.at(i);
        if(p.pos == pos) {
            startParenthesis = p;
            break;
        }
        i++;
    }
    if(startParenthesis.pos < 0)
        return;
    bool startOk = startParenthesisList.contains(startParenthesis.parenthesis);
    QChar parenthesisMatch = QChar(parenthesisPair[startParenthesis.parenthesis]);
    startParenthesisBlock = endParenthesisBlock = block;
    int count = 0;
    while(endParenthesisBlock.blockNumber() > -1) {
        TextBlockData *userData = (TextBlockData *)endParenthesisBlock.userData();
        if(userData == nullptr)
            continue;
        while(i > -1 && i < userData->parenthesis.length()) {
            Parenthesis p = userData->parenthesis.at(i);
            if(p.parenthesis == parenthesisMatch)
                count--;
            else if(p.parenthesis == startParenthesis.parenthesis)
                count++;
            if(count == 0) {
                endParenthesis = p;
                rehighlightBlock(startParenthesisBlock);
                rehighlightBlock(endParenthesisBlock);
                return;
            }
            if(startOk)
                i++;
            else
                i--;
        }
        if(startOk) {
            endParenthesisBlock = endParenthesisBlock.next();
            i = 0;
        } else {
            endParenthesisBlock = endParenthesisBlock.previous();
            userData = (TextBlockData *)endParenthesisBlock.userData();
            if(userData != nullptr) {
                i = userData->parenthesis.length() - 1;
            }
        }
    }
}

void SyntaxHighlighter::hightlightTextInside(const QString & text, const SyntaxStartEnd & syntaxInside, int offset, int final)
{
    FormatToApply formatToApply;
    while(offset >= 0 && offset < final) {
        formatToApply.offset = -1;
        hightlightText(text, wordsFormat, syntaxInside.words, offset, &formatToApply);
        hightlightText(text, stringsFormat, syntaxInside.strings, offset, &formatToApply);
        hightlightText(text, commentsFormat, syntaxInside.comments, offset, &formatToApply);
        if(formatToApply.offset >= 0 && formatToApply.offset < final)
            setFormat(formatToApply.offset, formatToApply.length, formatToApply.format);
        else
            break;
        offset = formatToApply.offset + formatToApply.length;
    }
}

void SyntaxHighlighter::highlightBlock(const QString & text)
{
    QList<Parenthesis> parenthesisList;
    if(globalConfig->isHighlightParenthesisMatch()) {
        QRegularExpressionMatchIterator parenthesisIt = parenthesis.globalMatch(text);
        while(parenthesisIt.hasNext()) {
            QRegularExpressionMatch match = parenthesisIt.next();
            Parenthesis p;
            p.pos = match.capturedStart();
            p.parenthesis = text.at(p.pos);
            parenthesisList.append(p);
        }
    }
    if(syntax != nullptr && globalConfig->isSyntaxHighlight()) {
        FormatToApply formatToApply;
        int offset = 0;
        while(offset >= 0) {
            formatToApply.offset = -1;
            formatToApply.state = BlockState::None;
            formatToApply.syntaxInsideOk = false;
            int state = BlockState::Other;
            hightlightText(text, wordsFormat, syntax->words, offset, &formatToApply);
            hightlightText(text, stringsFormat, syntax->strings, offset, &formatToApply);
            hightlightText(text, commentsFormat, syntax->comments, offset, &formatToApply);
            setCurrentBlockState(BlockState::None);
            hightlightText(text, wordsFormat, syntax->wordsBlock, state, offset, &formatToApply);
            hightlightText(text, stringsFormat, syntax->stringsBlock, state, offset, &formatToApply);
            hightlightText(text, commentsFormat, syntax->commentsBlock, state, offset, &formatToApply);
            if(formatToApply.offset >= 0) {
                setFormat(formatToApply.offset, formatToApply.length, formatToApply.format);
                // commentsBlock, wordsBlock or stringsBlock could contains words that should be
                // hightlighted:
                if(formatToApply.syntaxInsideOk) {
                    hightlightTextInside(text, formatToApply.syntaxInside, formatToApply.offset, formatToApply.offset+formatToApply.length);
                }
                if(formatToApply.state != BlockState::None) {
                    setCurrentBlockState(formatToApply.state);
                    offset = -1;
                } else { 
                    offset = formatToApply.offset + formatToApply.length;
                }
                if(globalConfig->isHighlightParenthesisMatch()) {
                    // Delete parenthesis which have got some kind of format
                    for(int i=0; i<parenthesisList.length();) {
                        Parenthesis p = parenthesisList[i];
                        if(p.pos >= formatToApply.offset && p.pos <= (formatToApply.offset + formatToApply.length - 1)) {
                            parenthesisList.takeAt(i);
                        } else
                            i++;
                    }
                }
            }
            else
                offset = -1;
        }
    }
    // Show spaces as tabs
    int i = 0;
    while( i < text.size() && text[i] == ' ') {
        if(i > 0 && i%4 == 0)
            setFormat(i, 1, tabPositionFormat);
        i++;
    }
    if(globalConfig->isHighlightParenthesisMatch()) {
        // Highlight parenthesis and add them to block user data
        TextBlockData *blockData = (TextBlockData*)currentBlockUserData();
        if(blockData == nullptr)
            blockData = new TextBlockData();
        blockData->parenthesis = parenthesisList;
        setCurrentBlockUserData(blockData);
        if(startParenthesisBlock == currentBlock() && startParenthesis.pos > -1) {
            for(const Parenthesis p : parenthesisList) {
                if(startParenthesis.pos == p.pos) {
                    setFormat(p.pos, 1, wordsFormat);
                    break;
                }
            }
        }
        if(endParenthesisBlock == currentBlock() && endParenthesis.pos > -1) {
            for(const Parenthesis p : parenthesisList) {
                if(endParenthesis.pos == p.pos) {
                    setFormat(p.pos, 1, wordsFormat);
                    break;
                }
            }
        }
    }
}


QList<Syntax*> SyntaxHighlighter::syntaxsList()
{
    QList<Syntax*> list = loadSyntaxFromFile(QString(SYNTAX_PATH "/syntax.json"));
    return list;
}


