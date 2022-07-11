/*
 * AminoVisApp.cpp
 *
 *  Created on: 15.03.2017
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

#include <AminoVisApp.h>
#include <QFile>
#include <QtQml>
#include <QMessageBox>
#include <QProgressDialog>
#include <QDebug>

#include <QFileDialog>
#include <QShortcut>
#include <QElapsedTimer>

#include "Dialogs/SettingsWidget.h"

#include "DebugDockWidget.h"
#include "Dialogs/PlainTextMessageBox.h"
#include "Dialogs/FiltersHelpWidget.h"

#include "Dialogs/RenderDockWidget.h"
#include <Util/ResourcePath.h>
#include <QClipboard>

inline void about(QWidget *parent) {
    QMessageBox msgBox(parent);
    msgBox.setIcon(QMessageBox::Information);
    msgBox.setWindowTitle("About");
    //msgBox.setWindowIcon(QMessageBox::Information);
    msgBox.setTextFormat(Qt::RichText);
    msgBox.setText(
            "<b>AminoVis version " PROJECT_VERSION ".</b><br><br>"
            "For a chemist it is imported to see how a amino acid inside a protein behaves over time. Depending if it moves inwards or outwards the scientist can make conclusions on the binding behavior of the protein with other molecules. This tool allows a scientist to load a protein (.pdb;.gro) and it's trajectory (.xtc), visualize them with different methods and display their surface interactions in an interactive way.<br>"\

            "Written by Vladimir Ageev (agueev@uni-koblenz.de) for his master thesis."\
                "You can find the source code here: <a href='https://gitlab.uni-koblenz.de/agueev/AminoAcidVis'>https://gitlab.uni-koblenz.de/agueev/AminoAcidVis</a><br>"\

            "<br><b>GNU Lesser General Public License (LGPL):</b> <br>"\
                "<br>Copyright (C) 2017  Vladimir Ageev"\
                "<br><br>This program is free software: you can redistribute it and/or modify"\
                "<br>it under the terms of the GNU Lesser General Public License as published by"\
                "<br>the Free Software Foundation, version 3 of the License."\

            "<br><br>This program is distributed in the hope that it will be useful,"\
                "<br>but WITHOUT ANY WARRANTY; without even the implied warranty of"\
                "<br>MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the"\
                "<br>GNU Lesser General Public License for more details."\

            "<br><br>You should have received a copy of the GNU Lesser General Public License"\
                "<br>along with this program.  If not, see <a href='http://doc.qt.io/qt-5/lgpl.html'> http://doc.qt.io/qt-5/lgpl.html</a>"\
                "<br>or <a href='http://www.gnu.org/licenses/'> http://www.gnu.org/licenses/ </a>."\
);

    msgBox.exec();
}

#include <QStandardItemModel>
#include <QListWidgetItem>
#include "Dialogs/FrameContolWidget.h"

#include <QQuickItem>

AminoVisApp::AminoVisApp(QWidget *parent) : QMainWindow(parent),
                                            m_settings(configPath() + "/config.ini", QSettings::IniFormat, this) {
    setupUi(this);
    QMainWindow::centralWidget()->hide();

    if (!QFileInfo(configPath() + "/config.ini").exists()) {
        QMessageBox::warning(this, "Error", "Failed to find 'config.ini' file at:\n" + configPath(), QMessageBox::Ok);
    }

    //m_settings.sync();
    if (m_settings.status() != QSettings::NoError) {
        QMessageBox::critical(this, "Error", "Failed to parse 'config.ini' file!", QMessageBox::Ok);
        m_settings.clear();
    }

    //========= Setup data =========
    m_colors = new ColorLibrary(configPath() + "/colors.ini", this);
    m_data = new Atoms(resourcePath(), this);
    m_timeline = new Timeline(this);
    m_filterAtomsListModel = new FilterAtomsListModel(m_data, m_timeline, this);
    m_heatmapProvider = new SurfaceLayersImageProvider(m_data, m_timeline, m_colors);
    m_data->setData(m_timeline, m_filterAtomsListModel);

    addGlWidget(0);


    //data connections
    connect(m_data, &Atoms::onFramesChanged, this, [this] {
        m_timeline->setMaxFrame(m_data->numberOfFrames());
        m_timeline->reset();
    });

    //connect(this, SIGNAL(onStyleChanged()), openGLWidget, SLOT(onStyleChanged()));
    connect(this, SIGNAL(onStyleChanged()), m_colors, SIGNAL(onStyleChanged()));

    connect(m_colors, &ColorLibrary::colorsChanged, this, [this] {
        emit updateHeatMap();
    });

    connect(m_filterAtomsListModel, SIGNAL(filterHasChanged()), this, SIGNAL(updateHeatMap()));
    connect(m_filterAtomsListModel, SIGNAL(disableTimelineChanged()), this, SIGNAL(updateHeatMap()));

    //=========  File =========
    connect(actionOpenFiles, SIGNAL(triggered()), this, SLOT(doOpen()));

    connect(actionImport_Layer_Data, &QAction::triggered, this, [this] {
        if (*m_data && m_threads.empty()) {
            const QString fileName = QFileDialog::getOpenFileName(this, tr("Import Atom Layer Data"),
                                                                  QFileInfo(m_data->getTitle()).baseName(),
                                                                  tr("Binary Atom Layer Data (*.bald);; Comma Separated Values (*.csv)"));
            if (!fileName.isEmpty()) {
                float sasRadius;
                if (m_data->importLayerData(fileName, sasRadius)) {
                    m_currentlyUsedPropeRadius = sasRadius;
                    probeSizeDoubleSpinBox->setValue(sasRadius);

                    //openGLWidget->updateLayers();
                    emit updateGlLayers();
                    emit updateHeatMap();
                    windowStatusBar->showMessage("Successfully imported atom layer data from: " + fileName, 4000);
                } else {
                    QMessageBox::warning(this, "Failed to import!",
                                         "Failed to import atom layer data!\nTarget File: " + fileName,
                                         QMessageBox::Ok);
                    windowStatusBar->showMessage("Failed to import atom layer data!", 6000);
                }
            }
        }
    });

    connect(actionExport_Layer_Data, &QAction::triggered, this, [this] {
        if (*m_data && m_threads.empty()) {
            QString fileName = QFileDialog::getSaveFileName(this, tr("Export Atom Layer Data"),
                                                            QFileInfo(m_data->getTitle()).baseName(),
                                                            tr("Binary Atom Layer Data (*.bald);; Comma Separated Values (*.csv)"));
            if (!fileName.isEmpty()) {
                if (m_data->exportLayerData(fileName, m_currentlyUsedPropeRadius)) {
                    windowStatusBar->showMessage("Successfully exported atom layer data to: " + fileName, 4000);
                } else {
                    QMessageBox::warning(this, "Failed to export!",
                                         "Failed to export atom layer data!\nTarget File: " + fileName,
                                         QMessageBox::Ok);
                    windowStatusBar->showMessage("Failed to export atom layer data!", 6000);
                }
            }
        }
    });

    connect(actionSettings, SIGNAL(triggered()), this, SLOT(openSettings()));

    //=========  View menu =========
    //connect(actionCenterCamera, SIGNAL(triggered()), openGLWidget, SLOT(doCenterCamera()));
    //connect(actionKeepCentered, SIGNAL(toggled(bool)), openGLWidget, SLOT(setKeepCentered(bool)));

    connect(actionThemeLight, &QAction::triggered, this, [this] {
        setStyleSheet("");
        setPalette(QApplication::style()->standardPalette());
        m_colors->setPalette(palette());
        emit onStyleChanged();
        emit updateHeatMap();
    });

    connect(actionThemeDark, &QAction::triggered, this, [this] {
        QFile f(resourcePath() + "/styles/dark/style.qss");
        if (f.open(QFile::ReadOnly | QFile::Text)) {
            //QApplication::setStyle(QString(f.readAll()).arg(resourcePath + "/styles/dark" ));
            //QApplication::setStyle(QStyleFactory::create("Fusion"));
            setStyleSheet(QString(f.readAll()).arg(resourcePath() + "/styles/dark"));
            f.close();
            m_colors->setPalette(palette());
            emit onStyleChanged();//my custom emit
            emit updateHeatMap();
        } else
            qDebug() << "Unable to set stylesheet, file not found!";
    });

    connect(actionDefaultWindowState, &QAction::triggered, this, [this] {
        restoreGeometry(m_settings.value("Layout/DefaultGeometry").toByteArray());
        restoreState(m_settings.value("Layout/DefaultWindowState").toByteArray());
    });


    connect(timelineDockWidget, SIGNAL(visibilityChanged(bool)), actionTimeline_View, SLOT(setChecked(bool)));
    connect(actionTimeline_View, SIGNAL(triggered(bool)), timelineDockWidget, SLOT(setVisible(bool)));

    connect(timelineControlsDockWidget, SIGNAL(visibilityChanged(bool)), actionTimeline_Control_View,
            SLOT(setChecked(bool)));
    connect(actionTimeline_Control_View, SIGNAL(triggered(bool)), timelineControlsDockWidget, SLOT(setVisible(bool)));

    connect(extractDockWidget, SIGNAL(visibilityChanged(bool)), actionExtract_Surface_View, SLOT(setChecked(bool)));
    connect(actionExtract_Surface_View, SIGNAL(triggered(bool)), extractDockWidget, SLOT(setVisible(bool)));

    connect(selectionDockWidget, SIGNAL(visibilityChanged(bool)), actionSelection_View, SLOT(setChecked(bool)));
    connect(actionSelection_View, SIGNAL(triggered(bool)), selectionDockWidget, SLOT(setVisible(bool)));

    connect(filterDockWidget, SIGNAL(visibilityChanged(bool)), actionFilterView, SLOT(setChecked(bool)));
    connect(actionFilterView, SIGNAL(triggered(bool)), filterDockWidget, SLOT(setVisible(bool)));

    connect(actionShow_all_render_views, &QAction::triggered, this, [this] { ;
        for (RenderDockWidget *wd: m_renderes)
            wd->setVisible(true);
        m_renderes.first()->raise();
    });

    //========= Edit menu ==========
    connect(actionReloadShaders, SIGNAL(triggered()), this, SIGNAL(reloadGlShaders()));

    connect(actionReloadQML, &QAction::triggered, this, [this] { ;
        timelineQuickWidget->setSource(QUrl());
        timelineQuickWidget->engine()->clearComponentCache();
        timelineQuickWidget->setSource(QUrl::fromLocalFile(resourcePath() + "/qml/Timeline.qml"));
        PlainTextMessageBox::qmlErrors(this, "QML compile error!",
                                       "'/qml/Timeline.qml' failed to compile with the following message:",
                                       timelineQuickWidget->errors());

        filterQuickWidget->setSource(QUrl());
        filterQuickWidget->engine()->clearComponentCache();
        filterQuickWidget->setSource(QUrl::fromLocalFile(resourcePath() + "/qml/Filters.qml"));
        PlainTextMessageBox::qmlErrors(this, "QML compile error!",
                                       "'/qml/Filters.qml' failed to compile with the following message:",
                                       filterQuickWidget->errors());

    });

    //========= About menu =========
    connect(actionAbout_Qt, &QAction::triggered, this, [this] {
        QMessageBox::aboutQt(this);
    });

    connect(actionAboutApp, &QAction::triggered, this, [this] {
        about(this);
    });

    //setup staring style
    //does't work properly
    if (m_settings.value("Style", "Light").toString() == "Dark") {
        actionThemeDark->trigger();
        //force correct colors, is this a BUG?
        QPalette p = palette();
        p.setColor(QPalette::ColorRole::Window, QColor("#31363b"));
        p.setColor(QPalette::ColorRole::Base, QColor("#31363b"));
        p.setColor(QPalette::ColorRole::Text, QColor("#eff0f1"));
        p.setColor(QPalette::ColorRole::WindowText, QColor("#eff0f1"));
        setPalette(p);
        m_colors->setPalette(palette());
        //update();
    }

    {//========= Time edit ========

        timelineDistanceLabel->setPixmap(QPixmap(resourcePath() + "/textures/double_arrow.png"));
        //frame setter
        connect(m_timeline, &Timeline::onStartValueChanged, this, [this](int time) {
            endFrameSpinBox->setMinimum(time);
            startFrameSpinBox->setValue(time);
            emit updateHeatMap();
        });
        connect(m_timeline, &Timeline::onEndValueChanged, this, [this](int time) {
            startFrameSpinBox->setMaximum(time);
            endFrameSpinBox->setValue(time);
            emit updateHeatMap();
        });

        connect(m_timeline, &Timeline::onMaxValueChanged, this, [this](int time) {
            endFrameSpinBox->setMaximum(time);
            emit updateHeatMap();
        });

        connect(startFrameSpinBox, SIGNAL(valueChanged(int)), m_timeline, SLOT(setStart(int)));
        connect(endFrameSpinBox, SIGNAL(valueChanged(int)), m_timeline, SLOT(setEnd(int)));

        //add new
        connect(addGlWidgetPushButton, SIGNAL(clicked()), this, SLOT(addGlWidget()));

        //speed slider
        speedSlider->setValue(m_settings.value("Timeline/DefaultInterval", 300).toInt());
        connect(speedSlider, SIGNAL(valueChanged(int)), m_timeline, SLOT(setInterval(int)));
        m_timeline->setInterval(speedSlider->value());
        speedSpinBox->setValue(speedSlider->value());
        speedSlider->setMaximum(m_settings.value("Timeline/MaxInterval", 1000).toInt());
        speedSpinBox->setMaximum(m_settings.value("Timeline/MaxInterval", 1000).toInt());

        connect(speedSlider, SIGNAL(valueChanged(int)), speedSpinBox, SLOT(setValue(int)));
        connect(speedSpinBox, SIGNAL(valueChanged(int)), speedSlider, SLOT(setValue(int)));

        //smooth trajectories
        connect(smoothTrajectorysHorizontalSlider, SIGNAL(valueChanged(int)), smoothTrajectorysSpinBox,
                SLOT(setValue(int)));
        connect(smoothTrajectorysSpinBox, SIGNAL(valueChanged(int)), smoothTrajectorysHorizontalSlider,
                SLOT(setValue(int)));
        connect(smoothTrajectorysSpinBox, SIGNAL(valueChanged(int)), &m_data->getStream(), SLOT(setWindowRadius(int)));

        const int maxSmoothTrajectorys = m_settings.value("Render/MaxSmoothTrajectorys", 4).toInt();
        smoothTrajectorysSpinBox->setMaximum(maxSmoothTrajectorys);
        smoothTrajectorysHorizontalSlider->setMaximum(maxSmoothTrajectorys);
        smoothTrajectorysSpinBox->setValue(m_settings.value("Render/DefaultSmoothTrajectorys", 0).toInt());
    }

    {//========= Selection View ========

        //selector
        connect(m_data, &Atoms::onModelDataChanged, this, [this] {
            atomSelectionSpinBox->setMaximum(m_data->numberOfAtroms() - 1);
        });

        connect(atomSelectionSpinBox, SIGNAL(valueChanged(int)), m_data, SLOT(setSelectedAtom(int)));
        connect(atomSelectionSpinBox, SIGNAL(valueChanged(int)), this, SLOT(setAtomInfo(int)));

        connect(m_data, &Atoms::selectionChanged, this, [this] {
            atomSelectionSpinBox->setValue(m_data->getSelectedAtom());
        });

    }

    {//========= Filter View ========

        connect(filterPushButton, &QPushButton::clicked, this, [this] {
            if (filterLineEdit->text().isEmpty()) {
                m_filterAtomsListModel->refresh();
                return;
            }
            filterLineEdit->submit();
            m_filterAtomsListModel->addFilters(filterLineEdit->text());
            filterLineEdit->clear();
            /*
            qDebug() << filterQuickWidget->rootObject()->clipRect();
            qDebug() << filterQuickWidget->rootObject()->childrenRect();
            qDebug() << filterQuickWidget->rootObject()->boundingRect();
            qDebug() << filterQuickWidget->rootObject()->height();
            qDebug() << filterQuickWidget->rootObject()->width();
            */
        });

        connect(filterLineEdit, SIGNAL(returnPressed()), filterPushButton, SIGNAL(clicked()));

        connect(filterHelpToolButton, &QToolButton::clicked, this, [this] {
            FiltersHelpWidget filterHelp(m_filterAtomsListModel, this);
            filterHelp.setWindowModality(Qt::NonModal);
            filterHelp.exec();
            //filterHelp.show();
        });
        //filterListView->setModel(&m_filters);

    }

    {//========= Timeline View ========

        scrollToSelectedToolButton->setIcon(QIcon(resourcePath() + "/textures/center.png"));
        scrollToSelectedToolButton->setEnabled(false);

        connect(scrollToSelectedToolButton, &QToolButton::clicked, this, [this]() {
            int index = m_data->getSelectedAtom();
            if (index > 0) {
                //emit timelineScrollTo(m_data->index(m_data->getAtom(index).groupID-1, 0) );
                emit timelineScrollTo(m_data->index(m_data->getAtom(index).groupID - 1, 0));
            }
        });

        connect(m_data, &Atoms::selectionChanged, this, [this]() {
            if (m_data->getSelectedAtom() > 0)
                scrollToSelectedToolButton->setEnabled(true);
            else
                scrollToSelectedToolButton->setEnabled(false);
        });


        connect(filterToolButton, SIGNAL(toggled(bool)), m_filterAtomsListModel, SLOT(setEnabled(bool)));

        filterToolButton->setIcon(QIcon(resourcePath() + "/textures/filter2.png"));
        trackerComboBox->setMaximumHeight(27);

        connect(m_timeline, &Timeline::trackersSizeChanged, this, [this] {
            trackerComboBox->blockSignals(true);
            trackerComboBox->clear();
            for (int i = 0; i < m_timeline->getSize(); i++) {
                QPixmap pixmap(16, 16);
                pixmap.fill(m_timeline->get(i)->getColor());
                trackerComboBox->addItem(pixmap, QString::number(i + 1));
            }
            trackerComboBox->setCurrentIndex(m_timeline->getActiveTracker());
            trackerComboBox->blockSignals(false);
        });
        m_timeline->trackersSizeChanged();

        connect(trackerComboBox, SIGNAL(currentIndexChanged(int)), m_timeline, SLOT(setActiveTracker(int)));
        connect(m_timeline, SIGNAL(activeTrackerValueChanged(int)), trackerComboBox, SLOT(setCurrentIndex(int)));


        //play
        toolButton_PlayPause->setIcon(QIcon(resourcePath() + "/textures/play.png"));
        toolButton_nextFrame->setIcon(QIcon(resourcePath() + "/textures/next_frame.png"));
        toolButton_previousFrame->setIcon(QIcon(resourcePath() + "/textures/previous_frame.png"));

        connect(toolButton_PlayPause, &QToolButton::clicked, this, [this]() {
            m_timeline->get(m_timeline->getActiveTracker())->togglePlay();
        });

        connect(toolButton_nextFrame, &QToolButton::clicked, this, [this]() {
            m_timeline->get(m_timeline->getActiveTracker())->next();
        });

        connect(toolButton_previousFrame, &QToolButton::clicked, this, [this]() {
            m_timeline->get(m_timeline->getActiveTracker())->previous();
        });


        connect(m_timeline, &Timeline::activeTrackerValueChanged, this, [this](int index) {
            Tracker *t = m_timeline->get(index);

            spinBox->blockSignals(true);

            t->disconnect(m_timelineConnections[0]);
            m_timelineConnections[0] = connect(t, &Tracker::onPlay, this, [this] {
                toolButton_PlayPause->setIcon(QIcon(resourcePath() + "/textures/pause.png"));
            });
            t->disconnect(m_timelineConnections[1]);
            m_timelineConnections[1] = connect(t, &Tracker::onStop, this, [this] {
                toolButton_PlayPause->setIcon(QIcon(resourcePath() + "/textures/play.png"));
            });

            t->disconnect(m_timelineConnections[2]);
            m_timelineConnections[2] = connect(t, SIGNAL(valueChanged(int)), spinBox, SLOT(setValue(int)));
            t->disconnect(m_timelineConnections[3]);
            m_timelineConnections[3] = connect(spinBox, SIGNAL(valueChanged(int)), t, SLOT(setValue(int)));

            if (t->isActive()) {
                toolButton_PlayPause->setIcon(QIcon(resourcePath() + "/textures/pause.png"));
            } else
                toolButton_PlayPause->setIcon(QIcon(resourcePath() + "/textures/play.png"));

            spinBox->setValue(t->get());

            spinBox->blockSignals(false);
        });
        m_timeline->activeTrackerValueChanged(0);

        connect(m_timeline, &Timeline::nanosecondsUseVlaueChanged, this, [this](bool use) {
            if (use) {
                spinBox->setSuffix("ns");
                startFrameSpinBox->setSuffix("ns");
                endFrameSpinBox->setSuffix("ns");
            } else {
                spinBox->setSuffix("");
                startFrameSpinBox->setSuffix("");
                endFrameSpinBox->setSuffix("");
            }
        });

        spinBox->setMaximum(m_timeline->getMaxFrame());
        connect(m_timeline, &Timeline::onMaxFrameValueChanged, this, [this](int f) {
            spinBox->setMaximum(f);
        });

        //screenshot
        m_hss.width = m_settings.value("HeatmapScreenshot/width", 800).toInt();
        m_hss.rowHeight = m_settings.value("HeatmapScreenshot/rowHeight", 20).toInt();
        m_hss.font = m_settings.value("HeatmapScreenshot/Font", "Arial").toString();
        m_hss.colorPreset = m_settings.value("HeatmapScreenshot/ColorPreset", "Default").toString();
        m_hss.xLabels = m_settings.value("HeatmapScreenshot/PaintxLabels", false).toBool();
        m_hss.yLabels = m_settings.value("HeatmapScreenshot/PaintyLabels", false).toBool();
        m_hss.collapseAll = m_settings.value("HeatmapScreenshot/CollapseAll", false).toBool();

        if (!m_colors->getPresets().contains(m_hss.colorPreset))
            m_hss.colorPreset = "Default";


        toolButton_snapshot->setIcon(QIcon(resourcePath() + "/textures/camera.png"));
        connect(toolButton_snapshot, &QToolButton::clicked, this, [this] {
            saveHeatmapToClipboard();
        });

        //heatmap
        widget_timelineScreenshotSettingsArea->setup(m_colors, m_timeline, &m_hss);

        toolButton_snapshotSettings->setIcon(QIcon(resourcePath() + "/textures/settings.png"));
        toolButton_snapshotSettings->setStatusTip("Open menu to the right with more options");
        toolButton_snapshotSettings->setCheckable(true);
        toolButton_snapshotSettings->setChecked(false);
        toolButton_snapshotSettings->setToolTip("Heatmap settings");

        frame_heatmap_settings->hide();
        connect(toolButton_snapshotSettings, &QToolButton::toggled, this, [this](bool b) {
            frame_heatmap_settings->setHidden(!b);
            pushButtonTimelineSettings->setText(
                    (b) ? ">" : "<"); //QString::fromUtf8("\u23F5")QString::fromUtf8("\u23F4")
        });
        connect(pushButtonTimelineSettings, SIGNAL(clicked()), toolButton_snapshotSettings, SLOT(toggle()));

        connect(widget_timelineScreenshotSettingsArea, &SettingsHeatmapScreenshot::openSettingsMenu, this, [this] {
            openSettings(true);
        });

        connect(widget_timelineScreenshotSettingsArea, SIGNAL(saveHeatmapToClipboard()), this,
                SLOT(saveHeatmapToClipboard()));
        connect(widget_timelineScreenshotSettingsArea, SIGNAL(saveHeatmapToFile()), this, SLOT(saveHeatmapToFile()));
        connect(widget_timelineScreenshotSettingsArea, SIGNAL(saveHeatmapToHTML()), this, SLOT(saveHeatmapToHTML()));

    }

    {//========= Extract Surface View ========

        //probe size 1.4
        const float maxProbeRadius = m_settings.value("SurfaceExtraction/MaxProbeRadius", 4.f).toFloat();
        const float probeRadius = m_settings.value("SurfaceExtraction/DefaultProbeRadius", 1.4f).toFloat();

        probeSizeHorizontalSlider->setValue((probeRadius / maxProbeRadius) * 1000);
        probeSizeDoubleSpinBox->setMaximum(maxProbeRadius);
        probeSizeDoubleSpinBox->setValue(probeRadius);

        //default value
        QAction *probeSizeDefaultAct = new QAction(tr("Reset to default"), this);
        probeSizeDefaultAct->setStatusTip(tr("Set the prope radius the default value."));
        connect(probeSizeDefaultAct, &QAction::triggered, this, [this, probeRadius]() {
            probeSizeDoubleSpinBox->setValue(probeRadius);
        });
        probeSizeHorizontalSlider->addAction(probeSizeDefaultAct);

        //previous value
        QAction *probeSizePreviousAct = new QAction(tr("Reset to currently used value"), this);
        probeSizePreviousAct->setStatusTip(tr("Set the prope radius the currently used value."));
        connect(probeSizePreviousAct, &QAction::triggered, this, [this, probeRadius]() {
            if (m_currentlyUsedPropeRadius >= 0)
                probeSizeDoubleSpinBox->setValue(m_currentlyUsedPropeRadius);
            else
                probeSizeDoubleSpinBox->setValue(probeRadius);
        });
        probeSizeHorizontalSlider->addAction(probeSizePreviousAct);


        probeSizeHorizontalSlider->setContextMenuPolicy(Qt::ActionsContextMenu);


        connect(probeSizeHorizontalSlider, &QSlider::valueChanged, this, [this, maxProbeRadius](int value) {
            probeSizeDoubleSpinBox->setValue((maxProbeRadius / 1000.f) * value);;
        });

        connect(probeSizeHorizontalSlider, &QSlider::sliderPressed, this, [this]() {
            m_animateSAS = true;
        });

        connect(probeSizeHorizontalSlider, &QSlider::sliderReleased, this, [this]() {
            if (!showPropeSurfaceCheckBox->isChecked()) emit changeGlPropeRadius(0);
            m_animateSAS = false;
        });

        connect(
                probeSizeDoubleSpinBox, static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged),
                [this, maxProbeRadius](double v) {
                    probeSizeHorizontalSlider->setValue((v / maxProbeRadius) * 1000);
                    // to indicate the surface
                    emit changeGlPropeRadius((m_animateSAS || showPropeSurfaceCheckBox->isChecked()) ? v : 0.f);
                });

        //calculateSurface
        connect(m_data, &Atoms::onFramesChanged, this, [this] {
            if (m_data->numberOfFrames() > 0)
                extractSurfaceLayersLabel->setText("Ready to start extraction");
            else
                extractSurfaceLayersLabel->setText("No file loaded!");
        });

        m_extractSurfaceTimer = new QTimer(this);
        connect(extractSurfaceLayersPushButton, &QPushButton::clicked, this, [this] {
            if (m_timeline->getEndFrame() - m_timeline->getStartFrame() < 0 || m_data->numberOfFrames() <= 0) {
                extractSurfaceLayersLabel->setText("There needs to be at least one frame!");
                return;
            }
            const int maxNumberOfThreads = m_settings.value("SurfaceExtraction/Threads", 4).toInt();
            if (m_threads.empty()) { // if there are no threads running then we start
                if (m_data->numberOfFrames() && maxNumberOfThreads > 0 && maxNumberOfThreads < 42) {

                    //clear existing data if the probe radius is different
                    if (m_currentlyUsedPropeRadius != (float) probeSizeDoubleSpinBox->value())
                        for (Atoms::layerFrame &frame: m_data->getLayers())
                            frame.maxLayer = -1;
                    m_currentlyUsedPropeRadius = (float) probeSizeDoubleSpinBox->value();

                    m_SASExtractionTimer.start();

                    extractSurfaceLayersPushButton->setText("Stop");
                    extractSurfaceLayersProgressBar->setValue(0);
                    actionImport_Layer_Data->setEnabled(false);
                    actionExport_Layer_Data->setEnabled(false);

                    int step = (m_timeline->getEndFrame() - m_timeline->getStartFrame()) / (float) maxNumberOfThreads;
                    step += 1;
                    if (step <= 1) step = 100; // in case getEndFrame() is very small

                    for (int start = m_timeline->getStartFrame(); start <= m_timeline->getEndFrame(); start += step) {
                        qDebug() << "Starting thread" << start << "-" << (start + step) << ".";
                        ExtractSurfaceThread *thread = new ExtractSurfaceThread(m_data, start, start + step,
                                                                                probeSizeDoubleSpinBox->value(), this);

                        connect(thread, &QThread::finished,
                                [=]() {
                                    m_threadMutex.lock();
                                    qDebug() << "Thread Done!";
                                    QMutableListIterator<ExtractSurfaceThread *> i(m_threads);
                                    while (i.hasNext()) {
                                        if (i.next()->isFinished()) i.remove();
                                    }
                                    thread->deleteLater(); //QT manages the deletion for us
                                    m_threadMutex.unlock();
                                    //top timer if all threads finished
                                    if (m_threads.empty()) m_SASTime = m_SASExtractionTimer.elapsed();
                                });
                        //connect(thread, SIGNAL(finished()), thread, SLOT(deleteLater())); //QT manages the deletion for us

                        m_threads.push_back(thread);
                        thread->start();
                    }

                    //on timeout we collect all the progress of all threads
                    connect(m_extractSurfaceTimer, &QTimer::timeout,
                            [=]() {
                                m_threadMutex.lock();
                                if (m_threads.empty()) {
                                    extractSurfaceLayersProgressBar->setValue(100);
                                    m_extractSurfaceTimer->stop();
                                    extractSurfaceLayersPushButton->setText("Start");
                                    emit updateGlLayers();
                                    extractSurfaceLayersLabel->setText(
                                            "Finished in " + QString::number(m_SASTime) + "ms");
                                    actionImport_Layer_Data->setEnabled(true);
                                    actionExport_Layer_Data->setEnabled(true);
                                } else {
                                    float totalProgress = 0;
                                    float avarage = 0; //in ms
                                    int remaining = 0;
                                    for (ExtractSurfaceThread *th: m_threads) {
                                        totalProgress += (th->isFinished()) ? 1 : th->getProgress();
                                        remaining += th->getRemainingFrames();
                                        avarage += th->getAverageTime();
                                    }
                                    totalProgress /= m_threads.size();
                                    avarage /= m_threads.size();
                                    extractSurfaceLayersProgressBar->setValue(100 * totalProgress);

                                    //convert remaining time to minutes and seconds
                                    float milliseconds = ((remaining * avarage) / m_threads.size());
                                    int seconds = (int) fmodf((milliseconds / 1000), 60.f);
                                    int minutes = (int) ((milliseconds / 1000) / 60);
                                    extractSurfaceLayersLabel->setText("Average: " + QString::number(avarage)
                                                                       + "ms Remaining: " +
                                                                       QString("%1.%2").arg(minutes).arg(seconds, 2, 10,
                                                                                                         QChar('0')) +
                                                                       "min");
                                }

                                emit updateHeatMap();
                                m_threadMutex.unlock();
                            });
                    //connect(timer, SIGNAL(timeout()), this, SLOT(update()));
                    m_extractSurfaceTimer->setInterval(1000);
                    m_extractSurfaceTimer->start();

                    extractSurfaceLayersLabel->setText(
                            "Started with " + QString::number(m_threads.size()) + " threads");
                }
            } else { //we stop all threads
                stopThreads();
            }
            /*
            qDebug()<<"==========================";
            if(m_data->numberOfFrames()){
                extractSurface(m_data->getAtoms(), m_data->getFrame(m_timeline->getFrame()), atomSizeDoubleSpinBox->value()+probeSizeDoubleSpinBox->value(), atomSelectionSpinBox->value());
                openGLWidget->onFrameChanged();
            }
            */
        });


        connect(showPropeSurfaceCheckBox, &QCheckBox::toggled, this, [this](bool checked) {
            m_animateSAS = false;
            emit changeGlPropeRadius((checked) ? probeSizeDoubleSpinBox->value() : 0.f);
        });
    }


    //========= QML ========
    qmlRegisterType<Atoms>("AtomsData", 0, 1, "Atoms");
    qmlRegisterType<Timeline>("Timeline", 0, 1, "Timeline");
    qmlRegisterType<ColorLibrary>("ColorLib", 0, 1, "ColorLib");
    qmlRegisterType<Tracker>("Frame", 0, 1, "Frame");
    qmlRegisterType<FilterAtomsListModel>("FilteredAtoms", 0, 1, "FilteredAtoms");
    qmlRegisterType<Atom::FilterList>("FilterList", 0, 1, "FilterList");
    qmlRegisterType<FilterAtomsListModel>("FilterProxy", 0, 1, "FilterProxy");
    //qmlRegisterType<Timeline>("RenderWidget",0,1, "RenderWidget");


    timelineQuickWidget->engine()->addImageProvider(QLatin1String("imageprovider"), m_heatmapProvider);
    timelineQuickWidget->rootContext()->setContextProperty("atomsData", m_data);
    timelineQuickWidget->rootContext()->setContextProperty("timeline", m_timeline);
    timelineQuickWidget->rootContext()->setContextProperty("colorLib", m_colors);
    timelineQuickWidget->rootContext()->setContextProperty("mainWidget", this);
    timelineQuickWidget->rootContext()->setContextProperty("atomsFiltered", m_filterAtomsListModel);

    timelineQuickWidget->setSource(QUrl::fromLocalFile(resourcePath() + "/qml/Timeline.qml"));
    PlainTextMessageBox::qmlErrors(this, "QML compile error!",
                                   "'/qml/Timeline.qml' failed to compile with the following message:",
                                   timelineQuickWidget->errors());

    filterQuickWidget->rootContext()->setContextProperty("colorLib", m_colors);
    filterQuickWidget->rootContext()->setContextProperty("filters", &m_filterAtomsListModel->getFilterList());
    filterQuickWidget->rootContext()->setContextProperty("filtersProxy", m_filterAtomsListModel);
    filterQuickWidget->setSource(QUrl::fromLocalFile(resourcePath() + "/qml/Filters.qml"));
    PlainTextMessageBox::qmlErrors(this, "QML compile error!",
                                   "'/qml/Filters.qml' failed to compile with the following message:",
                                   filterQuickWidget->errors());


    //selectionDockWidget->close();
    //renderingDockWidget->close();
    //tabifyDockWidget(dockWidget,renderingDockWidget);

    //restore the window layout
    restoreGeometry(m_settings.value("Layout/Geometry").toByteArray());
    restoreState(m_settings.value("Layout/WindowState").toByteArray());
    m_renderes[0]->setVisible(true);
    m_renderes[0]->raise();


    //========= DEBUG ========


    /*
    DebugDockWidget* w = new DebugDockWidget(m_data, m_timeline, this);

    connect(w->extractSurfaceDebugPushButton, &QPushButton::clicked, this, [this]{
        if(atomSelectionSpinBox->value() >= 0 )
            debugExtractSurface(m_data->getAtoms(), m_data->getFrame(m_timeline->getFrame()), (float)probeSizeDoubleSpinBox->value(), atomSelectionSpinBox->value());
    });

    connect(w, SIGNAL(updateAtomPositionGL(int)), openGLWidget, SLOT(updatePosition(int)));
    connect(w, SIGNAL(updateAtomRadiusGL(int)), openGLWidget, SLOT(updateRadius(int)));

    addDockWidget(Qt::RightDockWidgetArea, w);
    */

    //just print out in what reading order mode your CPU is
    int num = 1;
    if (*(char *) &num == 1)
        qDebug() << "Your CPU is in 'Little-Endian' mode.";
    else
        qDebug() << "Your CPU is in 'Big-Endian' mode.";
}

