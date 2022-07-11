/**
 * @file   		FrameContolWidget.h
 * @author 		Vladimir Ageev (vladimir.agueev@progsys.de)
 * @date   		04.08.2017
 *
 * @brief  		Widgets containing the frame controls of a tracker.
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
#ifndef EXECUTABLES_AMINOVIS_DIALOGS_FRAMECONTOLWIDGET_H_
#define EXECUTABLES_AMINOVIS_DIALOGS_FRAMECONTOLWIDGET_H_

#include <QFrame>
#include <ui_frameControlWidget.h>
#include <QListWidgetItem>

#include <Atoms/Timeline.h>

class FrameContolWidget: public QFrame, public Ui::FrameControlFrame {
	Q_OBJECT
public:
	FrameContolWidget(Tracker* frame, QWidget *parent = 0);
	virtual ~FrameContolWidget();

public slots:
	void dockVisibilityChanged(bool visibility);
signals:
	void toggleDockVisibility(bool visibility);
private:
	bool m_dockVisibility = true;
};

#endif /* EXECUTABLES_AMINOVIS_DIALOGS_FRAMECONTOLWIDGET_H_ */
