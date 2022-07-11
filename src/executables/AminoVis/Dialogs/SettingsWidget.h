/**
 * @file   		SettingsWidget.h
 * @author 		Vladimir Ageev (vladimir.agueev@progsys.de)
 * @date   		19.05.2017
 *
 * @brief  		The settings window dialog.
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


#ifndef EXECUTABLES_AMINOVIS_SETTINGSWIDGET_H_
#define EXECUTABLES_AMINOVIS_SETTINGSWIDGET_H_

#include <ui_settingsWindow.h>
#include <QWidget>
#include <QDialog>
#include <QSettings>
#include <ColorLibrary.h>

class SettingsWidget: public QDialog, public Ui::SettingsForm {
	Q_OBJECT
public:
	SettingsWidget(QSettings& settigns, ColorLibrary* colorLib, bool colorSettingsTop = false, QWidget *parent = nullptr);
	virtual ~SettingsWidget();

	void closeEvent(QCloseEvent *event) override;
public slots:
	void save();
	void cancel();
signals:
	void onClose();
private:
	void setUpColorTable(QTableView* table, QToolButton* addbtn, ColorsRow* model);
	void updateColorSettings();
	QSettings& m_settigns;
	ColorLibrary* m_colors = nullptr;

};

#endif /* EXECUTABLES_AMINOVIS_SETTINGSWIDGET_H_ */
