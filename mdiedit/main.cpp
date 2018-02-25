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

#include <QApplication>
#include <QTranslator>
#include <QLibraryInfo>
#include <QDebug>
#include <stdlib.h> 
#include "mainwindow.h"
#include "config.h"

int main(int argn, char *argv[])
{
	QApplication app(argn, argv);
	
	QTranslator qtTranslator;
	qtTranslator.load(QString(TRANSLATIONS_PATH "/mdiedit-") + QLocale::system().name()+ QString(".qm"));
	app.installTranslator(&qtTranslator);
	
	// qt translation for default dialogs (QFileDialog) and so on
     QTranslator qtTranslator2;
	qtTranslator2.load("qt_" + QLocale::system().name(), QLibraryInfo::location(QLibraryInfo::TranslationsPath));
	app.installTranslator(&qtTranslator2);

	qDebug() << QLibraryInfo::location(QLibraryInfo::TranslationsPath) << "  " << QLocale::system().name();
	
	QStringList fileList;
	for(int i=1;i<argn;i++) {
		fileList << argv[i];
	}
	
	MainWindow mainWin;
	// Init spell check
	SpellCheck *spellChecker = new SpellCheck(&mainWin);
	{
	   char *lang = getenv("LANG");
	   if(lang != nullptr) {
	       spellChecker->setLang(lang);
	       spellChecker->setEnable(true);
	   }
	}
	mainWin.setSpellChecker(spellChecker);
	mainWin.setWindowIcon(QIcon(ICON_PATH "/hicolor/scalable/apps/mdiedit.svg"));
	mainWin.show();
	QString file;
	foreach(file, fileList)
		mainWin.open(file);
	return app.exec();
}
