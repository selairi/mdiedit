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

#include "globalconfig.h"

GlobalConfig::GlobalConfig(QObject *parent):QObject(parent)
{
    snippesActivateOk = replaceTabsBySpacesOk = trailingSpacesWhenSaveOk = false;
    syntaxHighlightOk = highlightParenthesisMatchOk = true;
    autoindent = true;
    tabsSpacesSize = 4;
    spellChecker = nullptr;
    encoding = "UTF-8";
}

void GlobalConfig::setSyntaxHighlight(bool activatedOk)
{
    syntaxHighlightOk = activatedOk;
    emit syntaxHighlightChanged(activatedOk);
}

void GlobalConfig::setHighlightParenthesisMatch(bool activatedOk)
{
    highlightParenthesisMatchOk = activatedOk;
}

SpellCheck *GlobalConfig::getSpellCheck()
{
    return spellChecker;
}

void GlobalConfig::setSpellCheck(SpellCheck *spellChecker)
{
    this->spellChecker = spellChecker;
}

QString GlobalConfig::getEncoding()
{
    return this->encoding;
}

void GlobalConfig::setEncoding(QString encoding)
{
    if(! encoding.isEmpty())
        this->encoding = encoding;
}


QString GlobalConfig::getTextTheme()
{
    return this->textTheme;
}

void GlobalConfig::setTextTheme(QString textTheme)
{
    if(textTheme.isEmpty())
        textTheme = "Default";
    this->textTheme = textTheme;
}


void GlobalConfig::setTrailingSpacesWhenSave(bool activatedOk)
{
    trailingSpacesWhenSaveOk = activatedOk;
}