AminoVisApp::~AminoVisApp() {
    for (ExtractSurfaceThread *th: m_threads) {
        th->deleteLater();
        th->requestInterruption();
        th->wait(5000);
    }

    if (m_data) delete m_data;
    if (m_timeline) delete m_timeline;
    if (m_colors) delete m_colors;
}


void AminoVisApp::closeEvent(QCloseEvent *event) {
    //save the window layout when closing app
    m_settings.setValue("Layout/Geometry", saveGeometry());
    m_settings.setValue("Layout/WindowState", saveState());

    //save heatmap screenshot settigns
    m_settings.setValue("HeatmapScreenshot/width", m_hss.width);
    m_settings.setValue("HeatmapScreenshot/rowHeight", m_hss.rowHeight);
    m_settings.setValue("HeatmapScreenshot/Font", m_hss.font);
    m_settings.setValue("HeatmapScreenshot/ColorPreset", m_hss.colorPreset);
    m_settings.setValue("HeatmapScreenshot/PaintxLabels", m_hss.xLabels);
    m_settings.setValue("HeatmapScreenshot/PaintyLabels", m_hss.yLabels);
    m_settings.setValue("HeatmapScreenshot/CollapseAll", m_hss.collapseAll);

    //close QML widget before other elements get deleted
    //so they don't access invalid data
    timelineQuickWidget->setSource(QUrl());
    timelineQuickWidget->engine()->clearComponentCache();

    filterQuickWidget->setSource(QUrl());
    filterQuickWidget->engine()->clearComponentCache();

    QMainWindow::closeEvent(event);
}


