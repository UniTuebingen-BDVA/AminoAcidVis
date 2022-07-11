/**
 * @file   		Timeline.h
 * @author 		Vladimir Ageev (vladimir.agueev@progsys.de)
 * @date   		25.04.2017
 *
 * @brief  		Contains the logic of the timeline.
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
#ifndef EXECUTABLES_AMINOVIS_TIMELINE_H_
#define EXECUTABLES_AMINOVIS_TIMELINE_H_

#include <QObject>
#include <QDebug>
#include <QTimer>
#include <QVector>
#include <QVariant>
#include <QColor>

//forward decleration
class Timeline;

/*!
 * @brief Manages a timeline frame and it's position.
 */
class Tracker: public QObject {
	Q_OBJECT
	Q_PROPERTY(unsigned int frame READ get WRITE set NOTIFY frameChanged)
	Q_PROPERTY(unsigned int value READ getValue WRITE setValue NOTIFY frameChanged)
	Q_PROPERTY(QColor color READ getColor NOTIFY colorChanged)
	Q_PROPERTY(QColor colorGray READ getColorGray NOTIFY colorChanged)
public:
	/*!
	 * @brief Constructor of the tracker
	 * @param timeline The parent timeline
	 * @param frame Init frame
	 */
	Tracker(Timeline* timeline = nullptr, int frame = 0);
	virtual ~Tracker();

	/*! @returns The parent timeline. */
	inline Timeline* getTimeline() {return m_timeline;}
	/*! @returns The parent timeline. */
	inline const Timeline* getTimeline() const {return m_timeline;}
	//Frame(const Frame& frame);
	//void operator=(const Frame& frame);

	inline const QColor& getColor() const {return m_color;}
	inline const QColor getColorGray() const {return QColor(m_color.red()*0.7, m_color.green()*0.7, m_color.blue()*0.7);}

	/*! @see get */
	inline operator int() const { return m_frame; }
	inline void operator=(int i) { set(i); }
	inline bool operator==(int i) { return m_frame == i; }
	inline bool operator>(int i) { return m_frame > i; }
	inline bool operator>=(int i) { return m_frame >= i; }
	inline bool operator<(int i) { return m_frame < i; }
	inline bool operator<=(int i) { return m_frame <= i; }

	/*! @see next */
	void operator++(int);
	/*! @see previous */
	void operator--(int);

	/*! @see next */
	void operator+=(int i);
	/*! @see previous */
	void operator-=(int i);

	/*!
	 * @brief Multiples the current frame with the given value.
	 * @see next
	 * */
	void operator*=(int i);

public slots:
	/*! @brief Starts moving the frame forward with the set interval until stopped or end frame is reached. */
	void play();
	/*! @brief Stops the frame from moving. */
	void stop();
	/*!
	 *  @brief Toggles movement of the frame. If it's active it stops, if it is inactive it plays.
	 *  @see play
	 *  @see stop
	 */
	void togglePlay();
	/*!
	 * @returns true, if the frame is played.
	 */
	bool isActive() const;
	/*!
	 * @brief Sets the interval. The smaller it is the faster it will move.
	 * @param interval The interval value in milliseconds.
	 */
	void setInterval(int interval);

	/*! @brief Moves the frame forward with the given value. */
	void next(unsigned int offset = 1);
	/*! @brief Moves the frame back with the given value. */
	void previous(unsigned int offset = 1);

	/*! @brief Set the frame position. The frame will be clipped to the range of the timeline */
	void set(int frame);
	/*! @brief Set the frame position depending if ns is used. The frame will be clipped to the range of the timeline */
	void setValue(int time);

	/*! @brief Get the current frame.*/
	int get() const;
	/*! @brief Get the current frame or ns.*/
	int getValue() const;

	int getMaxFrame() const;
	int getStartFrame() const;
	int getEndFrame() const;

signals:
	/// Send out when the frame value has changed.
	void frameChanged();
	/// Send out when the frame value has changed, containing the new value if the frame itself.
	void frameValueChanged(int);
	void valueChanged(int);

	/// Send out when play is triggered.
	void onPlay();
	/// Send out when stop is triggered.
	void onStop();

	void colorChanged();
private:
	Timeline* m_timeline = nullptr; /// Parent timeline
	int m_frame;
	QColor m_color;

