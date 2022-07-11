/*
 * SettingsWidget.cpp
 *
 *  Created on: 19.05.2017
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

#include <Dialogs/SettingsWidget.h>
#include <ColorLibrary.h>
#include <QColorDialog>
#include <stdexcept>
#include <QMenu>
#include <QAction>

#include <QStyledItemDelegate>
#include <QPainter>
#include <QBrush>
#include <QPen>
#include <QInputDialog>
#include <QMessageBox>
#include <Util/ResourcePath.h>

#include <Rendering/OpenGL.h>
#include <QThread>
/*!
 * @brief Costom paint delegate, so the selection color dosn't overwrite then table background color.
 * @see http://www.qtforum.org/article/34125/disable-qtablewidget-selection-color.html
 */
class BackgroundDelegate : public QStyledItemDelegate
{
public:
    explicit BackgroundDelegate(QObject *parent = 0): QStyledItemDelegate(parent){}
    void paint(QPainter *painter, const QStyleOptionViewItem &option,const QModelIndex &index) const{
    	//if selected then we just paint the background color
    	if(option.state & QStyle::State_Selected){
            QVariant background = index.data(Qt::BackgroundRole);
            if (background.canConvert<QBrush>())
                painter->fillRect(option.rect, background.value<QBrush>());
    	}else QStyledItemDelegate::paint(painter, option, index);
    }
};

void SettingsWidget::setUpColorTable(QTableView* table, QToolButton* addbtn, ColorsRow* model){

	table->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
	table->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
	table->setModel(model);
	table->setItemDelegate(new BackgroundDelegate(this));

	//custom context menu
	table->setContextMenuPolicy(Qt::CustomContextMenu);
	connect(table, &QTableView::customContextMenuRequested, this, [this,model,table](const QPoint& p){
		QModelIndex index=table->indexAt(p);
		if(index.isValid() && index.column() < model->size()){
			QMenu *menu=new QMenu(this);
			//Edit
			QAction* act = new QAction("Edit", this);
			connect(act, &QAction::triggered, this, [this,model,index](){
				QColor c = QColorDialog::getColor(model->getColor(index.column()), this, "Edit color "+QString::number(index.column()));
				if(c.isValid()) model->setColor(index.column(), c);
			});
			menu->addAction(act);
			//Move left
			if(index.column() > 0){
				act = new QAction(QIcon(resourcePath() + "/textures/previous.png"),"Move left", this);
				connect(act, &QAction::triggered, this, [this,index,model](){
					model->moveLeft(index.column());
				});
				menu->addAction(act);
			}
			//Move right
			if(index.column() < model->size()-1){
				act = new QAction(QIcon(resourcePath() + "/textures/next.png"), "Move right", this);
				connect(act, &QAction::triggered, this, [this,model, index](){
					model->moveRight(index.column());
				});
				menu->addAction(act);
			}
			menu->addSeparator();
			//Remove
			act = new QAction("Remove", this);
			connect(act, &QAction::triggered, this, [this,index,model](){
				model->remove(index.column());
			});
			menu->addAction(act);

			menu->popup(table->mapToGlobal(p));
		}
	});

	//edit color with double click
	connect(table, &QTableView::doubleClicked , this, [this,model](const QModelIndex & index){
		QColor c = QColorDialog::getColor(model->getColor(index.column()), this, "Edit color "+QString::number(index.column()));
		if(c.isValid()) model->setColor(index.column(), c);
	});

	//add new color +
	connect(addbtn, &QToolButton::clicked, this, [this,model](){
		QColor c = QColorDialog::getColor(QColor(255,255,255,255), this, "Add new color");
		if(c.isValid()) model->push_back(c);
	});
}

