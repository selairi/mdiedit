/****************************************************************************
**
**   Copyright (C) 2014 P.L. Lucas
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

#include "finddialog.h"

FindDialog::FindDialog(QWidget * parent):QDialog(parent) {
	ui.setupUi(this);
	connect(ui.findButton, SIGNAL(clicked()), this, SLOT(findClicked()));
	connect(ui.replaceButton, SIGNAL(clicked()), this, SLOT(replaceClicked()));
	connect(ui.replaceAllButton, SIGNAL(clicked()), this, SLOT(replaceAllClicked()));
	connect(ui.closeButton, SIGNAL(clicked()), this, SLOT(accept()));
}

void FindDialog::findClicked() {
	emit find(ui.findLineEdit->text(), findFlags());
}

void FindDialog::replaceClicked() {
	emit replace(ui.findLineEdit->text(), ui.replaceLineEdit->text(), findFlags());
}

void FindDialog::replaceAllClicked() {
	emit replaceAll(ui.findLineEdit->text(), ui.replaceLineEdit->text(), findFlags());
}

QTextDocument::FindFlags FindDialog::findFlags() {
	QTextDocument::FindFlags flags = 0;
	if(ui.sensitiveCheckBox->isChecked())
		flags |= QTextDocument::FindCaseSensitively;
	if(ui.backwardsCheckBox->isChecked())
		flags |= QTextDocument::FindBackward;
	return flags;
}

QString FindDialog::text()
{
	return ui.findLineEdit->text();
}

void FindDialog::showDialog() {
	ui.findLineEdit->setFocus(Qt::ActiveWindowFocusReason);
	show();
}