void AminoVisApp::doOpen() {
    const QString modelFileName = QFileDialog::getOpenFileName(this, tr("Open pdb"), "",
                                                               tr("Data (*.pdb *.gro );;Protein Database (*.pdb );;Gromos87 (*.gro )"));
    QString path;
    if (modelFileName.isEmpty())
        return;
    else {
        path = QFileInfo(modelFileName).path(); // store path for next file dialog
    }

    const QString xtcFileName = QFileDialog::getOpenFileName(this, tr("Open xtc"), path, tr("Data (*.xtc )"));

    if (xtcFileName.isEmpty())
        return;

    doOpen(modelFileName, xtcFileName);
    emit GlCenterCamera();
}

void AminoVisApp::doOpen(const QString &pbdFileName, const QString &xtcFileName) {
    setEnabled(false);
    m_filterAtomsListModel->clearFilters();
    stopThreads();
    m_timeline->stopAll();

    //show a progress dialog
    QProgressDialog progress(this);
    progress.setWindowFlags(Qt::Window | Qt::WindowTitleHint | Qt::CustomizeWindowHint | Qt::WindowStaysOnTopHint);
    progress.setWindowTitle("Please wait");
    progress.setWindowModality(Qt::WindowModal);
    progress.setLabelText("File opening in progress...");
    progress.setCancelButton(0);

    //progress.setRange(0,100);
    progress.setRange(0, 0);
    progress.show();
    QApplication::processEvents();
    /*
    QFuture<bool> thread = QtConcurrent::run(m_data, &Atoms::open, pbdFileName, xtcFileName);

    while(thread.isRunning()){
        //progress.
        //progress.setValue(m_treeModel->getProgress());
        //QApplication::processEvents();
    }
     */
    bool opened = m_data->open(pbdFileName, xtcFileName);
    QApplication::processEvents();

    if (opened) {
        setWindowTitle("Amino Vis - " + m_data->getTitle() + " and " + QFileInfo(xtcFileName).fileName());
        windowStatusBar->showMessage("Successfully opened files!", 4000);
        actionImport_Layer_Data->setEnabled(true);
        actionExport_Layer_Data->setEnabled(true);
    } else {
        actionImport_Layer_Data->setEnabled(false);
        actionExport_Layer_Data->setEnabled(false);
    }
    //m_timeline->setEndFrame(m_timeline->getMaxFrame());

    //settings
    smoothTrajectorysSpinBox->blockSignals(true);
    smoothTrajectorysSpinBox->setValue(0);
    smoothTrajectorysSpinBox->blockSignals(false);

    smoothTrajectorysHorizontalSlider->blockSignals(true);
    smoothTrajectorysHorizontalSlider->setValue(0);
    smoothTrajectorysHorizontalSlider->blockSignals(false);

    progress.close();
    setEnabled(true);
}

