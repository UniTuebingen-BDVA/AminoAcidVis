/**
 *
 * @file   		RenderDockWidget.h
 * @author 		Vladimir Ageev (vladimir.agueev@progsys.de)
 * @date   		03.08.2017
 *
 * @brief The widget containing the GL rendering window.
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
#ifndef EXECUTABLES_AMINOVIS_DIALOGS_RENDERDOCKWIDGET_H_
#define EXECUTABLES_AMINOVIS_DIALOGS_RENDERDOCKWIDGET_H_

#include <ui_atomRenderWidget.h>
#include <QWidget>
#include <QDockWidget>

#include <Atoms/Atoms.h>
#include <Atoms/Timeline.h>
#include <ColorLibrary.h>
#include <QSettings>
#include <QComboBox>

class RenderDockWidget: public QDockWidget, public Ui::AtomRenderWidget{
	Q_OBJECT
public:
	//RenderDockWidget(QSettings& settings, QWidget *parent = nullptr);
	RenderDockWidget(QSettings& settings, Atoms* data, Tracker* frame, ColorLibrary* colorLib, FilterAtomsListModel* filter, QWidget *parent = nullptr);
	virtual ~RenderDockWidget();
private:
	QComboBox* m_styleComboBox = nullptr;
	QComboBox* m_scolorComboBox = nullptr;
};


#endif /* EXECUTABLES_AMINOVIS_DIALOGS_RENDERDOCKWIDGET_H_ */
