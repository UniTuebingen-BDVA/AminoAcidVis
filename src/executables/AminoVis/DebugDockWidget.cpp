/*
 * DebugDockWidget.cpp
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

#include <DebugDockWidget.h>
#include <Atoms/ProteinSurface.h>

DebugDockWidget::DebugDockWidget(Atoms* data, Timeline* timeline, QWidget *parent): QDockWidget(parent), m_data(data), m_timeline(timeline) {
	setupUi(this);


	connect(m_data, (&Atoms::selectionChanged), this, [this](){
		if(m_data->getSelectedAtom() >= 0){
			const glm::vec3& pos = m_data->getFrame(m_timeline->getFrame(0)).positions[m_data->getSelectedAtom()];
			xPosDoubleSpinBox->blockSignals(true);
			yPosDoubleSpinBox->blockSignals(true);
			zPosDoubleSpinBox->blockSignals(true);
			radiusDoubleSpinBox->blockSignals(true);

			xPosDoubleSpinBox->setValue(pos.x);
			yPosDoubleSpinBox->setValue(pos.y);
			zPosDoubleSpinBox->setValue(pos.z);

			radiusDoubleSpinBox->setValue(m_data->getAtom(m_data->getSelectedAtom()).radius);

			xPosDoubleSpinBox->blockSignals(false);
			yPosDoubleSpinBox->blockSignals(false);
			zPosDoubleSpinBox->blockSignals(false);
			radiusDoubleSpinBox->blockSignals(false);
		}
	});

	//set atoms position
	connect(xPosDoubleSpinBox, static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged), this, [this](double value){
		if(m_data->getSelectedAtom() >= 0){
			m_data->getFrame(m_timeline->getFrame(0)).positions[m_data->getSelectedAtom()].x = value;
			emit updateAtomPositionGL(m_data->getSelectedAtom());
		}
	});
	connect(yPosDoubleSpinBox, static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged), this, [this](double value){
		if(m_data->getSelectedAtom() >= 0){
			m_data->getFrame(m_timeline->getFrame(0)).positions[m_data->getSelectedAtom()].y = value;
			emit updateAtomPositionGL(m_data->getSelectedAtom());
		}
	});
	connect(zPosDoubleSpinBox, static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged), this, [this](double value){
		if(m_data->getSelectedAtom() >= 0){
			m_data->getFrame(m_timeline->getFrame(0)).positions[m_data->getSelectedAtom()].z = value;
			emit updateAtomPositionGL(m_data->getSelectedAtom());
		}
	});

	//radius
	connect(radiusDoubleSpinBox, static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged), this, [this](double value){
		if(m_data->getSelectedAtom() >= 0){
			m_data->getAtom(m_data->getSelectedAtom()).radius = value;
			emit updateAtomRadiusGL(m_data->getSelectedAtom());
		}
	});

	connect(rebuildAABBPushButton,&QPushButton::clicked, this, [this](){
		TrajectoryStream::xtcFrame& frame = m_data->getFrame(m_timeline->getFrame(0));

		frame.box.min = glm::vec3(std::numeric_limits<float>::max(),std::numeric_limits<float>::max(),std::numeric_limits<float>::max());
		frame.box.max = glm::vec3(std::numeric_limits<float>::min(),std::numeric_limits<float>::min(),std::numeric_limits<float>::min());
		for(const glm::vec3& position: frame.positions){
			if(position.x < frame.box.min.x) frame.box.min.x = position.x;
			if(position.y < frame.box.min.y) frame.box.min.y = position.y;
			if(position.z < frame.box.min.z) frame.box.min.z = position.z;

			if(position.x > frame.box.max.x) frame.box.max.x = position.x;
			if(position.y > frame.box.max.y) frame.box.max.y = position.y;
			if(position.z > frame.box.max.z) frame.box.max.z = position.z;
		}
	});


	//test intersection
	connect(copySelectedToCPushButton,&QPushButton::clicked, this, [this](){
		cSelSpinBox->setValue(m_data->getSelectedAtom());
	});
	connect(copySelectedToAPushButton,&QPushButton::clicked, this, [this](){
		aSelSpinBox->setValue(m_data->getSelectedAtom());
	});
	connect(copySelectedToBPushButton,&QPushButton::clicked, this, [this](){
		bSelSpinBox->setValue(m_data->getSelectedAtom());
	});

	connect(testIntersectionPushButton,&QPushButton::clicked, this, [this](){
		if(
				cSelSpinBox->value() >= 0 && cSelSpinBox->value() < m_data->numberOfAtroms() &&
				aSelSpinBox->value() >= 0 && aSelSpinBox->value() < m_data->numberOfAtroms() &&
				bSelSpinBox->value() >= 0 && bSelSpinBox->value() < m_data->numberOfAtroms()
		){
			QString out;
			debugFindEndPoints(out, m_data->getAtoms(), m_data->getFrame(m_timeline->getFrame(0)), 1.4f, cSelSpinBox->value(), aSelSpinBox->value(), bSelSpinBox->value());
			qDebug()<<out;
		}
	});


}

DebugDockWidget::~DebugDockWidget() {
	// TODO Auto-generated destructor stub
}

