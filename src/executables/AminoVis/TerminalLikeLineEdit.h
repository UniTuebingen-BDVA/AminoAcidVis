/*!
 * TerminalLikeLineEdit.h
 *
 *  Created on: 12.08.2017
 *      Author: Vladimir Ageev (vladimir.agueev@progsys.de
 *
 * @brief Implements a terminal like line edit box.
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

#ifndef EXECUTABLES_AMINOVIS_TERMINALLIKELINEEDIT_H_
#define EXECUTABLES_AMINOVIS_TERMINALLIKELINEEDIT_H_

#include <QLineEdit>
#include <QList>
#include <QString>

/*!
 * @brief Expands upon QLineEdit to save previous submitted commands.
 * Similar to a terminal previous submitted commands can be recalled using the up and down arrow keys.
 * @see submit
 */
class TerminalLikeLineEdit: public QLineEdit {
	Q_OBJECT
public:
	TerminalLikeLineEdit(QWidget* parent=0);
	virtual ~TerminalLikeLineEdit();

	/*!
	 * @brief Submit current entered text.
	 * When this method is called the text inside the line edit box will be pushed back as an entry inside a QString list.
	 * Also the index location will be reset to the end of the list, which is equal to position of the new entry.
	 * Note: The line edit box will not be cleared.
	 */
	void submit();
	/*!
	 * @brief The up and down arrow keys are captured to recall previous submitted commands.
	 * @see submit
	 */
	void keyPressEvent(QKeyEvent *event);
private:
	/// Current location
	int m_index = 0;
	/// Submitted command storage
	QList<QString> m_lastEntries;
};

#endif /* EXECUTABLES_AMINOVIS_TERMINALLIKELINEEDIT_H_ */
