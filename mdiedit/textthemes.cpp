/****************************************************************************
**
**   Copyright (C) 2019 P.L. Lucas
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
#include <QDebug>
#include <QJsonArray>
#include <QJsonObject>
#include "textthemes.h"
#include "config.h"

TextThemes::TextThemes()
{
}

/*
QStringList TextThemes::themeNames()
{
    QStringList themes;
    themes.append("Default");
    themes.append("DarkPastels");
    return themes;
}

TextTheme TextThemes::getTextTheme(QString name, QPalette defaultPalette)
{
    TextTheme theme;
    if(name == "DarkPastels") {
        theme.background = QColor("#293134");
        theme.foreground = QColor("#E0E2E4");
        theme.selectionForeground = QColor("#E0E2E4");
        theme.selectionBackground = QColor("#804000");
        theme.tabPosition.setBackground(QBrush(QColor("#A6A684"), Qt::Dense7Pattern));
        theme.words.setForeground(QBrush(QColor("#93C763")));
        theme.words.setFontWeight(QFont::Bold);
        theme.comments.setForeground(QBrush(QColor("#A6A684")));
        theme.strings.setForeground(QBrush(QColor("#2AA198")));
        theme.spellCheck.setFontUnderline(true);
        theme.spellCheck.setUnderlineColor(QColor("#D20019"));
        theme.spellCheck.setUnderlineStyle(QTextCharFormat::SpellCheckUnderline);
    } else {
        theme.background = defaultPalette.color(QPalette::Base);
        theme.foreground = defaultPalette.color(QPalette::Text);
        theme.selectionForeground = defaultPalette.color(QPalette::HighlightedText);
        theme.selectionBackground = defaultPalette.color(QPalette::Highlight);
        theme.tabPosition.setBackground(QBrush(defaultPalette.color(QPalette::Disabled, QPalette::Highlight), Qt::Dense7Pattern));
        theme.words.setForeground(QBrush(defaultPalette.color(QPalette::Inactive, QPalette::Highlight)));
        theme.words.setFontWeight(QFont::Bold);
        theme.comments.setForeground(QBrush(defaultPalette.color(QPalette::Disabled, QPalette::Highlight)));
        theme.strings.setForeground(QBrush(defaultPalette.color(QPalette::Active, QPalette::Highlight)));
        theme.spellCheck.setFontUnderline(true);
        theme.spellCheck.setUnderlineColor(QColor("red"));
        theme.spellCheck.setUnderlineStyle(QTextCharFormat::SpellCheckUnderline);
    }
    return theme;
}
*/

QStringList TextThemes::themeNames()
{
    QStringList themes;
    themes = readTextThemeNamesFromJSON(SHARE_PATH "/textthemes.json");
    themes.prepend("Default");
    return themes;
}

TextTheme TextThemes::getTextTheme(QString name, QPalette defaultPalette)
{
    bool ok;
    TextTheme theme = readTextThemeFromJSON(SHARE_PATH "/textthemes.json", name, &ok);
    
    if(! ok) {
        theme.background = defaultPalette.color(QPalette::Base);
        theme.foreground = defaultPalette.color(QPalette::Text);
        theme.selectionForeground = defaultPalette.color(QPalette::HighlightedText);
        theme.selectionBackground = defaultPalette.color(QPalette::Highlight);
        theme.tabPosition.setBackground(QBrush(defaultPalette.color(QPalette::Disabled, QPalette::Highlight), Qt::Dense7Pattern));
        theme.words.setForeground(QBrush(defaultPalette.color(QPalette::Inactive, QPalette::Highlight)));
        theme.words.setFontWeight(QFont::Bold);
        theme.comments.setForeground(QBrush(defaultPalette.color(QPalette::Disabled, QPalette::Highlight)));
        theme.strings.setForeground(QBrush(defaultPalette.color(QPalette::Active, QPalette::Highlight)));
        theme.spellCheck.setFontUnderline(true);
        theme.spellCheck.setUnderlineColor(QColor("red"));
        theme.spellCheck.setUnderlineStyle(QTextCharFormat::SpellCheckUnderline);
    }
    
    return theme;
}

QJsonDocument TextThemes::openJSON(const char *path)
{
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning("Couldn't open text themes file.");
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
        qWarning() << "Json error in " << path
            << line << ":" << (jsonError.offset-i) << jsonError.errorString();
    }
    return doc;
}

QStringList TextThemes::readTextThemeNamesFromJSON(const char *path)
{
    QStringList themeNames;
    QJsonDocument json = openJSON(path);
    
    if(!json.isArray()) {
        qWarning("Error in textthemes.json. No item array.");
        return themeNames;
    }
    
    QJsonArray arrayJson = json.array();
    for(QJsonValue arrayItem : arrayJson) {
        QJsonObject obj = arrayItem.toObject();
        themeNames.append(obj.value("title").toString());
    }
    
    return themeNames;
}

TextTheme TextThemes::readTextThemeFromJSON(const char *path, QString name, bool *ok)
{
    TextTheme theme;
    *ok = false;
    QJsonDocument json = openJSON(path);
    
    if(!json.isArray()) {
        qWarning("Error in textthemes.json. No item array.");
        return theme;
    }
    
    
    QJsonArray arrayJson = json.array();
    for(QJsonValue arrayItem : arrayJson) {
        QJsonObject obj = arrayItem.toObject();
        if(name == obj.value("title").toString()) {
            *ok = true;
            theme.background = QColor(obj.value("background").toString());
            theme.foreground = QColor(obj.value("foreground").toString());
            theme.selectionForeground = QColor(obj.value("selectionForeground").toString());
            theme.selectionBackground = QColor(obj.value("selectionBackground").toString());
            theme.tabPosition.setBackground(QBrush(QColor(obj.value("tabPosition").toString()), Qt::Dense7Pattern));
            theme.words.setForeground(QBrush(QColor(obj.value("words").toString())));
            theme.words.setFontWeight(QFont::Bold);
            theme.comments.setForeground(QBrush(QColor(obj.value("comments").toString())));
            theme.strings.setForeground(QBrush(QColor(obj.value("strings").toString())));
            theme.spellCheck.setFontUnderline(true);
            theme.spellCheck.setUnderlineColor(QColor(obj.value("spellCheck").toString()));
            theme.spellCheck.setUnderlineStyle(QTextCharFormat::SpellCheckUnderline);
            
            return theme;
        }
    }
    
    return theme;
}