	QTimer m_timer;
};

//Q_DECLARE_METATYPE(Frame);

/*!
 * @brief The timeline is defined via a minimum and maximum frame that is given from a trajectory file and
 * a window that shows a subset of the timeline which can be defined by the user.
 * Additionally it manages all the takers on the timeline.
 * @see Tracker
 */
class Timeline: public QObject {
	Q_OBJECT
	Q_PROPERTY(int size READ getSize NOTIFY trackersSizeChanged)
	Q_PROPERTY(int activeTracker READ getActiveTracker WRITE setActiveTracker NOTIFY activeTrackerChanged)
	Q_PROPERTY(unsigned int max READ getMaxFrame NOTIFY onMaxFrameChanged)
	Q_PROPERTY(unsigned int start READ getStartFrame WRITE setStartFrame NOTIFY onStartFrameChanged)
	Q_PROPERTY(unsigned int end READ getEndFrame WRITE setEndFrame NOTIFY onEndFrameChanged)

	Q_PROPERTY(int startVis READ getStart WRITE setStartFrame NOTIFY onStartFrameChanged)
	Q_PROPERTY(int endVis READ getEnd WRITE setEndFrame NOTIFY onEndFrameChanged)

	Q_PROPERTY(bool isNsUsed READ isNanosecondsUsed WRITE setUseNanoseconds NOTIFY nanosecondsUseChanged)

public:
	Timeline(QObject* parent = nullptr);
	virtual ~Timeline();

	Tracker* get(int i);
	const Tracker* get(int i) const;

	/*!
	 * @returns A Tracker as an QVariant. Used by QML.
	 */
	Q_INVOKABLE QVariant getR(int i) {return QVariant::fromValue(get(i));}

	inline int getActiveTracker() const { return m_activeTracker; }

	int getInterval() const;

	int getFrame(int i) const;

	/*! @returns The number of frames managed by the timeline.*/
	int getSize() const;

	int getMaxFrame() const;
	int getStartFrame() const;
	int getEndFrame() const;

	int getMaxNs() const;
	int getStartNs() const;
	int getEndNs() const;

	int getMax() const;
	int getStart() const;
	int getEnd() const;

	inline bool isNanosecondsUsed() const{ return m_useNanoseconds; }
	inline int getNanosecondsLength() const{ return m_nsLenght; }

	Tracker* addTracker(int frame = 0);

public slots:
	/*!
	 * @brief Sets the interval of all trackers.
	 * @see Tracker::setInterval
	 */
	void setInterval(int interval);
	/*!
	 * @brief Sets all frames to 0 and resets the start/end range to the min/max frame.
	 */
	void reset();
	void setActiveTracker(int index);

	void setMaxFrame(int frame);
	void setStartFrame(int frame);
	void setEndFrame(int frame);

	void setStartNs(int ns);
	void setEndNs(int ns);

	void setStart(int time);
	void setEnd(int time);

	void setUseNanoseconds(bool enabled);
	void setNanosecondsLenght(int lenght);

	void removeTracker(Tracker* frame);
	void removeTracker(int index);

	void stopAll();
signals:
	void onMaxFrameChanged();
	void onMaxFrameValueChanged(int);
	void onStartFrameChanged();
	void onStartFrameValueChanged(int);
	void onEndFrameChanged();
	void onEndFrameValueChanged(int);

	void onMaxChanged();
	void onMaxValueChanged(int);
	void onStartChanged();
	void onStartValueChanged(int);
	void onEndChanged();
	void onEndValueChanged(int);

	void nanosecondsUseChanged();
	void nanosecondsUseVlaueChanged(bool);
	void nanosecondsValueChanged(int);

	void anyTrackerChanged();

	void trackersSizeChanged();
	void activeTrackerChanged();
	void activeTrackerValueChanged(int);
private:
	QVector<Tracker*> m_frames;
	int m_maxFrame = 0;

	int m_startFrame = 0;
	int m_endFrame = 0;

	int m_interval = 1000.f;

	int m_activeTracker = 0;

	bool m_useNanoseconds = false;
	int m_nsLenght = 10000;
};



#endif /* EXECUTABLES_AMINOVIS_TIMELINE_H_ */