void AminoVisApp::setAtomInfo(int id) {
    if (id < 0 || id >= (int) m_data->numberOfAtroms()) {
        atomInfoTextBrowser->setText("No selection.");
        return;
    }
    const Atoms::atom &a = m_data->getAtom(id);
    const TrajectoryStream::xtcFrame &f = m_data->getCurrentFrame();
    const Atoms::layerFrame &l = m_data->getLayer(m_timeline->get(m_timeline->getActiveTracker())->get());
    atomInfoTextBrowser->setText(
            "Name: " + a.name + "\n"
            + "Residue: " + a.residue + "\n"
            + "Position: (" + QString::number(f.positions[id].x) + ", " + QString::number(f.positions[id].y) + ", " +
            QString::number(f.positions[id].z) + ")\n"
            + "Layer: " + QString::number(((l.layers.empty()) ? 0 : l.layers[id])) + "\n"
            + "Residue Layer: " + QString::number(
                    m_data->getGroupLayer(a.groupID - 1, m_timeline->get(m_timeline->getActiveTracker())->get())) + "\n"
            + "Max layer: " + QString::number(((l.layers.empty()) ? 0 : l.maxLayer)) + "\n"
    );

}

static unsigned int glRenderCount = 0;

void AminoVisApp::addGlWidget(int frame) {
    Tracker *frameObject = m_timeline->addTracker(frame);
    RenderDockWidget *GlWidget = new RenderDockWidget(m_settings, m_data, frameObject, m_colors, m_filterAtomsListModel,
                                                      this);
    QListWidgetItem *item = new QListWidgetItem(listWidgetFrames);
    FrameContolWidget *control = new FrameContolWidget(frameObject, listWidgetFrames);

    GlWidget->setWindowTitle("Render View " + QString::number(m_renderes.size() + 1));
    GlWidget->setObjectName("renderView" + QString::number(glRenderCount));
    glRenderCount++;

    connect(GlWidget, SIGNAL(visibilityChanged(bool)), control, SLOT(dockVisibilityChanged(bool)));
    connect(control, &FrameContolWidget::toggleDockVisibility, this, [this, GlWidget](bool visible) {
        if (visible) {
            GlWidget->setVisible(true);
            GlWidget->raise();
        } else
            GlWidget->setVisible(false);

    });
    //connect(control, SIGNAL(toggleDockVisibility(bool)), GlWidget, SLOT(setVisible(bool)));

    if (m_renderes.empty()) {
        addDockWidget(Qt::RightDockWidgetArea, GlWidget);
        control->pushButton_close->setDisabled(true);
        //always show the first created dock widget
        GlWidget->setVisible(true);
        GlWidget->raise();
    } else {
        tabifyDockWidget(m_renderes.back(), GlWidget);
        static_cast<FrameContolWidget *>(listWidgetFrames->itemWidget(
                listWidgetFrames->item(0)))->pushButton_close->setDisabled(false);
    }
    m_renderes.push_back(GlWidget);


    listWidgetFrames->addItem(item);
    connect(control->pushButton_close, &QPushButton::clicked, this, [this, GlWidget] {
        this->removeGlWidget(GlWidget);
    });

    item->setSizeHint(control->minimumSizeHint());
    listWidgetFrames->setItemWidget(item, control);

}

