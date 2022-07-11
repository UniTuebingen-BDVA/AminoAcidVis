/*
 * Timeline.cpp
 *
 *  Created on: 25.04.2017
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

#include <Timeline.h>

static QColor colorWheel[] = { "#f03e70", "#2ec32e", "#3fbeff", "#c32eb0", "#2e9ec3", "#e3bf42", "#97db82" };
static unsigned int colorWheelID = 0;

Tracker::Tracker(Timeline* timeline, int frame): QObject(timeline), m_timeline(timeline), m_frame(0), m_color(colorWheel[colorWheelID]){
	set(frame);
	connect(&m_timer, SIGNAL(timeout()), this, SLOT(next()));

	colorWheelID++;
	if(colorWheelID >= sizeof(colorWheel)/sizeof(QColor))
		colorWheelID = 0;
}

Tracker::~Tracker(){}


//Tracker::Tracker(const Tracker& frame): m_timeline(frame.m_timeline), m_frame(frame.m_frame){}

//void Tracker::operator=(const Tracker& frame){

//}

void Tracker::operator++(int){
	next();
}
void Tracker::operator--(int){
	previous();
}

void Tracker::operator+=(int i){
	next(i);
}
void Tracker::operator-=(int i){
	previous(i);
}

void Tracker::operator*=(int i){
	set(m_frame*i);
}

void Tracker::next(unsigned int offset){
	if(m_frame < m_timeline->getEndFrame()){
		m_frame+=offset;
		if(m_frame > m_timeline->getEndFrame())
			m_frame = m_timeline->getEndFrame();
		emit frameChanged();
		emit frameValueChanged(m_frame);
		emit valueChanged(getValue());
	}else if(isActive()) stop();
}
void Tracker::previous(unsigned int offset){
	if(m_frame > m_timeline->getStartFrame()){
		m_frame-=offset;
		if(m_frame < m_timeline->getStartFrame())
			m_frame = m_timeline->getStartFrame();
		emit frameChanged();
		emit frameValueChanged(m_frame);
		emit valueChanged(getValue());
	}
}

void Tracker::set(int frame){
	if(frame < 0) frame = 0;
	if(frame > m_timeline->getEndFrame()) frame = m_timeline->getEndFrame();

	//qDebug()<<"[set Frame]: "<<m_frame<<" to "<<frame;
	if(frame != m_frame){
		m_frame = frame;
		emit frameChanged();
		emit frameValueChanged(m_frame);
		emit valueChanged(getValue());
	}
}

void Tracker::setValue(int time){
	if(m_timeline->isNanosecondsUsed()){
		const double step = m_timeline->getMaxFrame()/(double)m_timeline->getNanosecondsLength() ;
		//qDebug()<<"[setValue]: Step: "<<step<<": "<<(step*time)<<" time "<<time;
		set(step*time);
	}else
		set(time);
}

int Tracker::get() const{
	return m_frame;
}

int Tracker::getValue() const{
	if(m_timeline->isNanosecondsUsed()){
		const double step = m_timeline->getNanosecondsLength()/(double)m_timeline->getMaxFrame() ;
		//qDebug()<<"[getValue]: Step: "<<step<<": "<<(step*get())<<" time "<<get();
		return step*get();
	}else{
		return get();
	}
}

int Tracker::getMaxFrame() const {return m_timeline->getMaxFrame();}
int Tracker::getStartFrame() const {return m_timeline->getStartFrame();}
int Tracker::getEndFrame() const {return m_timeline->getEndFrame();}


void Tracker::play(){
	m_timeline->stopAll();
	m_timer.start(m_timeline->getInterval());
	emit onPlay();
}

void Tracker::stop(){
	m_timer.stop();
	emit onStop();
}

void Tracker::togglePlay(){
	if(m_frame >= m_timeline->getMaxFrame()){
		stop();
	}else{
		if(isActive()) stop(); else play();
	}
}

bool Tracker::isActive() const{
	return m_timer.isActive();
}


void Tracker::setInterval(int interval){
	m_timer.setInterval(interval);
}

Timeline::Timeline(QObject* parent ):QObject(parent){
	//connect(&m_timer, SIGNAL(timeout()), this, SLOT(next()));

	//m_frames.push_back(new Tracker(this));
}


void Timeline::setInterval(int interval){
	if(m_interval == interval) return;
	m_interval = interval;
	for(Tracker* f: m_frames)
		f->setInterval(m_interval);
}


int Timeline::getInterval() const{
	return m_interval;
}

Tracker* Timeline::get(int i){
	return m_frames[i];
}
const Tracker* Timeline::get(int i) const{
	return m_frames[i];
}

int Timeline::getFrame(int i) const{
	return *get(i);
}

int Timeline::getSize() const{
	return m_frames.size();
}

void Timeline::reset(){
	for(Tracker* f: m_frames)
		f->set(0);
	m_startFrame = 0;
	m_endFrame = m_maxFrame;
	emit onStartFrameChanged();
	emit onStartFrameValueChanged(m_startFrame);
	emit onEndFrameChanged();
	emit onEndFrameValueChanged(m_endFrame);

	emit onStartChanged();
	emit onStartValueChanged(getStart());
	emit onEndChanged();
	emit onEndValueChanged(getEnd());
}

int Timeline::getMaxFrame() const{
	return m_maxFrame;
}

void Timeline::setActiveTracker(int index) {
	if(index >= 0 && index < getSize() && m_activeTracker != index){
		m_activeTracker = index;
		emit activeTrackerChanged();
		emit activeTrackerValueChanged(m_activeTracker);
	}
}


void Timeline::setMaxFrame(int frame){
	if(frame < 0) frame = 0;
	if(m_maxFrame != frame){
		if(frame) frame--;
		m_maxFrame = frame;
		emit onMaxFrameChanged();
		emit onMaxFrameValueChanged(m_maxFrame);

		emit onMaxChanged();
		emit onMaxValueChanged(getMax());

		if(m_endFrame > m_maxFrame){
			m_endFrame = m_maxFrame;
			emit onEndFrameChanged();
			emit onEndFrameValueChanged(m_endFrame);

			emit onEndChanged();
			emit onEndValueChanged(getEnd());
		}
	}
}

int Timeline::getStartFrame() const{
	return m_startFrame;
}
void Timeline::setStartFrame(int frame){
	if(frame < 0) frame = 0;
	if(m_startFrame != frame){
		m_startFrame = frame;
		emit onStartFrameChanged();
		emit onStartFrameValueChanged(m_startFrame);

		emit onStartChanged();
		emit onStartValueChanged(getStart());
	}
}

int Timeline::getEndFrame() const{
	return m_endFrame;
}
void Timeline::setEndFrame(int frame){
	if(frame < 0) frame = 0;
	if(frame >= m_maxFrame) frame = m_maxFrame;
	if(m_endFrame != frame){
		m_endFrame = frame;
		emit onEndFrameChanged();
		emit onEndFrameValueChanged(m_endFrame);

		emit onEndChanged();
		emit onEndValueChanged(getEnd());
	}
}

void Timeline::setStartNs(int ns){
	const double step = m_maxFrame/(double)m_nsLenght;
	setStartFrame(step*ns);
}
void Timeline::setEndNs(int ns){
	const double step = m_maxFrame/(double)m_nsLenght;
	setEndFrame(step*ns);
}

void Timeline::setStart(int time){
	if(m_useNanoseconds) setStartNs(time); else setStartFrame(time);
}
void Timeline::setEnd(int time){
	if(m_useNanoseconds) setEndNs(time); else setEndFrame(time);
}

int Timeline::getMaxNs() const{
	return m_nsLenght;
}
int Timeline::getStartNs() const{
	const double step = m_nsLenght/(double)m_maxFrame;
	return step*m_startFrame;
}
int Timeline::getEndNs() const{
	const double step = m_nsLenght/(double)m_maxFrame;
	return step*m_endFrame;
}

int Timeline::getMax() const{
	return (m_useNanoseconds)? getMaxNs(): getMaxFrame();
}
int Timeline::getStart() const{
	return (m_useNanoseconds)? getStartNs(): getStartFrame();
}
int Timeline::getEnd() const{
	return (m_useNanoseconds)? getEndNs(): getEndFrame();
}

void Timeline::setUseNanoseconds(bool enabled){
	if(enabled != m_useNanoseconds){
		m_useNanoseconds = enabled;
		emit nanosecondsUseChanged();
		emit nanosecondsUseVlaueChanged(m_useNanoseconds);
		emit onStartFrameChanged();
		emit onStartFrameValueChanged(m_startFrame);
		emit onEndFrameChanged();
		emit onEndFrameValueChanged(m_endFrame);

		emit onMaxChanged();
		emit onMaxValueChanged(getMax());
		emit onStartChanged();
		emit onStartValueChanged(getStart());
		emit onEndChanged();
		emit onEndValueChanged(getEnd());

		//update all
		for(Tracker* t: m_frames){
			t->frameChanged();
			t->frameValueChanged(t->get());
			t->valueChanged(t->getValue());
		}
	}
}

void Timeline::setNanosecondsLenght(int lenght){
	if(lenght < 10) lenght = 10;
	if(m_nsLenght != lenght){
		m_nsLenght = lenght;
		emit nanosecondsValueChanged(m_nsLenght);
		if(m_useNanoseconds){
			emit onStartFrameChanged();
			emit onStartFrameValueChanged(m_startFrame);
			emit onEndFrameChanged();
			emit onEndFrameValueChanged(m_endFrame);

			emit onMaxChanged();
			emit onMaxValueChanged(getMax());
			emit onStartChanged();
			emit onStartValueChanged(getStart());
			emit onEndChanged();
			emit onEndValueChanged(getEnd());
		}
	}
}

Tracker* Timeline::addTracker(int frame){
	m_frames.push_back(new Tracker(this, frame));
	connect(m_frames.back(), SIGNAL(frameChanged()), this, SIGNAL(anyTrackerChanged()));
	emit trackersSizeChanged();
	return m_frames.back();
}

void Timeline::removeTracker(Tracker* frame){
	removeTracker(m_frames.indexOf(frame));
}

void Timeline::removeTracker(int index){
	if(index < 0) return;

	m_frames.removeAt(index);
	emit trackersSizeChanged();
	if(index <= m_activeTracker){
		if(m_activeTracker > 0) m_activeTracker--;
		emit activeTrackerChanged();
		emit activeTrackerValueChanged(m_activeTracker);
	}
}

void Timeline::stopAll(){
	for(Tracker* f: m_frames)
		f->stop();
}

Timeline::~Timeline(){
	for(Tracker* f: m_frames)
		delete f;
}
