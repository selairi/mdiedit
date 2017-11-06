/****************************************************************************
**
**   Copyright (C) 2015 P.L. Lucas
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

#include "snippesdialog.h"
#include <QSettings>
#include <QMessageBox>
#include <QFileDialog>
#include <QInputDialog>
#include <QDebug>

SnippesDialog::SnippesDialog(QHash<QString,QString> *snippes, bool activate, QWidget * parent):QDialog(parent) 
{
	ui.setupUi(this);
	connect(ui.addPushButton, SIGNAL(clicked()), this, SLOT(add()));
	connect(ui.removePushButton, SIGNAL(clicked()), this, SLOT(remove()));
	connect(ui.openPushButton, SIGNAL(clicked()), this, SLOT(open()));
	connect(ui.savePushButton, SIGNAL(clicked()), this, SLOT(save()));
	connect(ui.plainTextEdit, SIGNAL(textChanged()), this, SLOT(textChanged()));
	if(snippes!=NULL) {
		QHash<QString, QString>::const_iterator i = snippes->constBegin();
		while (i != snippes->constEnd()) {
			new QListWidgetItem(i.key(), ui.listWidget);
			this->snippes[i.key()] = i.value();
			++i;
		}
	}
	ui.snippesCheckBox->setChecked(activate);
	
	connect(ui.listWidget, SIGNAL(currentItemChanged(QListWidgetItem*,QListWidgetItem*)), 
		this, SLOT(changePage(QListWidgetItem*,QListWidgetItem*)));
	
	ui.listWidget->setToolTip(tr("Add triggers here and their text.\nThe text will be written when writing the trigger and press the Tab key.\nExample:\nIf you create a trigger called 'h' with text 'Hello world',\nwhen you type 'h' and press the Tab key,\n'Hello world' will be written."));
}

void SnippesDialog::changePage(QListWidgetItem*current,QListWidgetItem*previous)
{
	if(previous!=NULL)
		snippes[previous->text()]=ui.plainTextEdit->toPlainText();
	if(current!=NULL)
		ui.plainTextEdit->setPlainText(snippes[current->text()]);
}

void SnippesDialog::textChanged()
{
	QListWidgetItem *item=ui.listWidget->currentItem();
	if(item!=NULL) {
		snippes[item->text()]=ui.plainTextEdit->toPlainText();
	}
}

void SnippesDialog::add()
{
	bool ok;
	QString text = QInputDialog::getText(this, tr("Trigger"), tr("Name:"), QLineEdit::Normal, QString(), &ok);
	if (ok && !text.isEmpty()) {
		snippes[text]="";
		QListWidgetItem *item = new QListWidgetItem(text, ui.listWidget);
		ui.listWidget->setCurrentItem(item);
	}
}

void SnippesDialog::remove()
{
	ui.plainTextEdit->setPlainText("");
	QListWidgetItem *item=ui.listWidget->currentItem();
	if(item) {
		ui.listWidget->removeItemWidget(item);
		delete item;
	}
}

void SnippesDialog::open()
{
	QString fileName = QFileDialog::getOpenFileName(this, tr("Open"));
	if(fileName.isEmpty())
		return;
	QMessageBox msgBox;
	msgBox.setText(tr("Append"));
	msgBox.setInformativeText(tr("Do you want to append?"));
	msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
	msgBox.setDefaultButton(QMessageBox::No);
	int ret = msgBox.exec();
	if(ret==QMessageBox::No) {
		QList<QListWidgetItem *> list = ui.listWidget->findItems("*", Qt::MatchWildcard);
		foreach(QListWidgetItem *item, list) {
			ui.listWidget->removeItemWidget(item);
			delete item;
		}
	}
	QSettings settings(fileName, QSettings::IniFormat);
	QStringList keys = settings.childKeys();
	foreach(QString key, keys) {
		snippes[key]=settings.value(key).toString();
		new QListWidgetItem(key, ui.listWidget);
	}
}

void SnippesDialog::save()
{
	QString fileName = QFileDialog::getSaveFileName(this, tr("Save"));
	if(fileName.isEmpty())
		return;
	QSettings settings(fileName, QSettings::IniFormat);
	QStringList keys = settings.allKeys();
	foreach(QString key, keys) {
		settings.remove(key);
	}
	QHash<QString, QString>::const_iterator i = snippes.constBegin();
	while (i != snippes.constEnd()) {
		settings.setValue(i.key(), i.value());
		++i;
	}
}

bool SnippesDialog::getActivateSnippes()
{
	return ui.snippesCheckBox->isChecked();
}