void AminoVisApp::removeGlWidget(RenderDockWidget *wd) {
    if (m_renderes.size() < 2) { //there must be at least one
        return;
    }

    int i = m_renderes.indexOf(wd);
    if (i < 0) return;

    m_renderes.removeOne(wd);
    Tracker *frame = m_timeline->get(i);
    m_timeline->removeTracker(i);

    QListWidgetItem *item = listWidgetFrames->item(i);
    QWidget *widget = listWidgetFrames->itemWidget(item);
    listWidgetFrames->removeItemWidget(item);

    wd->close();

    delete item;
    delete widget;
    delete wd;
    delete frame;

    i = 1;
    for (RenderDockWidget *wid: m_renderes) {
        wid->setWindowTitle("Render View " + QString::number(i));
        i++;
    }
    if (m_renderes.size() == 1)
        static_cast<FrameContolWidget *>(listWidgetFrames->itemWidget(
                listWidgetFrames->item(0)))->pushButton_close->setDisabled(true);


}

bool AminoVisApp::isRowItemExpanded(const QModelIndex &row) {
    QVariant returnedValue;
    QVariant msg = row;
    QMetaObject::invokeMethod(timelineQuickWidget->rootObject(), "isExpanded",
                              Q_RETURN_ARG(QVariant, returnedValue),
                              Q_ARG(QVariant, msg));

    //qDebug() << "QML function returned:" << returnedValue.toBool();
    return returnedValue.toBool();
}

