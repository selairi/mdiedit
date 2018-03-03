/****************************************************************************
**
**   Copyright (C) 2018 P.L. Lucas
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

#include "spellchecktools.h"
#include <QDir>

static QString _getFirstPart(QString str, QString split)
{
    QStringList parts = str.split(split);
    if(parts.empty())
        return str;
    return parts[0];
}

SpellCheck::SpellCheck(QObject *parent):QObject(parent)
{
    spellChecker = nullptr;
    if(langs.empty()) {
        // Look for available dicts
        QDir dicts("/usr/share/hunspell/");
        QStringList filters;
        filters.append("*.dic");
        QStringList files = dicts.entryList(filters, QDir::Files, QDir::Name);
        for(QString file : files) {
            langs.append( _getFirstPart(file, ".") );
        }
    }
}

SpellCheck::~SpellCheck()
{
    if(spellChecker != nullptr)
        delete spellChecker;
}

bool SpellCheck::spell(const QString word)
{
    return spellChecker == nullptr || spellChecker->spell(word.toLatin1().toStdString());
}

const QStringList SpellCheck::getLangs()
{
    return langs;
}

void SpellCheck::setEnable(bool enable)
{
    if(enable && spellChecker == nullptr) {
        spellChecker = new Hunspell(QString("/usr/share/hunspell/"+lang+".aff").toLocal8Bit().data(), QString("/usr/share/hunspell/"+lang+".dic").toLocal8Bit().data());
    } else {
        if(spellChecker != nullptr) {
            delete spellChecker;
            spellChecker = nullptr;
        }
    }
}

bool SpellCheck::isEnabled()
{
    return spellChecker != nullptr;
}

bool SpellCheck::setLang(const QString & lang)
{
    bool ok = false;
    // Sometimes $LANG variable is like "es_ES.utf8".
    // Remove codification part:
    QString dict = _getFirstPart(lang, ".");
    if(langs.contains(dict)) {
        ok = true;
    } else {
        // The dictionary is not available.
        // Try to remove the country part: "es_ES" -> "es"
        dict = _getFirstPart(lang, "_");
        if(langs.contains(dict)) {
            ok = true;
        }
    }
    if(ok) {
        bool renable = spellChecker != nullptr && this->lang != dict;
        this->lang = dict;
        if(renable) {
            setEnable(false);
            setEnable(true);
        }
    }
    return ok;
}
