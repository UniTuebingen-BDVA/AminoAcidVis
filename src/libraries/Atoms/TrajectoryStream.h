/*
 * TrajectoryStream.h
 *
 *  Created on: 29.06.2017
 *      Author: Vladimir Ageev
 *
 * @brief  		Contains xtc streaming class.
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

#ifndef EXECUTABLES_AMINOVIS_TRAJECTORYSTREAM_H_
#define EXECUTABLES_AMINOVIS_TRAJECTORYSTREAM_H_

#include <QObject>
#include <QString>
#include <QVector>
#include <QList>
#include <glm/glm.hpp>
#include <QDebug>
#include <Util/AABB.h>

#include <xdrfile_xtc.h>
#include <limits>


/*!
 * @brief Opens and reads xtc trajectory frames as a stream. Note the header of the xtc and the offsets of each frame need to be read out beforehand.
 * Also it is possible to define a smoothing window to smooth out the movement of the molecules.
 */
class TrajectoryStream: public QObject {
	Q_OBJECT
public:
	/*!
	 * @brief This struct contains the positions of one frame of the trajectories.
	 */
	struct xtcFrame{
		int index;
		float time; /// The time of the frame.
		float precision; /// The compression precision of the xtc file, for the given frame. (is usually constant).
		aabb box; /// The bounding box (aabb) of the frame.
		QVector<glm::vec3> positions; /// The positions of each atom. The indices match with the model.
	};

	TrajectoryStream(QObject* parent);
	/*!
	 * @brief This constructor will directly open a xtc file.
	 * @param parent The QT parent object
	 * @param trajectoryFile The file path to the xtc file.
	 * @param numberOfAtoms The number of expected atoms inside the xtc file. Can be extracted form the header and should be the same as in the model.
	 * @param frameOffsets The offsets of each frame. They need to be extracted beforehand.
	 * @param window Init value of the smoothing window radius.
	 */
	TrajectoryStream(QObject* parent, int numberOfAtoms, const QVector<unsigned int>* frameOffsets, const QString& trajectoryFile, int window = 0);
	virtual ~TrajectoryStream();

	/*! @returns The number of atoms inside the xtc file. This should be the same as in the model file!  */
	inline int numberOfAtoms() const{return m_numberOfAtoms;}
	/*! @returns The number of frames inside the xtc file.  */
	inline int numberOfFrames() const{ return m_frameOffsets->size(); }
	/*! @returns The radius of the trajectory smooth window.  */
	inline int getWindowRadius() const{ return m_windowRadius;}
	/*! @returns The current opened xtc file name.  */
	inline const QString& getFileName() const { return m_trajectoryFile;}
	/*! @returns The number of frames inside the smoothing window.  */
	inline int size() const{ return m_window.size(); }

	/*!
	 * @brief Will open a data stream of a given xtc file. Note this will not read the header of the xtc, but just the data.
	 * @param trajectoryFile The file path to the xtc file.
	 * @param numberOfAtoms The number of expected atoms inside the xtc file. Can be extracted form the header and should be the same as in the model.
	 * @param frameOffsets The offsets of each frame. They need to be extracted beforehand.
	 * @param window Init value of the smoothing window radius.
	 */
	bool open(const QString& trajectoryFile, int numberOfAtoms, const QVector<unsigned int>* frameOffsets, int window = 0);

	/*!
	 * @brief Will return the frame with the index i.
	 * This will move the window if needed.
	 * @return An xtcFrame.
	 */
	xtcFrame& getFrame(int i);

	/*! @returns The current frame set via getFrame().*/
	xtcFrame& getCurrentFrame();
	/*! @returns The current frame set via getFrame().*/
	const xtcFrame& getCurrentFrame() const;

	/*!
	 * @brief Sets the given vector with the mean values of all positions inside the smoothing window, if radius if bigger then zero.
	 * @param smoothedPositions The target output vector with the smoothed positions.
	 */
	void getSmoothedPositions(QVector<glm::vec3>& smoothedPositions) const;

public slots:
	/*!
	 * @brief Sets the smoothing radius of the window.
	 * Inside the window the mean value of all positions is calculated to smooth out the movement of the molecules.
	 * @see getWindowRadius
	 * @see getSmoothedPositions
	 */
	void setWindowRadius(int radius);

	/*!
	 * @brief Closes the xtc data stream and deletes all the data.
	 */
	void clear();
private:
	void readFrames(unsigned int offset, unsigned int size);
	void readFrame(unsigned int offset, xtcFrame& frame);

	QString m_trajectoryFile;
	int m_numberOfAtoms = 0;
	int m_currentIndex = 0;
	XDRFILE* m_xtcfile = nullptr; /// xtc file handle
	const QVector<unsigned int>* m_frameOffsets = nullptr; /// The frame offsets form the xtc file

	int m_windowRadius = 0; /// The radius of the window
	int m_windowIndex = 0;/// The center index of the window
	QList<xtcFrame> m_window; /// The window containing the xtc frames with the molecules positions
};

#endif /* EXECUTABLES_AMINOVIS_TRAJECTORYSTREAM_H_ */
