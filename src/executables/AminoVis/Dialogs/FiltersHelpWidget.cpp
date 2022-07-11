/*
 * FiltersHelpWidget.cpp
 *
 *  Created on: 19.08.2017
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


#include <Dialogs/FiltersHelpWidget.h>
#include <Util/ResourcePath.h>
#include <QUrl>

FiltersHelpWidget::FiltersHelpWidget(FilterAtomsListModel* filter, QWidget *parent): QDialog(parent) {
	setupUi(this);

	int i = 0;
	for(const Atom::FilterVariable* v: filter->getRegistredVariables()){
		variablesTableWidget->insertRow(i);
		variablesTableWidget->setItem(i,0,new QTableWidgetItem(v->display()+"\n["+v->getArguments().join(", ")+"]" ));
		variablesTableWidget->setItem(i,1,new QTableWidgetItem(v->getDiscription()));
		variablesTableWidget->setRowHeight(i, 40);
		i++;
	}
	variablesTableWidget->resizeColumnsToContents();
	//variablesTableWidget->resizeRowsToContents();

	i = 0;
	for(const Atom::FilterFunction* f: filter->getRegistredFunctions()){
		functionsTableWidget->insertRow(i);
		functionsTableWidget->setItem(i,0,new QTableWidgetItem(f->name()+"\n["+f->getArguments().join(", ")+"]"));
		functionsTableWidget->setItem(i,1,new QTableWidgetItem(f->getDiscription()));
		functionsTableWidget->setRowHeight(i, 40);
		i++;
	}
	functionsTableWidget->resizeColumnsToContents();
	//functionsTableWidget->resizeRowsToContents();
	//textBrowser->document()->setMetaInformation( QTextDocument::DocumentUrl, resourcePath() + "/textrues/" );
	//textBrowser->document()->setMetaInformation( QTextDocument::ImageResource, resourcePath() + "/" );
	textBrowser->setSearchPaths({resourcePath(), resourcePath() + "/textures/"});
	textBrowser->setSource( QUrl::fromLocalFile(resourcePath()+"/data/filters.html"));
	//textBrowser->setHtml(resourcePath()+"/data/filters.html");
	/*
	textBrowser->setText("Filters are a powerful tool to manipulate the data you see, to hide certain items or change it's appearance."
			"This is done by submitting C++ inspired expressions, for example 'AtomElement == H' (Atom element is equal H) will hide all hydrogen (H) atoms form the timeline and render views."
			"<br>There are two types of expressions that have an effect on the data, distinct by there return type: <ul>"
			"<li>Boolean - Expressions that return true or false. They will hide a item or items form the data. (Example: 'ResidueName == CYS')</li>"
			"<li>Numerical - Expressions that return a float or integer. They will manipulate the heatmap values. (Example: '1' or 'ResidueLayer(Tracker(0))')</li>"
			"</ul> "
			"Expressions"
			"test <br> rer  <img src='"+ resourcePath()+"/textures/test3.jpg' alt='Smiley face' height='42' width='42'> ");*/
}

FiltersHelpWidget::~FiltersHelpWidget() {
	// TODO Auto-generated destructor stub
}

