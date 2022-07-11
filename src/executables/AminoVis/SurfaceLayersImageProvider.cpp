/*
 * SurfaceLayersImageProvider.cpp
 *
 *  Created on: 08.05.2017
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

#include <SurfaceLayersImageProvider.h>
#include <QDebug>
#include <stdexcept>
#include <Atoms/FilterNode.h>

SurfaceLayersImageProvider::SurfaceLayersImageProvider(Atoms* data, Timeline* timeline, ColorLibrary* colors):
	QQuickImageProvider(QQmlImageProviderBase::Image), m_data(data), m_timeline(timeline), m_colors(colors) {
	if(!data || !timeline || !colors) throw std::invalid_argument("[SurfaceLayersImageProvider] One of the parameters is nullptr!");
}

inline QImage getBGImage(const QColor& bgColor){
	QImage img(1,1, QImage::Format_RGB32);
	rawcolor c = {(unsigned char)bgColor.blue(), (unsigned char)bgColor.green(), (unsigned char)bgColor.red(), 0};
	(((int*)img.bits())[0]) = c.data;
	return img;
}

QImage SurfaceLayersImageProvider::requestImage(const QString &id, QSize* size, const QSize &requestedSize){
	const QColor& bgColor = m_colors->getWindow();
	if(id.isEmpty()) return getBGImage(bgColor);
	QStringList paras = id.split("-");
	if(paras.size() != 2) return QImage(1,1, QImage::Format_RGB32);

	return requestImage(paras[0] == "residue", paras[1].toInt(), m_colors->getLayerColors(true), size, requestedSize);
}


QImage SurfaceLayersImageProvider::requestImage(bool residue, int id, const ColorsRow& colors, QSize *size, const QSize &requestedSize){
	const QColor& bgColor = m_colors->getWindow();
	if(m_timeline->getStartFrame() > m_timeline->getEndFrame() || m_data->numberOfAtroms() == 0 || m_data->numberOfFrames() == 0 || id < 0 || requestedSize.width() < 0 || requestedSize.height() < 0){
		return getBGImage(bgColor);
	}

	//qDebug()<<__LINE__<<" requestedSize: "<<requestedSize;
	if(residue){
		if(id >= m_data->numberOfGroups()) return getBGImage(bgColor);
		//ignore water
		{
			const QString groupName = m_data->getGroupName(id);
			if(groupName == "HOH" || groupName.toLower() == "water") return getBGImage(bgColor);
		}
		QImage heatmap(requestedSize, QImage::Format_RGB32); //(0xffRRGGBB)
		if(size) *size = QSize(requestedSize);
		const int timeSteps =  m_timeline->getEndFrame() - m_timeline->getStartFrame();
		const float step = timeSteps/(float)requestedSize.width();
		float currentFrame = m_timeline->getStartFrame();
		for(int x = 0; x < requestedSize.width();){
			const int currentFrameIndex = (int)currentFrame;
			const Atoms::layerFrame& frame = m_data->getLayer(currentFrameIndex);
			rawcolor c = {(unsigned char)bgColor.blue(), (unsigned char)bgColor.green(), (unsigned char)bgColor.red(), 0};
			if(frame.maxLayer >= 0 && !frame.layers.empty()){
				c = colors.getInterpolatedRawColor(m_data->getGroupLayer(id, currentFrameIndex));
			}

			//fill color
			while(x < requestedSize.width() && currentFrameIndex == (int)(currentFrame)){
				for(int y = 0; y < requestedSize.height(); y++){
					(((int*)heatmap.bits())[x+(y)*heatmap.width()]) = c.data;
				}
				currentFrame += step;
				x++;
			}
		}

		return heatmap;
	}else{
		if(id >= m_data->numberOfAtroms()) return getBGImage(bgColor);
		//ignore water
		{
			const QString& groupName = m_data->getAtom(id).residue;
			if(groupName == "HOH" || groupName.toLower() == "water") return getBGImage(bgColor);
		}

		//qDebug()<<" Atom: "<<a;
		QImage heatmap(requestedSize, QImage::Format_RGB32); //(0xffRRGGBB)
		if(size) *size = QSize(requestedSize);
		const int timeSteps =  m_timeline->getEndFrame() - m_timeline->getStartFrame();
		const float step = timeSteps/(float)requestedSize.width();
		float currentFrame = m_timeline->getStartFrame();
		for(int x = 0; x < requestedSize.width();){
			const int currentFrameIndex = (int)currentFrame;
			const Atoms::layerFrame& frame = m_data->getLayer(currentFrameIndex);
			rawcolor c = {(unsigned char)bgColor.blue(), (unsigned char)bgColor.green(), (unsigned char)bgColor.red(), 0};
			if(frame.maxLayer >= 0 && !frame.layers.empty()){
				c = colors.getInterpolatedRawColor(m_data->getAtomLayer(id, currentFrameIndex));//getResidueLayersColor(frame.layer[a]/(float)frame.maxLayer);
			}

			//fill color
			while(x < requestedSize.width() && currentFrameIndex == (int)(currentFrame)){
				for(int y = 0; y < requestedSize.height(); y++){
					(((int*)heatmap.bits())[x+(y)*heatmap.width()]) = c.data;
				}
				currentFrame += step;
				x++;
			}
		}
		return heatmap;
	}

}

SurfaceLayersImageProvider::~SurfaceLayersImageProvider() {}

