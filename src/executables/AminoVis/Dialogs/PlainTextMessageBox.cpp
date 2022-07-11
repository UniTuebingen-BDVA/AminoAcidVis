/*
 * PlainTextMessageBox.cpp
 *
 *  Created on: 27.06.2017
 *      Author: Vladimir Ageev (vladimir.agueev@progsys.de
 *
 * @copyright{
 *   AminoAcidVis
 *   Copyright (C) 2017 Vladimir Ageev
 *
 *   This library is free software; you can redistribute it and/or
 *   modify it under the terms of the GNU Lesser General Public
 *   License as published by the Free Software Foundation; either
 *   version 2.1 of the License, or (at your option) any later version.
 *
 *   This library is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *   Lesser General Public License for more details.
 *
 *   You should have received a copy of the GNU Lesser General Public
 *   License along with this library; if not, write to the Free Software
 *   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301
 *   USA
 *  }
 */

#include <Dialogs/PlainTextMessageBox.h>

PlainTextMessageBox::PlainTextMessageBox(const QString& title, const QString& text, QWidget *parent): QDialog(parent) {
	setupUi(this);
	setWindowTitle(title);

	plainTextEdit->setPlainText(text);

	connect(btnClose, SIGNAL(clicked()), this, SLOT(close()));
}

PlainTextMessageBox::~PlainTextMessageBox() {
	// TODO Auto-generated destructor stub
}

void PlainTextMessageBox::info(QWidget *parent, const QString& title, const QString& text){
	PlainTextMessageBox box(title, text, parent);
	box.exec();
}

void PlainTextMessageBox::qmlErrors(QWidget *parent, const QString& title, const QString& text, const QList<QQmlError>& errors){
	if(!errors.empty()){
		QString errorsText;
		for(const QQmlError& er : errors)
			errorsText += er.toString()+"\n";
		PlainTextMessageBox box(title, text+"\n"+errorsText, parent);
		box.exec();
	}
}