QImage AminoVisApp::createHeatmapImage() {
    if (!m_heatmapProvider || m_hss.width <= 0 || m_data->numberOfGroups() <= 0 || m_hss.rowHeight <= 0)
        return QImage();


    const int yLableWidth = (m_hss.rowHeight >= 30) ? 100 : 55;
    const int xLableHeight = (m_hss.rowHeight >= 30) ? m_hss.rowHeight : 20;

    const int heatmapWidth = (m_hss.yLabels) ? m_hss.width - yLableWidth : m_hss.width;
    const int heatmapXOffset = (m_hss.yLabels) ? yLableWidth : 0;
    const int heatmapYOffset = (m_hss.xLabels) ? xLableHeight : 0;


    struct paintItem {
        int id;
        bool isResidue;
        bool last;
    };
    QVector<paintItem> paintItems;
    {//collect all items to paint
        paintItems.reserve(m_filterAtomsListModel->rowCount());
        //qDebug()<<"rowCount: "<<m_filterAtomsListModel->rowCount();
        for (int i = 0; i < m_filterAtomsListModel->rowCount(); i++) {
            const QModelIndex index_filtered = m_filterAtomsListModel->index(i, 0);
            const QModelIndex index_source = m_filterAtomsListModel->mapToSource(index_filtered);
            if (m_data->getGroupName(index_source.row()) == "HOH") continue;

            const bool isExpanded = isRowItemExpanded(index_filtered);

            paintItems.push_back({index_source.row(), true, false});
            //qDebug()<<index_source.row()<<", "<<"true"<<", isExpanded: "<<isExpanded;

            if (isExpanded && !m_hss.collapseAll) { // && m_hss.paintAtoms
                //qDebug()<<"atom rowCount: "<<m_filterAtomsListModel->rowCount(index_filtered);
                const int firstAtomIndex = m_data->getGroupStartID(index_source.row());
                if (m_filterAtomsListModel->rowCount(index_filtered) > 0) {
                    for (int a = 0; a < m_filterAtomsListModel->rowCount(index_filtered); a++) {
                        const QModelIndex index_atom_filtered = m_filterAtomsListModel->index(a, 0, index_filtered);
                        const QModelIndex index_atom_source = m_filterAtomsListModel->mapToSource(index_atom_filtered);
                        paintItems.push_back({firstAtomIndex + index_atom_source.row(), false, false});
                        //qDebug()<<"  -- "<<(firstAtomIndex+index_atom_source.row())<<", "<<"false";
                    }
                    paintItems.last().last = true;
                }
            }
        }
    }

    const int totalHeight = paintItems.size() * m_hss.rowHeight + ((m_hss.xLabels) ? xLableHeight : 0);
    qDebug() << "totalHeight: " << totalHeight << paintItems.size() << m_hss.rowHeight;
    if (totalHeight > 8000) {
        QMessageBox::critical(this, "Error",
                              "Heatmap image height is too big!\nTotal height would be " + QString::number(totalHeight)
                              + "px.\n Use filters to reduce the number of items.");
        return QImage();
    }

    QImage out_image(m_hss.width, totalHeight, QImage::Format_RGB32);
    if (out_image.isNull()) {
        QMessageBox::critical(this, "Error", "Heatmap image is too big or has invalid values!\nTotal height would be " +
                                             QString::number(totalHeight) + "px.");
        return QImage();
    }
    out_image.fill(Qt::white);
    QPainter painter(&out_image);
    painter.setPen(Qt::black);

    //adjust font size
    {
        painter.setFont(QFont(m_hss.font, xLableHeight - 1));
        const float factor = (xLableHeight - 2) / ((float) painter.fontMetrics().height() + (xLableHeight * 0.5f));
        if ((factor < 1) || (factor > 1.25)) {
            QFont f = painter.font();
            f.setPointSizeF(f.pointSizeF() * factor);
            painter.setFont(f);
        }
    }

    if (m_hss.xLabels) {
        const int count =
                heatmapWidth / (float) (painter.fontMetrics().width(QString::number(m_timeline->getEnd())) + 15);
        const int step = heatmapWidth / count;
        for (int i = 0; i < count; i++) {
            painter.drawText(QRect(heatmapXOffset + step * i + 3, 4, step, xLableHeight), Qt::AlignLeft | Qt::AlignTop,
                             QString::number(std::floor(m_timeline->getStart() +
                                                        i * (m_timeline->getEnd() - m_timeline->getStart()) /
                                                        std::round(heatmapWidth / (float) step))));
            painter.drawLine(heatmapXOffset + step * i, 0, heatmapXOffset + step * i, xLableHeight);
        }
    }
    /////DRAW HEATMAP
    //adjust font size
    QFont residueFont;
    QFont atomFont;
    {
        painter.setFont(QFont(m_hss.font, m_hss.rowHeight - 1));
        const float factor = (yLableWidth - 5) / (float) painter.fontMetrics().width("22:GGGG");
        if ((factor < 1) || (factor > 1.25)) {
            residueFont = painter.font();
            residueFont.setPointSizeF(residueFont.pointSizeF() * factor);
            atomFont = painter.font();
            atomFont.setPointSizeF(atomFont.pointSizeF() * factor * 0.8f);
        }
    }
    const QString prevPreset = m_colors->getPreset();
    m_colors->blockSignals(true);
    m_colors->setPreset(m_hss.colorPreset);

    int yOffset = heatmapYOffset;
    for (const paintItem &item: paintItems) {

        if (m_hss.yLabels) {
            if (item.isResidue) {
                painter.setFont(residueFont);
                const QString groupName = m_data->getGroupName(item.id);
                painter.drawText(QRect(1, yOffset, yLableWidth, m_hss.rowHeight), Qt::AlignLeft | Qt::AlignVCenter,
                                 QString::number(item.id + 1) + ":" + groupName);
            } else {
                painter.setFont(atomFont);
                painter.drawText(QRect(1, yOffset, yLableWidth, m_hss.rowHeight), Qt::AlignLeft | Qt::AlignVCenter,
                                 ((item.last) ? "\u2514 " : "\u251C ") + QString::number(item.id + 1) + ":" +
                                 m_data->getAtom(item.id).name);
                //2514
            }
        }
        painter.drawImage(QRect(heatmapXOffset, yOffset, heatmapWidth, m_hss.rowHeight),
                          m_heatmapProvider->requestImage(item.isResidue, item.id, m_colors->p_layersColors, nullptr,
                                                          {heatmapWidth, m_hss.rowHeight}));

        yOffset += m_hss.rowHeight;
    }

    m_colors->setPreset(prevPreset);
    m_colors->blockSignals(false);

    return out_image;
}

