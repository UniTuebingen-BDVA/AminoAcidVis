/*
 * FrameContolWidget.cpp
 *
 *  Created on: 04.08.2017
 *      Author: Vladimir Ageev (vladimir.agueev@progsys.de
 */

#include <Dialogs/FrameContolWidget.h>
#include <Util/ResourcePath.h>

FrameContolWidget::FrameContolWidget(Tracker* frame, QWidget *parent): QFrame(parent) {
	setupUi(this);

	toolButton_HideShow->setIcon(QIcon(resourcePath() + "/textures/eye_open.png"));

	toolButton_PlayPause->setIcon(QIcon(resourcePath() + "/textures/play.png"));
	toolButton_nextFrame->setIcon(QIcon(resourcePath() + "/textures/next_frame.png"));
	toolButton_previousFrame->setIcon(QIcon(resourcePath() + "/textures/previous_frame.png"));

	if(!frame) return;


	connect(toolButton_HideShow, &QToolButton::clicked, this, [this]{
		m_dockVisibility = !m_dockVisibility;
		emit toggleDockVisibility(m_dockVisibility);
		if(m_dockVisibility)
			toolButton_HideShow->setIcon(QIcon(resourcePath() + "/textures/eye_open.png"));
		else
			toolButton_HideShow->setIcon(QIcon(resourcePath() + "/textures/eye_closed.png"));
	});

	connect(toolButton_nextFrame, SIGNAL(clicked()), frame, SLOT(next()));
	connect(toolButton_previousFrame, SIGNAL(clicked()), frame, SLOT(previous()));



	connect(toolButton_PlayPause, SIGNAL(clicked()), frame, SLOT(togglePlay()));

	connect(frame, &Tracker::onPlay, this, [this]{
			toolButton_PlayPause->setIcon(QIcon(resourcePath() + "/textures/pause.png"));
	});

	connect(frame, &Tracker::onStop, this, [this]{
			toolButton_PlayPause->setIcon(QIcon(resourcePath() + "/textures/play.png"));
	});

	spinBox->setMaximum(frame->getMaxFrame());
	connect(frame, SIGNAL(valueChanged(int)), spinBox, SLOT(setValue(int)));
	connect(spinBox, SIGNAL(valueChanged(int)), frame, SLOT(setValue(int)));

	connect(frame->getTimeline(), &Timeline::nanosecondsUseVlaueChanged, this, [this](bool use){
		if(use){
			spinBox->setSuffix("ns");
		}else{
			spinBox->setSuffix("");
		}
	});

	connect(frame->getTimeline(), &Timeline::onMaxFrameValueChanged, this, [this](int f){
		spinBox->setMaximum(f);
	});

	//set color
	QPixmap pixmap(5, 24);
	pixmap.fill(frame->getColor());
	colorLabel->setPixmap(pixmap);
	//colorLabel->setText("1:");
}

FrameContolWidget::~FrameContolWidget() {

}

void FrameContolWidget::dockVisibilityChanged(bool visibility){
	if(m_dockVisibility == visibility) return;
	m_dockVisibility = visibility;
	if(visibility)
		toolButton_HideShow->setIcon(QIcon(resourcePath() + "/textures/eye_open.png"));
	else
		toolButton_HideShow->setIcon(QIcon(resourcePath() + "/textures/eye_closed.png"));
}

