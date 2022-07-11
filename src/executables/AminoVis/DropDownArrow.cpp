/*
 * DropDownArrow.cpp
 *
 *  Created on: 16.09.2017
 *      Author: Vladimir Ageev
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

#include <DropDownArrow.h>
#include <QStylePainter>
#include <QDebug>
#include <QState>
#include <QStyleOption>
#include <QStyleOptionComboBox>

DropDownArrow::DropDownArrow(QWidget *parent): QComboBox(parent) {
	// TODO Auto-generated constructor stub

}

DropDownArrow::~DropDownArrow() {
	// TODO Auto-generated destructor stub
}
QT_BEGIN_NAMESPACE
void DropDownArrow::paintEvent(QPaintEvent *)
{
    QStylePainter painter(this);
    painter.setPen(palette().color(QPalette::Text));
    QStyleOptionComboBox opt;
    initStyleOption(&opt);
    painter.drawPrimitive(QStyle::PE_IndicatorArrowDown, opt);
}
QT_END_NAMESPACE
