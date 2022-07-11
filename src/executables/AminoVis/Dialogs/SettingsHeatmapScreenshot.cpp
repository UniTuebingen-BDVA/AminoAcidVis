/*
 * SettingsHeatmapScreenshot.cpp
 *
 *  Created on: 17.09.2017
 *      Author: Vladimir Ageev (vladimir.agueev@progsys.de
 */

#include <Dialogs/SettingsHeatmapScreenshot.h>
#include <QWidget>
#include <Util/ResourcePath.h>
#include <Atoms/Timeline.h>

SettingsHeatmapScreenshot::SettingsHeatmapScreenshot(QWidget *parent):
QWidget(parent){
	setupUi(this);
	setWindowModality(Qt::NonModal);
	setWindowTitle("Heatmap Screenshot Settings");

	timelineDistanceLabel->setPixmap(QPixmap(resourcePath()+"/textures/double_arrow.png"));
	toolButton_timelineColorSettings->setIcon(QIcon(resourcePath() + "/textures/settings.png"));
	toolButton_styleSettigns->setIcon(QIcon(resourcePath() + "/textures/settings.png"));
	pushButton_toClipboard->setIcon(QIcon(resourcePath() + "/textures/camera.png"));
	connect(toolButton_timelineColorSettings, SIGNAL(clicked()), this, SIGNAL(openSettingsMenu()));
	connect(toolButton_styleSettigns, SIGNAL(clicked()), this, SIGNAL(openSettingsMenu()));
	connect(pushButton_toClipboard, SIGNAL(clicked()), this, SIGNAL(saveHeatmapToClipboard()));
	connect(pushButton_toFile, SIGNAL(clicked()), this, SIGNAL(saveHeatmapToFile()));
	connect(pushButton_toHTML, SIGNAL(clicked()), this, SIGNAL(saveHeatmapToHTML()));
}

SettingsHeatmapScreenshot::~SettingsHeatmapScreenshot() {
	// TODO Auto-generated destructor stub
}

void SettingsHeatmapScreenshot::setup(ColorLibrary* colorLib, Timeline* timeline, heatmapScreenshotSettings* hss){
	m_colorLib = colorLib;
	m_timeline = timeline;
	m_hss = hss;
	spinBox_width->setValue(m_hss->width);
	spinBox_rowHeight->setValue(m_hss->rowHeight);
	checkBox_xLabels->setChecked(m_hss->xLabels);
	checkBox_yLabels->setChecked(m_hss->yLabels);

	//========= Frame window set ============

	//frame setter
	connect(timeline, &Timeline::onStartValueChanged, this, [this](int time){
		endFrameSpinBox->setMinimum(time);
		startFrameSpinBox->setValue(time);
	});
	connect(timeline, &Timeline::onEndValueChanged, this, [this](int time){
		startFrameSpinBox->setMaximum(time);
		endFrameSpinBox->setValue(time);
	});

	connect(timeline, &Timeline::onMaxValueChanged, this, [this](int time){
		endFrameSpinBox->setMaximum(time);
	});


	connect(startFrameSpinBox, SIGNAL(valueChanged(int)), timeline, SLOT(setStart(int)));
	connect(endFrameSpinBox, SIGNAL(valueChanged(int)), timeline, SLOT(setEnd(int)));

	//nanoseconds
	connect(checkBoxUseNs, &QCheckBox::toggled, this, [this](bool checked){
		spinBox_Ns->setEnabled(checked);
		m_timeline->setUseNanoseconds(checked);
	});

	connect(m_timeline, &Timeline::nanosecondsUseVlaueChanged, this, [this](bool use){
		if(use){
			startFrameSpinBox->setSuffix("ns");
			endFrameSpinBox->setSuffix("ns");
		}else{
			startFrameSpinBox->setSuffix("");
			endFrameSpinBox->setSuffix("");
		}
	});

	spinBox_Ns->setValue(m_timeline->getNanosecondsLength());
	connect(spinBox_Ns, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), this, [this](int value){
		m_timeline->setNanosecondsLenght(value);
	});

	connect(m_timeline, SIGNAL(nanosecondsValueChanged(int)), spinBox_Ns, SLOT(setValue(int)));


	// ============= Color preset =============


	connect(comboBox_timelineColorPreset, &QComboBox::currentTextChanged, this, [this](const QString& text){
		m_colorLib->setPreset(text);
	});

	// ============= Screenshoot =============
	connect(checkBox_collapseAll, &QCheckBox::toggled, this, [this](bool checked){
		m_hss->collapseAll = checked;
	});


	connect(spinBox_width, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), this, [this](int value){
		m_hss->width = value;
	});

	connect(spinBox_rowHeight, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), this, [this](int value){
		m_hss->rowHeight = value;
	});

	connect(checkBox_xLabels, &QCheckBox::toggled, this, [this](bool checked){
		m_hss->xLabels = checked;
	});

	connect(checkBox_yLabels, &QCheckBox::toggled, this, [this](bool checked){
		m_hss->yLabels = checked;
	});

	connect(comboBox_font, &QComboBox::currentTextChanged, this, [this](const QString& text){
		m_hss->font = text;
	});

	updatePresets();

	connect(comboBox_style, &QComboBox::currentTextChanged, this, [this](const QString& text){
		m_hss->colorPreset = text;
	});
}

void SettingsHeatmapScreenshot::updatePresets(){
	const QStringList children = m_colorLib->getPresets();

	comboBox_timelineColorPreset->blockSignals(true);
	comboBox_timelineColorPreset->clear();
	comboBox_timelineColorPreset->addItems(children);
	comboBox_timelineColorPreset->setCurrentText(m_colorLib->getPreset());

	comboBox_timelineColorPreset->blockSignals(false);


	comboBox_style->blockSignals(true);
	comboBox_style->clear();
	comboBox_style->addItems(children);
	//qDebug()<<"1: m_hss->colorPreset"<<m_hss->colorPreset;
	if(children.contains(m_hss->colorPreset))
		comboBox_style->setCurrentText(m_hss->colorPreset);
	else{
		m_hss->colorPreset = "Default";
		comboBox_style->setCurrentText("Default");
	}
	comboBox_style->blockSignals(false);


}
