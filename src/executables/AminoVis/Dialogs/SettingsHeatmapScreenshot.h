/**
 * @file   		SettingsHeatmapScreenshot.h
 * @author 		Vladimir Ageev (vladimir.agueev@progsys.de)
 * @date   		17.09.2017
 *
 * @brief  		The settings side window dialog for the heatmap.
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
#ifndef EXECUTABLES_AMINOVIS_DIALOGS_SETTINGSHEATMAPSCREENSHOT_H_
#define EXECUTABLES_AMINOVIS_DIALOGS_SETTINGSHEATMAPSCREENSHOT_H_

#include <ui_settingsHeatmapScreenshot.h>
#include "HeatmapSettingsStruct.h"
#include <QDockWidget>
#include <QWidget>
#include <ColorLibrary.h>

class Timeline;

class SettingsHeatmapScreenshot: public QWidget, public Ui::SettingsHeatmapScreenshot {
	Q_OBJECT
public:
	SettingsHeatmapScreenshot(QWidget *parent = nullptr);
	virtual ~SettingsHeatmapScreenshot();
public slots:
	void updatePresets();
	void setup(ColorLibrary* colorLib, Timeline* timeline, heatmapScreenshotSettings* hss);
signals:
	void openSettingsMenu();
	void saveHeatmapToClipboard();
	void saveHeatmapToFile();
	void saveHeatmapToHTML();
private:
	ColorLibrary* m_colorLib = nullptr;
	heatmapScreenshotSettings* m_hss  = nullptr;
	Timeline* m_timeline = nullptr;
};

#endif /* EXECUTABLES_AMINOVIS_DIALOGS_SETTINGSHEATMAPSCREENSHOT_H_ */
