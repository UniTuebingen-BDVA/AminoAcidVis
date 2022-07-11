/*
 * RenderDockWidget.cpp
 *
 *  Created on: 03.08.2017
 *      Author: Vladimir Ageev (vladimir.agueev@progsys.de
 */

#include <Dialogs/RenderDockWidget.h>
#include <Atoms/FilterAtoms.h>
#include <QSpinBox>
#include <QApplication>
#include <QClipboard>
#include <Util/ResourcePath.h>
#include <QFileDialog>

#ifndef M_PI
    #define M_PI   3.14159265358979323846
#endif
#ifndef M_PI_2
	#define M_PI_2 1.57079632679489661923
#endif
#ifndef M_PIt2
	#define M_PIt2 6.28318530717958647692
#endif
//RenderDockWidget::RenderDockWidget(QSettings& settings, QWidget *parent): QDockWidget(parent) {}

RenderDockWidget::RenderDockWidget(QSettings& settings, Atoms* data, Tracker* frame, ColorLibrary* colorLib, FilterAtomsListModel* filter, QWidget *parent): QDockWidget(parent) {
	//setup window
	QMainWindow *window = new QMainWindow(nullptr);
	setupUi(window);

	setWindowTitle("Render View");
	setObjectName("Render View");

	openGLWidget->setData(data, frame, colorLib, filter );

	actionCenterCamera->setIcon(QIcon(resourcePath() + "/textures/center.png"));
	actionFollowProtain->setIcon(QIcon(resourcePath() + "/textures/follow.png"));
	actionApplyFilters->setIcon(QIcon(resourcePath() + "/textures/filter2.png"));

	connect(parent, SIGNAL(onStyleChanged()), openGLWidget, SLOT(onStyleChanged()));
	connect(actionCenterCamera, SIGNAL(triggered()), openGLWidget, SLOT(doCenterCamera()));
	connect(actionFollowProtain, SIGNAL(toggled(bool)), openGLWidget, SLOT(setKeepCentered(bool)));
	connect(actionApplyFilters, SIGNAL(toggled(bool)), openGLWidget, SLOT(setFiltersEnabled(bool)));
	openGLWidget->setKeepCentered(actionFollowProtain->isChecked());
	openGLWidget->setAtomBaseColor(settings.value("BaseAtomColor", "#a1c4f2").toString());


	m_styleComboBox = new QComboBox(this);
	m_styleComboBox->setStatusTip("Select the visualization style");
	m_styleComboBox->setToolTip("Visualization style");
	mainToolBar->addWidget(m_styleComboBox);
	m_styleComboBox->addItem(QIcon(resourcePath() + "/textures/space-fill.png"), "Space-fill");
	m_styleComboBox->addItem(QIcon(resourcePath() + "/textures/ball-stick.png"), "Ball & Stick");
	m_styleComboBox->addItem(QIcon(resourcePath() + "/textures/pill.png"), "Licorice");
	m_styleComboBox->addItem(QIcon(resourcePath() + "/textures/backbone.png"), "Backbone");
	m_styleComboBox->addItem(QIcon(resourcePath() + "/textures/helix.png"), "Helix");
	m_styleComboBox->setMaximumHeight(27);

	m_scolorComboBox = new QComboBox(this);
	m_scolorComboBox->setStatusTip("Select the visualization color");
	m_scolorComboBox->setToolTip("Visualization color");
	mainToolBar->addWidget(m_scolorComboBox);
	m_scolorComboBox->addItem( QIcon(resourcePath() + "/textures/color-default.png"), "Default");
	m_scolorComboBox->addItem( QIcon(resourcePath() + "/textures/color-cpk.png"),"CPK");
	m_scolorComboBox->addItem( QIcon(resourcePath() + "/textures/color-residue.png"), "Residue");
	m_scolorComboBox->addItem( QIcon(resourcePath() + "/textures/color-layer.png"), "Layer Atom");
	m_scolorComboBox->addItem( QIcon(resourcePath() + "/textures/color-layer.png"), "Layer Residue");
	m_scolorComboBox->setMaximumHeight(27);

	//play feedback
	QAction* actionPreviousFrame = new QAction( QIcon(resourcePath() + "/textures/previous_frame.png"), "<", this);
	actionPreviousFrame->setStatusTip("Previous frame");
	actionPreviousFrame->setToolTip("Previous frame");

	actionPreviousFrame->setAutoRepeat(true);
	connect(actionPreviousFrame, SIGNAL(triggered()), frame, SLOT(previous()));
	mainToolBar->addAction(actionPreviousFrame);

	QAction* actionPlay = new QAction( QIcon(resourcePath() + "/textures/play.png"), "P", this);
	actionPlay->setStatusTip("Play or Pause");
	actionPlay->setToolTip("Play/Pause");
	connect(actionPlay, SIGNAL(triggered()), frame, SLOT(togglePlay()));

	connect(frame, &Tracker::onPlay, this, [this, actionPlay]{
		actionPlay->setIcon(QIcon(resourcePath() + "/textures/pause.png"));
	});

	connect(frame, &Tracker::onStop, this, [this, actionPlay]{
		actionPlay->setIcon(QIcon(resourcePath() + "/textures/play.png"));
	});


	mainToolBar->addAction(actionPlay);

	QAction* actionNextFrame = new QAction( QIcon(resourcePath() + "/textures/next_frame.png"), ">", this);
	actionNextFrame->setStatusTip("Next frame");
	actionNextFrame->setToolTip("Next frame");
	actionNextFrame->setAutoRepeat(true);
	connect(actionNextFrame, SIGNAL(triggered()), frame, SLOT(next()));
	mainToolBar->addAction(actionNextFrame);


	QSpinBox* spinBox = new QSpinBox(this);
	spinBox->setToolTip("Frame");
	spinBox->setStatusTip("Set frame");
	spinBox->setMaximum(frame->getMaxFrame());
	connect(frame, SIGNAL(valueChanged(int)), spinBox, SLOT(setValue(int)));
	connect(spinBox, SIGNAL(valueChanged(int)), frame, SLOT(setValue(int)));

	connect(frame->getTimeline(), &Timeline::nanosecondsUseVlaueChanged, this, [this, spinBox](bool use){
		if(use){
			spinBox->setSuffix("ns");
		}else{
			spinBox->setSuffix("");
		}
	});

	//connect(frame->parent(), SIGNAL(onMaxFrameValueChanged(int)), spinBox, SLOT(setValue(int)));
	connect(frame->getTimeline(), &Timeline::onMaxFrameValueChanged, this, [this, spinBox](int f){
		spinBox->setMaximum(f);
	});

	mainToolBar->addWidget(spinBox);

	//make Screenshot
	QAction* actionScreenshot = new QAction( QIcon(resourcePath() + "/textures/camera.png"), "S", this);
	actionScreenshot->setToolTip("Screenshot");
	actionScreenshot->setStatusTip("Save current view to clipboard");
	connect(actionScreenshot, &QAction::triggered, this, [this]{
		//QApplication::clipboard()->setImage(openGLWidget->createImage(), QClipboard::Clipboard);
		if(radioButton_WindowSize->isChecked())
			QApplication::clipboard()->setImage(openGLWidget->createImage(), QClipboard::Clipboard);
		else
			QApplication::clipboard()->setImage(openGLWidget->createImage(spinBox_ResWidth->value(), spinBox_ResHeight->value()), QClipboard::Clipboard);
		openGLWidget->blink();
	});
	mainToolBar->addAction(actionScreenshot);

	//settings
	QAction* actionShowSettigns = new QAction( QIcon(resourcePath() + "/textures/settings.png"), "Settings", this);
	actionShowSettigns->setStatusTip("Open menu to the right with more options");
	actionShowSettigns->setObjectName(QStringLiteral("actionShowSettigns"));
	actionShowSettigns->setCheckable(true);
	actionShowSettigns->setChecked(false);
    actionShowSettigns->setToolTip("Rendering settings");
    mainToolBar->addAction(actionShowSettigns);

    settingsFrame->hide();
    //connect(actionShowSettigns, SIGNAL(toggled(bool)), settingsFrame, SLOT(setHidden(bool)));
	connect(actionShowSettigns, &QAction::toggled, this, [this](bool b){
		settingsFrame->setHidden(!b);
		pushButtonSettings->setText((b)?  ">": "<"); //QString::fromUtf8("\u23F5")QString::fromUtf8("\u23F4")
	});
    connect(pushButtonSettings, SIGNAL(clicked()), actionShowSettigns, SLOT(toggle()));

	{//========= Setings View Atom Scale ========
	const float maxAtomSize = settings.value("Render/MaxAtomScale", 4.f).toFloat();
	const float atomSize = settings.value("Render/DefaultAtomScale", 1.f).toFloat();
	atomSizeHorizontalSlider->setValue( (atomSize/maxAtomSize)*1000 );
	atomSizeDoubleSpinBox->setMaximum(maxAtomSize);
	atomSizeDoubleSpinBox->setValue( atomSize );

	connect(atomSizeHorizontalSlider, &QSlider::valueChanged, this, [this, maxAtomSize](int value){
		openGLWidget->setAtomScale((maxAtomSize/1000.f)*value);
		atomSizeDoubleSpinBox->setValue((maxAtomSize/1000.f)*value);
	});

	connect(
		atomSizeDoubleSpinBox,static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged), [this, maxAtomSize]( double v ) {
		atomSizeHorizontalSlider->setValue( (v/maxAtomSize)*1000 );
		openGLWidget->setAtomScale((float)v);
	});

	//hide water
	connect(waterGroupBox, SIGNAL(toggled(bool)), openGLWidget, SLOT(setVisibitltyWater(bool)));


	//connect(waterGroupBox, SIGNAL(toggled(bool)), openGLWidget, SLOT(setHideWater(bool)));
	connect(waterProzentHorizontalSlider, SIGNAL(valueChanged(int)), waterProzentSpinBox, SLOT(setValue(int)));
	connect(waterProzentSpinBox, SIGNAL(valueChanged(int)), waterProzentHorizontalSlider, SLOT(setValue(int)));
	connect(waterProzentSpinBox, SIGNAL(valueChanged(int)), openGLWidget, SLOT(setWaterSkip(int)));
	}

	//axis
	checkBox_AxisHelper->setChecked(openGLWidget->isAxisEnabled());
	connect(checkBox_AxisHelper, SIGNAL(toggled(bool)), openGLWidget, SLOT(setAxisEnabled(bool)));
	//legend
	checkBox_heatscale->setChecked(openGLWidget->isHeatscaleEnabled());
	connect(checkBox_heatscale, SIGNAL(toggled(bool)), openGLWidget, SLOT(setHeatscaleEnabled(bool)));

	{//========= Setings View Screenshot ========
		if(radioButton_WindowSize->isChecked()){
			spinBox_ResWidth->setEnabled(false);
			spinBox_ResHeight->setEnabled(false);
			comboBox->setEnabled(false);
			checkBox_previewSize->setEnabled(false);
		}
		connect(radioButton_CustomSize, &QRadioButton::toggled, this, [this](bool checked){
			spinBox_ResWidth->setEnabled(checked);
			spinBox_ResHeight->setEnabled(checked);
			comboBox->setEnabled(checked);
			checkBox_previewSize->setEnabled(checked);
			if(checked && checkBox_previewSize->isChecked())
				openGLWidget->setPreviewSize(spinBox_ResWidth->value(), spinBox_ResHeight->value());
			else
				openGLWidget->setPreviewSize();
		});

		connect(spinBox_ResWidth, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), this, [this](int val){
			if(checkBox_previewSize->isChecked())
				openGLWidget->setPreviewSize(val, spinBox_ResHeight->value());
			else openGLWidget->setPreviewSize();
		});

		connect(spinBox_ResHeight, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), this, [this](int val){
			if(checkBox_previewSize->isChecked())
				openGLWidget->setPreviewSize(spinBox_ResWidth->value(), val) ;
			else openGLWidget->setPreviewSize();
		});

		connect(checkBox_previewSize, &QCheckBox::toggled, this, [this](bool checked){
			if(checked && radioButton_CustomSize->isChecked()){
				openGLWidget->setPreviewSize(spinBox_ResWidth->value(), spinBox_ResHeight->value());
			}else
				openGLWidget->setPreviewSize();
		});

		connect(comboBox, &QComboBox::currentTextChanged, this, [this](const QString& val){
			const QStringList items = val.split('x');
			if(items.size() == 2){
				spinBox_ResWidth->setValue(items[0].toInt());
				spinBox_ResHeight->setValue(items[1].toInt());
			}
		});


		//pushButton_SaveToClipboard->setIcon(QIcon(resourcePath() + "/textures/camera.png"));
		connect(pushButton_SaveToClipboard, &QPushButton::clicked, this, [this]{
			if(radioButton_WindowSize->isChecked())
				QApplication::clipboard()->setImage(openGLWidget->createImage(), QClipboard::Clipboard);
			else
				QApplication::clipboard()->setImage(openGLWidget->createImage(spinBox_ResWidth->value(), spinBox_ResHeight->value()), QClipboard::Clipboard);
			openGLWidget->blink();
		});

		//pushButton_SaveToFile->setIcon(QIcon(resourcePath() + "/textures/save.png"));
		connect(pushButton_SaveToFile, &QPushButton::clicked, this, [this, data, frame]{
			QString fileName = QFileDialog::getSaveFileName(this, tr("Save to file"), QFileInfo(data->getTitle()).baseName()+"_"+QString::number(frame->get()), tr("Portable Network Graphics (*.png);;  Joint Photographic Experts Group (*.jpg)"));

			if(fileName.isEmpty()) return;
			const bool isPNG = QFileInfo(fileName).suffix().toLower() == "png";
			QImage img = (radioButton_WindowSize->isChecked())?
					openGLWidget->createImage(isPNG):
					openGLWidget->createImage(spinBox_ResWidth->value(), spinBox_ResHeight->value(), isPNG);

			img.save(fileName);
		});
	}

	//========= Camera Settings ========

	connect(openGLWidget, &GLRenderWidget::cameraChanged, this, [this](){
		doubleSpinBox_orientation_x->blockSignals(true);
		doubleSpinBox_orientation_y->blockSignals(true);
		horizontalSlider_orientation_x->blockSignals(true);
		horizontalSlider_orientation_y->blockSignals(true);
		doubleSpinBox_position_x->blockSignals(true);
		doubleSpinBox_position_y->blockSignals(true);
		doubleSpinBox_position_z->blockSignals(true);
		doubleSpinBox_relposition_x->blockSignals(true);
		doubleSpinBox_relposition_y->blockSignals(true);
		doubleSpinBox_relposition_z->blockSignals(true);

		{
		const float anglePreX = openGLWidget->getCameraRotationX()+M_PI;
		const float angleX = (anglePreX < 0)? M_PIt2+anglePreX: fmodf(anglePreX, M_PIt2);
		horizontalSlider_orientation_x->setValue(100 * angleX/M_PIt2);
		doubleSpinBox_orientation_x->setValue( angleX);
		const float angleY = fmodf(openGLWidget->getCameraRotationY(), M_PI_2);
		horizontalSlider_orientation_y->setValue(100 * (angleY+M_PI_2)/M_PI);
		doubleSpinBox_orientation_y->setValue(angleY);
		}

		doubleSpinBox_position_x->setValue(openGLWidget->getCameraPositionX());
		doubleSpinBox_position_y->setValue(openGLWidget->getCameraPositionY());
		doubleSpinBox_position_z->setValue(openGLWidget->getCameraPositionZ());

		doubleSpinBox_relposition_x->setValue(openGLWidget->getCameraRelativePositionX());
		doubleSpinBox_relposition_y->setValue(openGLWidget->getCameraRelativePositionY());
		doubleSpinBox_relposition_z->setValue(openGLWidget->getCameraRelativePositionZ());

		doubleSpinBox_orientation_x->blockSignals(false);
		doubleSpinBox_orientation_y->blockSignals(false);
		horizontalSlider_orientation_x->blockSignals(false);
		horizontalSlider_orientation_y->blockSignals(false);
		doubleSpinBox_position_x->blockSignals(false);
		doubleSpinBox_position_y->blockSignals(false);
		doubleSpinBox_position_z->blockSignals(false);
		doubleSpinBox_relposition_x->blockSignals(false);
		doubleSpinBox_relposition_y->blockSignals(false);
		doubleSpinBox_relposition_z->blockSignals(false);
	});

	connect(doubleSpinBox_orientation_x, static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged), this, [this](double v){
		openGLWidget->setCameraRotationX((v < 0.f)? M_PI+v : fmodf(v,M_PIt2)-M_PI);
	});
	connect(doubleSpinBox_orientation_y, static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged), this, [this](double v){
		openGLWidget->setCameraRotationY(fmodf(v, M_PI_2));
	});

	connect(horizontalSlider_orientation_x, static_cast<void (QSlider::*)(int)>(&QSlider::valueChanged), this, [this](int v){
		doubleSpinBox_orientation_x->setValue(M_PIt2 * (v/99.f));
	});

	connect(horizontalSlider_orientation_y, static_cast<void (QSlider::*)(int)>(&QSlider::valueChanged), this, [this](int v){
		doubleSpinBox_orientation_y->setValue( (M_PI * (v/99.f)) - M_PI_2);
	});

	connect(doubleSpinBox_position_x, static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged), this, [this](double v){
		openGLWidget->setCameraPositionX(v);
	});
	connect(doubleSpinBox_position_y, static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged), this, [this](double v){
		openGLWidget->setCameraPositionY(v);
	});
	connect(doubleSpinBox_position_z, static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged), this, [this](double v){
		openGLWidget->setCameraPositionZ(v);
	});

	connect(doubleSpinBox_relposition_x, static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged), this, [this](double v){
		openGLWidget->setCameraRelativePositionX(v);
	});
	connect(doubleSpinBox_relposition_y, static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged), this, [this](double v){
		openGLWidget->setCameraRelativePositionY(v);
	});
	connect(doubleSpinBox_relposition_z, static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged), this, [this](double v){
		openGLWidget->setCameraRelativePositionZ(v);
	});

	//========= Parent Connections ========
	if(!parent) return;
	connect(parent, SIGNAL(updateGlLayers()), openGLWidget, SLOT(updateLayers()));
	connect(parent, SIGNAL(reloadGlShaders()), openGLWidget, SLOT(onReloadShaders()));

	connect(m_styleComboBox, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, [this](int index){
		openGLWidget->setStyleMode(static_cast<GLRenderWidget::style>(index));
		atomSizeDoubleSpinBox->setValue(openGLWidget->getAtomScale());
	});
	connect(m_scolorComboBox, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, [this](int index){
		openGLWidget->setColorMode(static_cast<GLRenderWidget::color>(index));
		atomSizeDoubleSpinBox->setValue(openGLWidget->getAtomScale());
	});

	connect(parent, SIGNAL(changeGlPropeRadius(float)), openGLWidget, SLOT(setPropeRadius(float)));

	//========= Data Connections ========
	connect(data, &Atoms::onModelDataChanged, this, [this, data](){
			if(data->getWaterCount()){
				waterGroupBox->setEnabled(true);
				waterProzentHorizontalSlider->setEnabled(true);
				waterProzentSpinBox->setEnabled(true);

				openGLWidget->setWaterSkip(waterProzentSpinBox->value());
			}else{
				waterGroupBox->setEnabled(false);
				waterProzentHorizontalSlider->setEnabled(false);
				waterProzentSpinBox->setEnabled(false);
			}
			atomSizeDoubleSpinBox->setValue(openGLWidget->getAtomScale());
		});


	setWidget(window);
}


RenderDockWidget::~RenderDockWidget() {
	// TODO Auto-generated destructor stub
}

