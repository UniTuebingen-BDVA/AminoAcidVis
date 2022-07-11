/**
 *
 * @file   		PlainTextMessageBox.h
 * @author 		Vladimir Ageev (vladimir.agueev@progsys.de)
 * @date   		 27.06.2017
 *
 * @brief A simple message box with a plain text widget to support longer text.
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
#ifndef INCLUDE_SPRITESHEETEDITOR_COMPILEERRORBOX_H_
#define INCLUDE_SPRITESHEETEDITOR_COMPILEERRORBOX_H_

#include <ui_plainTextBox.h>
#include <QList>
#include <QQmlError>
/*!
 * @brief A simple message box with a plain text widget to support longer text.
 */
class PlainTextMessageBox: public QDialog, public Ui::PlainTextBox {
public:
	PlainTextMessageBox(const QString& title, const QString& text, QWidget *parent = 0);
	virtual ~PlainTextMessageBox();

	static void info(QWidget *parent, const QString& title, const QString& text);
	static void qmlErrors(QWidget *parent, const QString& title, const QString& text, const QList<QQmlError>& errors);
};

#endif /* INCLUDE_SPRITESHEETEDITOR_COMPILEERRORBOX_H_ */
