/*
 * DebugDockWidget.h
 *
 *  Created on: 21.05.2017
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

#ifndef EXECUTABLES_AMINOVIS_DEBUGDOCKWIDGET_H_
#define EXECUTABLES_AMINOVIS_DEBUGDOCKWIDGET_H_

#include <ui_debugDockWidget.h>
#include <QDockWidget>
#include <Atoms/Atoms.h>
#include <Atoms/Timeline.h>

class DebugDockWidget : public QDockWidget, public Ui::debugDockWidget{
	Q_OBJECT
public:
	DebugDockWidget(Atoms* data, Timeline* timeline, QWidget *parent = nullptr);
	virtual ~DebugDockWidget();
signals:
	void updateAtomPositionGL(int);
	void updateAtomRadiusGL(int);
private:
	Atoms* m_data;
	Timeline* m_timeline;
};

#endif /* EXECUTABLES_AMINOVIS_DEBUGDOCKWIDGET_H_ */
