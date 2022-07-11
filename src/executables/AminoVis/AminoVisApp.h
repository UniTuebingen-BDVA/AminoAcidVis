/**
 * @file   		AminoVisApp.h
 * @author 		Vladimir Ageev (vladimir.agueev@progsys.de)
 * @date   		15.03.2017
 *
 * @brief  		The main window of the Amino Acid Visualization App.
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

#ifndef EXECUTABLES_AMINOVIS_AMINOVISAPP_H_
#define EXECUTABLES_AMINOVIS_AMINOVISAPP_H_

#include <Rendering/OpenGL.h>
#include <QString>
#include <QMainWindow>
#include <ui_aminoVisMainWindow.h>
#include <Atoms/Atoms.h>
#include <Atoms/Timeline.h>
#include <ColorLibrary.h>
#include <Atoms/ProteinSurface.h>
#include <QList>
#include <QMutex>
#include <QSettings>
#include "SurfaceLayersImageProvider.h"
#include "Dialogs/RenderDockWidget.h"
//#include "Dialogs/SettingsHeatmapScreenshot.h"
#include <QWidget>
#include <QElapsedTimer>

#include <Atoms/FilterList.h>
#include <Atoms/FilterAtoms.h>
#include "HeatmapSettingsStruct.h"

/*!
 * @brief The main window of this application. From here everything else is handled.
 */
class AminoVisApp: public QMainWindow, public Ui::WindowMain {
		Q_OBJECT
public:
	AminoVisApp(QWidget *parent = 0);
	virtual ~AminoVisApp();

	/*!
	 * @brief The close event is overwritten to store the window layout inside the config file.
	 * @see QSettings
	 */
	void closeEvent(QCloseEvent *event) override;

	bool isRowItemExpanded(const QModelIndex& row);

public slots:
	/*!
	 * @brief Main member function to load a protein and it's trajectory.
	 * This will display a file dialog to the user to select the files.
	 * @see doOpen(const QString& modelFileName, const QString& xtcFileName)
	 */
	void doOpen();
	/*!
	 * @brief Will open the given protein and it's trajectory.
	 * @param modelFileName Target protein file. Can be a Protein Database (.pdb) or a Gromos87 (.gro) file.
	 * @param xtcFileName Target trajectory file. Must be .xtc.
	 */
	void doOpen(const QString& modelFileName, const QString& xtcFileName);

	void setAtomInfo(int id);

	/*!
	 * @brief Creates a new gl rendering dock widget.
	 * @param frame Starting frame for the widget.
	 */
	void addGlWidget(int frame = 0);
	void removeGlWidget(RenderDockWidget* wd);

	QImage createHeatmapImage();

	void saveHeatmapToClipboard();
	void saveHeatmapToFile();
	void saveHeatmapToHTML();

	void openSettings(bool colorSettingsTop = false);

signals:
	void onStyleChanged();
	void updateHeatMap();

	//void glUpdate();
	void updateGlLayers();
	void reloadGlShaders();
	void changeGlPropeRadius(float radius);

	void GlCenterCamera();

	void timelineScrollTo(const QModelIndex& row);
private:
	void stopThreads();

	Atoms* m_data = nullptr; /// Main atom data, like name, position, layers, so. on.
	Timeline* m_timeline = nullptr; /// Handles the timeline
	ColorLibrary* m_colors = nullptr; /// Handles the colors, like the layer colors
	SurfaceLayersImageProvider* m_heatmapProvider = nullptr; /// Provides heatmap images to QML
	FilterAtomsListModel* m_filterAtomsListModel = nullptr; /// Used to apply filters or effects to the Atoms data
	QList<RenderDockWidget*> m_renderes; /// The docked widgets containing the GL renderer

	//QWidget* m_settingsHeatmapWindow = nullptr;

	float m_currentlyUsedPropeRadius = -1.f; /// The last used SAS radius
	bool m_animateSAS = false; /// Used to animate the SAS surface when the slider is moved
	QMetaObject::Connection m_timelineConnections[4]; /// Manages timeline tracker movement connections

	//surface extraction threads
	QList<ExtractSurfaceThread*> m_threads; /// Stores the threads during SAS layer extraction
	QTimer* m_extractSurfaceTimer; /// Timer to update the heatmap images while they are build
	QMutex m_threadMutex; /// Mutex gate used to organize data access

	///heatmap screenshot settings
	heatmapScreenshotSettings m_hss;

	/// Config file
	QSettings m_settings;

	///Timer
	QElapsedTimer m_SASExtractionTimer;
	qint64 m_SASTime = 0;
};

#endif /* EXECUTABLES_AMINOVIS_AMINOVISAPP_H_ */
