/**
 * @file   		SurfaceLayersImageProvider.h
 * @author 		Vladimir Ageev (vladimir.agueev@progsys.de)
 * @date   		08.05.2017
 *
 * @brief  		Provides the atom surface layer heatmap to QML.
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


#ifndef EXECUTABLES_AMINOVIS_SURFACELAYERSIMAGEPROVIDER_H_
#define EXECUTABLES_AMINOVIS_SURFACELAYERSIMAGEPROVIDER_H_

#include <QImage>
#include <QQuickImageProvider>
#include <Atoms/Atoms.h>
#include <ColorLibrary.h>
#include <Atoms/Timeline.h>
#include <Atoms/FilterAtoms.h>
class SurfaceLayersImageProvider: public QQuickImageProvider {
	//Q_OBJECT
public:
	SurfaceLayersImageProvider(Atoms* data, Timeline* timeline,
			ColorLibrary* colors);

	QImage requestImage(const QString &id, QSize *size, const QSize &requestedSize);

	QImage requestImage(bool residue, int id, const ColorsRow& colors, QSize *size, const QSize &requestedSize);

	virtual ~SurfaceLayersImageProvider();
private:
	Atoms* m_data = nullptr;
	Timeline* m_timeline = nullptr;
	ColorLibrary* m_colors = nullptr;
};

#endif /* EXECUTABLES_AMINOVIS_SURFACELAYERSIMAGEPROVIDER_H_ */