SettingsWidget::SettingsWidget(QSettings& settigns, ColorLibrary* colorLib, bool colorSettingsTop, QWidget *parent): QDialog(parent), m_settigns(settigns), m_colors(colorLib) {
	setupUi(this);
	if(!m_colors){
		throw std::invalid_argument("[SettingsWidget] 'colorLib' is nullptr!");
	}
	m_colors->load();
	// ======== Surface extraction ========
	{
	const int idealThreadCount = QThread::idealThreadCount();
	int maxThreads = m_settigns.value("SurfaceExtraction/MaxThreads", 8).toInt();
	if(idealThreadCount > maxThreads){
		maxThreads = idealThreadCount;
		m_settigns.setValue("SurfaceExtraction/MaxThreads", QVariant::fromValue(maxThreads));
	}
	threadsHorizontalSlider->setMaximum(maxThreads);
	threadsSpinBox->setMaximum(maxThreads);

	connect(threadsHorizontalSlider, SIGNAL(valueChanged(int)), threadsSpinBox, SLOT(setValue(int)));
	connect(threadsSpinBox, SIGNAL(valueChanged(int)), threadsHorizontalSlider, SLOT(setValue(int)));

	threadsSpinBox->setValue(m_settigns.value("SurfaceExtraction/Threads", 4).toInt());
	//QThread::idealThreadCount()
	label_recomendedThreads->setText(QString::number(idealThreadCount));
	}
	// ======== Colors ========
	comboBox->clear();
	comboBox->addItems(m_colors->getPresets());
	comboBox->setCurrentText(m_colors->getPreset());
	connect(savePresetPushButton,  &QPushButton::clicked , this, [this](){
		bool ok;
		QString name = QInputDialog::getText(this, tr("Create new preset"),
											 tr("Preset name:"), QLineEdit::Normal,
											 "", &ok);
		if (ok && !name.isEmpty()){
			if(name.size() > 128){
				QMessageBox::warning(this, "Name too long", "Preset name is too long!", QMessageBox::Ok);
			}else if(comboBox->findText(name) != -1){
				QMessageBox::warning(this, "Invalid name", "Preset '"+name+"' already exists!", QMessageBox::Ok);
			}else{
				comboBox->addItem(name);
				comboBox->setCurrentText(name);
				m_colors->setPreset(name);
			}
		}
	});

	connect(comboBox, static_cast<void (QComboBox::*)(const QString&)>(&QComboBox::activated), this, [this](const QString& n){
		if(!n.isEmpty()){
			deletePresetToolButton->setEnabled(n != "Default");
			m_colors->setPreset(n);
			updateColorSettings();
		}
	});

	connect(deletePresetToolButton, &QPushButton::clicked, this, [this](){
		QString name = comboBox->currentText();
		if(name == "Default"){
			QMessageBox::warning(this, "Invalid operation", "You can't delete the 'Default' preset!", QMessageBox::Ok);
			return;
		}
		if(QMessageBox::question(this, "Delete?", "Do you want to delete the '"+name+"' preset?", QMessageBox::Yes| QMessageBox::No) == QMessageBox::Yes){
			int i = comboBox->currentIndex();
			comboBox->removeItem(i);
			m_colors->removePreset(name);

			if(i > 0) i--;
			m_colors->setPreset(m_colors->getPresets()[i]);
			updateColorSettings();
		}

	});


	setUpColorTable(layersColorsTableView, addLayerColorToolButton, &m_colors->p_layersColors);
	setUpColorTable(layersColorsGlTableView, addLayerColorGlToolButton, &m_colors->p_glLayersColors);


	connect(glLayerColorsGroupBox, SIGNAL(clicked(bool)), m_colors, SLOT(setOverwriteGLLayerColors(bool)));
	connect(glLayerColorsGroupBox, SIGNAL(clicked(bool)), layersColorsGlTableView, SLOT(setEnabled(bool)));
	connect(glLayerColorsGroupBox, SIGNAL(clicked(bool)), addLayerColorGlToolButton, SLOT(setEnabled(bool)));

	glLayerColorsGroupBox->setChecked(m_colors->getOverwriteGLLayerColors());
	layersColorsGlTableView->setEnabled(m_colors->getOverwriteGLLayerColors());
	addLayerColorGlToolButton->setEnabled(m_colors->getOverwriteGLLayerColors());


	connect(glBackgroundGroupBox, SIGNAL(clicked(bool)), glBackgroundPushButton, SLOT(setEnabled(bool)));
	connect(glBackgroundGroupBox, SIGNAL(clicked(bool)), m_colors, SLOT(setOverwriteGLBackground(bool)));
	glBackgroundGroupBox->setChecked(m_colors->getOverwriteGLBackground());
	glBackgroundPushButton->setEnabled(m_colors->getOverwriteGLBackground());

	connect(glBackgroundGroupBox, &QGroupBox::clicked, this, [this](bool on){
		if(on) glBackgroundPushButton->setStyleSheet(QString("background-color: %1;").arg(m_colors->getGLBackgroundColor().name()));
	});

	glBackgroundPushButton->setStyleSheet(QString("background-color: %1;").arg(m_colors->getGLBackgroundColor().name()));
	connect(glBackgroundPushButton, &QPushButton::clicked , this, [this](){
			QColor c = QColorDialog::getColor(m_colors->getGLBackgroundColor(), this, "Edit color");
			if(c.isValid()){
				glBackgroundPushButton->setStyleSheet(QString("background-color: %1;").arg(c.name()));
				m_colors->setGLBackgroundColor(c);
			}
		});

	// ======== Info ========
	infoTextBrowser->setText(
			QString("<html><b>Paths:</b> <br>")+
			"Config path: "+configPath()+"<br>"+
			"Resource path: "+resourcePath()+"<br>"+
			"<b>Render Info:</b> <br>"+
			"OpenGL version: "+QString(reinterpret_cast<const char*>(glGetString(GL_VERSION)))+"<br>"
			"GLSL version: "+QString(reinterpret_cast<const char*>(glGetString(GL_SHADING_LANGUAGE_VERSION)))+"<br>"
			"GLEW version: "+QString(reinterpret_cast<const char*>(glewGetString(GLEW_VERSION)))+"<br>"
			"Vendor: "+QString(reinterpret_cast<const char*>(glGetString(GL_VENDOR)))+"<br>"
			"Renderer: "+QString(reinterpret_cast<const char*>(glGetString(GL_RENDERER)))+"</html>"
	);
	// ======== Exit ========
	connect(savePushButton, SIGNAL(clicked()), this, SLOT(save()));
	connect(cancelPushButton, SIGNAL(clicked()), this, SLOT(cancel()));

	if(colorSettingsTop) colorTabWidget->setCurrentWidget(tab_2);

}

SettingsWidget::~SettingsWidget() {
	// TODO Auto-generated destructor stub
}

void SettingsWidget::closeEvent(QCloseEvent *event){
	emit onClose();
	QDialog::closeEvent(event);
}


void SettingsWidget::updateColorSettings(){
	glLayerColorsGroupBox->setChecked(m_colors->getOverwriteGLLayerColors());
	layersColorsGlTableView->setEnabled(m_colors->getOverwriteGLLayerColors());
	addLayerColorGlToolButton->setEnabled(m_colors->getOverwriteGLLayerColors());
	glBackgroundGroupBox->setChecked(m_colors->getOverwriteGLBackground());
	glBackgroundPushButton->setEnabled(m_colors->getOverwriteGLBackground());
	glBackgroundPushButton->setStyleSheet(QString("background-color: %1;").arg(m_colors->getGLBackgroundColor().name()));
}

void SettingsWidget::cancel(){
	m_colors->load();
	m_colors->colorsChanged();
	close();
}

void SettingsWidget::save(){
	m_settigns.setValue("SurfaceExtraction/Threads", threadsSpinBox->value());
	m_colors->save();
	close();
}
