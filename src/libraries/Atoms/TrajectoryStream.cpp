/*
 * TrajectoryStream.cpp
 *
 *  Created on: 29.06.2017
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
#include <TrajectoryStream.h>

TrajectoryStream::TrajectoryStream(QObject* parent): QObject(parent) {

}


TrajectoryStream::TrajectoryStream(QObject* parent, int numberOfAtoms, const QVector<unsigned int>* frameOffsets, const QString& trajectoryFile, int window):
		QObject(parent) {
	open(trajectoryFile, numberOfAtoms, frameOffsets, window);
}


bool TrajectoryStream::open(const QString& trajectoryFile, int numberOfAtoms, const QVector<unsigned int>* frameOffsets, int window){
	if(trajectoryFile.isEmpty() || !frameOffsets) return false;

	if(m_xtcfile) xdrfile_close(m_xtcfile);
	m_xtcfile = xdrfile_open(trajectoryFile.toLatin1(), "r");
	if(!m_xtcfile){
		qDebug()<<"Failed to open xtc file!";
		clear();
		return false;
	}
	m_trajectoryFile = trajectoryFile;
	m_numberOfAtoms = numberOfAtoms;
	m_frameOffsets = frameOffsets;
	m_windowRadius = -9999;

	setWindowRadius(window);
	return true;
}

TrajectoryStream::xtcFrame& TrajectoryStream::getFrame(int i){
	//qDebug()<<__LINE__<<"getFrame"<<i<<"size:"<<m_window.size()<<"m_windowIndex:"<<m_windowIndex<<"m_currentIndex:"<<m_currentIndex<<"filename:"<<m_trajectoryFile<<"/n";
	if(m_currentIndex != i){
		m_currentIndex = i;
		if(m_windowRadius == 0){
			m_windowIndex = 0;
			readFrame(m_frameOffsets->at(i),m_window.front());
		}else{
			m_windowIndex = m_windowRadius;
			int start = i-m_windowRadius;
			int end = i+m_windowRadius;
			if(start < 0) {
				m_windowIndex += start;
				start = 0;
			}
			if(end >= m_frameOffsets->size()) end = m_frameOffsets->size()-1;
			m_window.clear();
			readFrames(m_frameOffsets->at(start), end-start+1);
			//qDebug()<<__LINE__<<"i:"<<i<<":"<<start<<"-"<<end<<(end-start)<<" wI:"<<m_windowIndex<<" wS:"<<m_windowRadius<<m_window.size()<<"s:"<<m_frameOffsets->size();
		}
	}
	return m_window[m_windowIndex];
}

TrajectoryStream::xtcFrame& TrajectoryStream::getCurrentFrame(){
	return m_window[m_windowIndex];
}
const TrajectoryStream::xtcFrame& TrajectoryStream::getCurrentFrame() const{
	return m_window[m_windowIndex];
}

void TrajectoryStream::setWindowRadius(int size){
	if(size < 0) size = 0;
	if(m_windowRadius != size){
		m_windowRadius = size;
		m_window.clear();
		if(m_frameOffsets == nullptr) return;
		if(m_windowRadius == 0){
			m_windowIndex = 0;
			m_window.push_back(TrajectoryStream::xtcFrame());
			readFrame(m_frameOffsets->at(m_currentIndex),m_window.front());
		}else{
			int start = m_currentIndex-m_windowRadius;
			int end = m_currentIndex+m_windowRadius;
			if(start < 0) {
				m_windowIndex += start;
				start = 0;
			}
			if(end >= m_frameOffsets->size()) end = m_frameOffsets->size()-1;
			readFrames(m_frameOffsets->at(start), end-start+1);
		}
	}
}

void TrajectoryStream::getSmoothedPositions(QVector<glm::vec3>& smoothedPositions) const{
	smoothedPositions.fill(glm::vec3(0,0,0), m_numberOfAtoms);

	//go over frames in window and build average
	float count = 0;
	for(const TrajectoryStream::xtcFrame& frame : m_window){
		auto itSF = smoothedPositions.begin();
		for(const glm::vec3& p: frame.positions){
			*itSF = ((*itSF)*count + p)/(count+1);
			itSF++;
		}
		count += 1.f;
	}
}

inline void readXTCFrame(XDRFILE* xtcfile, TrajectoryStream::xtcFrame& frame, int numberOfAtoms){
	frame.positions.resize(numberOfAtoms); //make room for the position data
	matrix axis; // unused value
	if(read_xtc(xtcfile, numberOfAtoms, &frame.index, &frame.time, axis, (rvec*)frame.positions.data(), &frame.precision) != exdrOK){
		qDebug()<<__LINE__<<" Error Reading frame!";
		frame.index = -1;
		return;
	}

	//set up bounding box
	frame.box.min = glm::vec3(std::numeric_limits<float>::max(),std::numeric_limits<float>::max(),std::numeric_limits<float>::max());
	frame.box.max = glm::vec3(std::numeric_limits<float>::min(),std::numeric_limits<float>::min(),std::numeric_limits<float>::min());
	for(glm::vec3& position: frame.positions){
		position *= 10.f;//scale them TODO
		if(position.x < frame.box.min.x) frame.box.min.x = position.x;
		if(position.y < frame.box.min.y) frame.box.min.y = position.y;
		if(position.z < frame.box.min.z) frame.box.min.z = position.z;

		if(position.x > frame.box.max.x) frame.box.max.x = position.x;
		if(position.y > frame.box.max.y) frame.box.max.y = position.y;
		if(position.z > frame.box.max.z) frame.box.max.z = position.z;
	}
}

void TrajectoryStream::readFrames(unsigned int offset, unsigned int size){
	if(m_xtcfile){
		xdrfile_seek(m_xtcfile, offset);
		for(unsigned int i = 0; i < size; i++){
			m_window.push_back(TrajectoryStream::xtcFrame());
			readXTCFrame(m_xtcfile, m_window.last(), m_numberOfAtoms);
		}
	}else{
		qDebug()<<"[FATAL]: You are reading xtc file before it has bean init! App shutdown!";
		exit(0);
	}
}

void TrajectoryStream::readFrame(unsigned int offset, xtcFrame& frame){
	if(m_xtcfile){
		xdrfile_seek(m_xtcfile, offset);
		readXTCFrame(m_xtcfile, frame, m_numberOfAtoms);
	}else{
		qDebug()<<"[FATAL]: You are reading xtc file before it has bean init! App shutdown!";
		exit(0);
	}
}

void TrajectoryStream::clear(){
	if(m_xtcfile) xdrfile_close(m_xtcfile);
	m_xtcfile = nullptr;
	m_windowRadius = 0;
	m_windowIndex = 0;
	m_window.clear();
	m_frameOffsets = nullptr;
	m_currentIndex = 0;
	m_numberOfAtoms = 0;
}

TrajectoryStream::~TrajectoryStream() {
	if(m_xtcfile) xdrfile_close(m_xtcfile);
}

