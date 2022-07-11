/*!
 *
 * @file   		DropDownArrow.h
 * @author 		Vladimir Ageev (vladimir.agueev@progsys.de)
 * @date   		16.09.2017
 *
 * @brief A QComboBox with a custom paintEvent to only display its arrow.
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

#ifndef EXECUTABLES_AMINOVIS_DROPDOWNARROW_H_
#define EXECUTABLES_AMINOVIS_DROPDOWNARROW_H_

#include <QComboBox>
#include <QStylePainter>

/*!
 * @brief A QComboBox with a custom paintEvent to only display its arrow.
 *
 * This saves space.
 */
class DropDownArrow: public QComboBox {
public:
	DropDownArrow(QWidget *parent);
	virtual ~DropDownArrow();

	///Custom paint event, to only draw an arrow.
	void paintEvent(QPaintEvent *);
};

#endif /* EXECUTABLES_AMINOVIS_DROPDOWNARROW_H_ */