void AminoVisApp::saveHeatmapToClipboard() {
    const QImage img = createHeatmapImage();
    if (!img.isNull()) {
        QApplication::clipboard()->setImage(img, QClipboard::Clipboard);
        windowStatusBar->showMessage("Successfully saved heatmap to clipboard!", 4000);
    } else
        windowStatusBar->showMessage("Failed to saved heatmap to clipboard!", 4000);
}

void AminoVisApp::saveHeatmapToFile() {
    if (m_data->numberOfAtroms() <= 0) return;
    QString fileName = QFileDialog::getSaveFileName(this, tr("Save as image"),
                                                    QFileInfo(m_data->getTitle()).baseName() + "_heatmap",
                                                    tr("Portable Network Graphics (*.png);;  Joint Photographic Experts Group (*.jpg)"));
    if (fileName.isEmpty()) return;

    const QImage img = createHeatmapImage();

    if (!img.isNull()) {
        img.save(fileName);
        windowStatusBar->showMessage(QString("Successfully saved heatmap to file: ") + fileName + "!", 4000);
    } else
        windowStatusBar->showMessage("Failed to saved heatmap to file!", 4000);
}

void AminoVisApp::saveHeatmapToHTML() {
    if (m_data->numberOfAtroms() <= 0) return;
    QString fileName = QFileDialog::getSaveFileName(this, tr("Save as HTML"),
                                                    QFileInfo(m_data->getTitle()).baseName() + "_heatmap",
                                                    tr("Hypertext Markup Language (*.html)"));
    if (fileName.isEmpty()) return;


    struct paintItem {
        int id;
        bool isResidue;
        bool last;
    };
    QVector<paintItem> paintItems;
    {//collect all items to paint
        paintItems.reserve(m_filterAtomsListModel->rowCount());
        //qDebug()<<"rowCount: "<<m_filterAtomsListModel->rowCount();
        for (int i = 0; i < m_filterAtomsListModel->rowCount(); i++) {
            const QModelIndex index_filtered = m_filterAtomsListModel->index(i, 0);
            const QModelIndex index_source = m_filterAtomsListModel->mapToSource(index_filtered);
            if (m_data->getGroupName(index_source.row()) == "HOH") continue;

            const bool isExpanded = isRowItemExpanded(index_filtered);

            paintItems.push_back({index_source.row(), true, false});
            //qDebug()<<index_source.row()<<", "<<"true"<<", isExpanded: "<<isExpanded;

            if (isExpanded && !m_hss.collapseAll) { // && m_hss.paintAtoms
                //qDebug()<<"atom rowCount: "<<m_filterAtomsListModel->rowCount(index_filtered);
                const int firstAtomIndex = m_data->getGroupStartID(index_source.row());
                if (m_filterAtomsListModel->rowCount(index_filtered) > 0) {
                    for (int a = 0; a < m_filterAtomsListModel->rowCount(index_filtered); a++) {
                        const QModelIndex index_atom_filtered = m_filterAtomsListModel->index(a, 0, index_filtered);
                        const QModelIndex index_atom_source = m_filterAtomsListModel->mapToSource(index_atom_filtered);
                        paintItems.push_back({firstAtomIndex + index_atom_source.row(), false, false});
                        //qDebug()<<"  -- "<<(firstAtomIndex+index_atom_source.row())<<", "<<"false";
                    }
                    paintItems.last().last = true;
                }
            }
        }
    }

    //save HTML
    QFile data(fileName);
    if (data.open(QFile::WriteOnly)) {
        QTextStream out(&data);
        out << "<!DOCTYPE html>\n<html>\n<head>\n"
               "\t<!-- File crated by AminoVis using plotly -->\n"
               "\t<!-- More about plotly: https://github.com/plotly/plotly.js -->\n"
               "\t<meta charset=\"utf-8\" />\n";
        out << "\t<title>" << QFileInfo(m_data->getTitle()).baseName() << " heatmap</title>\n";
        out << "\t<!-- Latest compiled and minified plotly.js JavaScript -->\n";
        out << "\t<script type=\"text/javascript\" src=\"https://cdn.plot.ly/plotly-latest.min.js\"></script>\n";
        out << "\t<style type=\"text/css\">\n";
        out << "\tbody, html {\n";
        out << "\t\theight: 98%;\n";
        out << "\t}\n";
        out << "\t</style>\n";
        out << "</head>\n";
        out << "<body>\n";
        out << "\t<div id=\"heatmap\" style=\"width:100%;height:100%\"></div>\n";
        out << "\t<script type=\"text/javascript\">\n";
        out << "\tHEATMAP = document.getElementById('heatmap');\n";
        out << "\tPlotly.plot( HEATMAP, [{\n";
        out << "\"z\": [\n";
        for (const paintItem &item: paintItems) {
            out << "[";
            if (item.isResidue) {
                for (int i = 0; i < m_timeline->getMaxFrame(); i++)
                    out << m_data->getGroupLayerAvarage(item.id, i) << ",";
            } else {
                for (int i = 0; i < m_timeline->getMaxFrame(); i++)
                    out << m_data->getAtomLayer(item.id, i) << ",";
            }
            out << "],\n";
        }
        out << "],\n";

        //x axis frames
        out << "\"x\": [";
        for (int i = 0; i < m_timeline->getMaxFrame(); i++)
            out << i << ",";
        out << "],\n";

        //y axis names
        out << "\"y\": [";
        for (const paintItem &item: paintItems) {
            if (item.isResidue) {
                out << "\"" << (item.id + 1) << ":" << m_data->getGroupName(item.id) << "\",";
            } else {
                out << "\"" << "\\u2022 " << (item.id + 1) << ":" << m_data->getAtom(item.id).name << "\",";
            }
        }
        out << "],\n";


        out << "\"type\": \"heatmap\",";
        out << "\"colorbar\": {\"title\": \"Layer\"}, ";

        const QString prevPreset = m_colors->getPreset();
        m_colors->blockSignals(true);
        m_colors->setPreset(m_hss.colorPreset);
        out << "\"zmin\": 0, ";
        out << "\"zmax\": " << m_colors->p_layersColors.size() << ", ";
        out << "\"colorscale\": [";
        {
            float r = 0;
            const float step = 1.f / (m_colors->p_layersColors.size() - 1);
            for (const rawcolor &c: m_colors->p_layersColors) {
                out << "[" << r << ",\"rgb(" << c.bgra.r << "," << c.bgra.g << "," << c.bgra.b << ")\"],";
                r += step;
            }
        }
        out << "],";
        m_colors->setPreset(prevPreset);
        m_colors->blockSignals(false);
        //"\"YlGnBu\"";
        out << "\"showlegend\": " << ((m_hss.yLabels) ? "true" : "false");
        out << "}],\n";
        //Nanoseconds
        out << "{\"name\": \"layer\", \"title\": \"" << QFileInfo(m_data->getTitle()).baseName()
            << "\", \"xaxis\": {\"title\": \"Frames\", \"side\": \"bottom\"}, \"yaxis\": {\"tickfont\": {\"size\": 18}}, \"font\": {\"size\": 32}} );\n";
        out << "\t</script>\n";
        out << "</body>\n";
        out << "</html>";
    }
    data.close();
}

void AminoVisApp::openSettings(bool colorSettingsTop) {
    SettingsWidget settingsWindow(m_settings, m_colors, colorSettingsTop, this);
    connect(&settingsWindow, SIGNAL(onClose()), widget_timelineScreenshotSettingsArea, SLOT(updatePresets()));
    settingsWindow.exec();
}

void AminoVisApp::stopThreads() {
    m_extractSurfaceTimer->stop();
    extractSurfaceLayersPushButton->setText("Start");
    for (ExtractSurfaceThread *th: m_threads) {
        th->deleteLater();
        th->requestInterruption();
        th->wait();
    }
    m_threads.clear();
    emit updateGlLayers();
    actionImport_Layer_Data->setEnabled(true);
    actionExport_Layer_Data->setEnabled(true);
    extractSurfaceLayersLabel->setText("All threads have been stopped!");
    emit updateHeatMap();
}


