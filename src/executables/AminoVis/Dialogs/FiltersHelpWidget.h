/**
 * @file   		FiltersHelpWidget.h
 * @author 		Vladimir Ageev (vladimir.agueev@progsys.de)
 * @date   		19.08.2017
 *
 * @brief  		A simple window containing instructions about the filters.
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

#ifndef EXECUTABLES_AMINOVIS_DIALOGS_FILTERSHELPWIDGET_H_
#define EXECUTABLES_AMINOVIS_DIALOGS_FILTERSHELPWIDGET_H_

#include <ui_filterHelpWindow.h>
#include <QWidget>
#include <QDialog>
#include <Atoms/FilterAtoms.h>

/*!
 * @brief Simple help dialog about the filters.
 * @see FilterAtomsListModel
 */
class FiltersHelpWidget: public QDialog, public Ui::FilterHelpForm {
	Q_OBJECT
public:
	FiltersHelpWidget(FilterAtomsListModel* filter, QWidget *parent = nullptr);
	virtual ~FiltersHelpWidget();
};

#endif /* EXECUTABLES_AMINOVIS_DIALOGS_FILTERSHELPWIDGET_H_ */
