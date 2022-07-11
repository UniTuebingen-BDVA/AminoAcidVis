/*
 * ColorLibrary.cpp
 *
 *  Created on: 13.06.2017
 *      Author: Vladimir Ageev
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

#include <ColorLibrary.h>
#include <QtMath>
#include <QMetaType>
#include <QDebug>
#include <QBrush>
#include <QMimeData>

rawcolor operator*(rawcolor c, float f){
	return {(unsigned char)(c.bgra.b*f), (unsigned char)(c.bgra.g*f), (unsigned char)(c.bgra.r*f), (unsigned char)(c.bgra.a*f)};
}

rawcolor operator+(rawcolor a, rawcolor b){
	return {(unsigned char)(a.bgra.b+b.bgra.b),(unsigned char)(a.bgra.g+b.bgra.g), (unsigned char)(a.bgra.r+b.bgra.r), (unsigned char)(a.bgra.a+b.bgra.a)};
}

rawcolor interpolate(rawcolor a, rawcolor b, float val){
	return a * (1-val) + b * val;
}


QDataStream& operator<< (QDataStream& s, const QVector<rawcolor>& data){
	s<<data.size();
	s.writeRawData((char*)data.data(),data.size()*sizeof(rawcolor));
	return s;
}

QDataStream& operator>> (QDataStream& s, QVector<rawcolor>& data){
	int size;
	s>>size;
	data.resize(size);
	s.readRawData((char*)data.data(),data.size()*sizeof(rawcolor));
	return s;
}

QDataStream& operator<< (QDataStream& s, const rawcolor& data){
	s<<data.data;
	return s;
}
QDataStream& operator>> (QDataStream& s, rawcolor& data){
	s>>data.data;
	return s;
}

///////////////////

ColorsRow::ColorsRow(QObject *parent): QAbstractTableModel(parent){}
ColorsRow::ColorsRow(const QVector<rawcolor> &v, QObject *parent): QAbstractTableModel(parent), m_colors(v){}
ColorsRow::~ColorsRow(){}


void ColorsRow::operator=(const ColorsRow& colors){
	beginResetModel();
	m_colors = colors.m_colors;
	endResetModel();
	emit colorChanged();
}
void ColorsRow::operator=(const QVector<rawcolor> &v){
	beginResetModel();
	m_colors = v;
	endResetModel();
	emit colorChanged();
}

QColor ColorsRow::getColor(int index) const{
	const rawcolor& c = m_colors[index];
	return {c.bgra.r,c.bgra.g,c.bgra.b,255};
}

QColor ColorsRow::getInterpolatedColor(float val) const{
	const rawcolor c = getInterpolatedRawColor(val);
	return {c.bgra.r,c.bgra.g,c.bgra.b,255};
}
rawcolor ColorsRow::getInterpolatedRawColor(float val) const{
	if(val <= 0) return front();
	if(val >= size()) return last();
	const int i = qFloor(val);
	const rawcolor& a = m_colors[i];
	const rawcolor& b = m_colors[i+1];
	val -= (float)i;
	return interpolate(a,b, val);
}

void ColorsRow::setColor(int index, const QColor& colorIn){
	rawcolor& c = m_colors[index];
	c.bgra.r = colorIn.red();
	c.bgra.g = colorIn.green();
	c.bgra.b = colorIn.blue();
	emit colorChanged();
	QModelIndex changeAt = createIndex(0,index);
	emit dataChanged(changeAt, changeAt);
}

void ColorsRow::setColor(int index, const rawcolor& colorIn){
	m_colors[index] = colorIn;
	emit colorChanged();
	QModelIndex changeAt = createIndex(0,index);
	emit dataChanged(changeAt, changeAt);
}


void ColorsRow::push_back(const QColor& colorIn){
	beginInsertColumns(QModelIndex(), size(), size() );
	m_colors.push_back({(unsigned char)colorIn.blue(),(unsigned char)colorIn.green(),(unsigned char)colorIn.red(),0});
	endInsertColumns();
	emit colorChanged();
	emit dataChanged(createIndex(0,size()-2), createIndex(0,size()));
}

void ColorsRow::moveLeft(int index){
	Q_ASSERT(index > 0);
	rawcolor buffer = m_colors[index-1];
	m_colors[index-1] = m_colors[index];
	m_colors[index] = buffer;
	emit colorChanged();
	emit dataChanged(createIndex(0,index-1), createIndex(0,index));
}

void ColorsRow::moveRight(int index){
	Q_ASSERT(index < size());
	rawcolor buffer = m_colors[index+1];
	m_colors[index+1] = m_colors[index];
	m_colors[index] = buffer;
	emit colorChanged();
	emit dataChanged(createIndex(0,index), createIndex(0,index+1));
}

void ColorsRow::remove(int index){
	beginRemoveColumns(QModelIndex(), index,index);
	m_colors.remove(index);
	endRemoveColumns();
	emit colorChanged();
	emit dataChanged(createIndex(0,index), createIndex(0,index));
}

QVariant ColorsRow::data(const QModelIndex &index, int role) const{
    if (!index.isValid()) return QVariant();
     switch(role){
     	 case Qt::DisplayRole:
     	 case Qt::EditRole:
             return QString::number(index.column());
         break;
     	 case Qt::BackgroundRole:{
				//QBrush background(Qt::red);//getColor(index.column()));
				QBrush background(getColor(index.column()));
				return background;
			}
     	 break;
     	case Qt::TextAlignmentRole:
     		return  Qt::AlignCenter;
     	break;
     	 default:
     		return QVariant();
     	break;

     }


}

QVariant ColorsRow::headerData(int section, Qt::Orientation orientation,int role) const{
	if (role == Qt::SizeHintRole)
		return QSize(10, 40);
	return QAbstractTableModel::headerData(section,orientation,role);
}

QMimeData* ColorsRow::mimeData(const QModelIndexList &indexes) const{
    QMimeData *mimeData = new QMimeData();
    QByteArray encodedData;

    QDataStream stream(&encodedData, QIODevice::WriteOnly);
    //this is kinda hacked
    for (const QModelIndex &index: indexes) {
        if (index.isValid()) {
        	stream <<index.column();
        	stream << m_colors[index.column()];
        }
    }

    mimeData->setData("SwapBGRA8888", encodedData);
    return mimeData;
}

bool ColorsRow::canDropMimeData(const QMimeData *data, Qt::DropAction action, int , int , const QModelIndex&) const{
	if (action == Qt::IgnoreAction) return true;
	//if (!parent.isValid() || action != Qt::MoveAction || row != 0 || column < 0 || !data->hasFormat("BGRA8888")) return false;
	if (!data->hasFormat("SwapBGRA8888")) return false;
	return true;
}

bool ColorsRow::dropMimeData(const QMimeData *data, Qt::DropAction action, int, int column, const QModelIndex &parent){
	if (action == Qt::IgnoreAction) return true;
    if (action != Qt::CopyAction || !data->hasFormat("SwapBGRA8888")) return false;

    int beginColumn = (column < 0)? (parent.isValid())? parent.column() : columnCount(QModelIndex()) : column;
	//encoding data
	QByteArray encodedData = data->data("SwapBGRA8888");
	QDataStream stream(&encodedData, QIODevice::ReadOnly);
	//beginInsertColumns(QModelIndex(), beginColumn, beginColumn );
	while (!stream.atEnd()) {
		int swapTarget;
		stream >> swapTarget;
		rawcolor c;
		stream >> c;
		if(swapTarget >= 0 && swapTarget < size() && swapTarget != beginColumn){
			rawcolor buffer = m_colors[beginColumn];
			m_colors[beginColumn] = c;
			m_colors[swapTarget] = buffer;
		}
	}
	//endInsertColumns( );
	emit colorChanged();
	return true;
}

bool ColorsRow::insertColumns(int column, int count, const QModelIndex &parent){
	if(!parent.isValid() || column < 0 || column > size() || count < 1) return false;
	beginInsertRows(parent, column, column+count);
	m_colors.insert(column,count, {0,0,0,0});
	endInsertRows();
	emit colorChanged();
	return true;
}

bool ColorsRow::removeColumns(int column, int count, const QModelIndex &parent) {
	if(!parent.isValid() || column < 0 || column > size() || count > 1) return false;
	beginRemoveRows(parent, column, column+count);
	m_colors.remove(column,count);
	endRemoveRows();
	emit colorChanged();
	return true;
}

Qt::ItemFlags ColorsRow::flags(const QModelIndex &index) const{
    if (!index.isValid()) return 0;
    return QAbstractItemModel::flags(index) | Qt::ItemNeverHasChildren | Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled;;
}


int ColorsRow::rowCount(const QModelIndex&) const{
	return !empty();
}

int ColorsRow::columnCount(const QModelIndex&) const{
	return size();
}


////////////////////////
ColorLibrary::ColorLibrary(const QString& file, QObject* parent): QObject(parent),m_colorSettings(file, QSettings::IniFormat, this) {
	qRegisterMetaTypeStreamOperators<QVector<rawcolor>>("QVector<rawcolor>");
	connect(&p_layersColors, SIGNAL(colorChanged()), this, SIGNAL(colorsChanged()) );
	connect(&p_glLayersColors, SIGNAL(colorChanged()), this, SIGNAL(colorsChanged()) );
	load();

}
ColorLibrary::~ColorLibrary() {}

const QString& ColorLibrary::getPreset() const{
	return m_preset;
}

void ColorLibrary::removePreset(const QString& name){
	m_colorSettings.remove(name);
}

void ColorLibrary::setPreset(const QString& preset){
	if(preset.isEmpty() || m_preset == preset) return;
	if(m_colorSettings.childGroups().contains(preset)){
		load(preset);
	}else{
		save(preset);
		m_preset = preset;
	}
	m_colorSettings.setValue("Default/Preset", m_preset);
	emit colorsChanged();
}

bool ColorLibrary::getOverwriteGLLayerColors() const{
	return m_overwriteGLLayerColors;
}

void ColorLibrary::setOverwriteGLLayerColors(bool enable){
	if(m_overwriteGLLayerColors == enable) return;
	m_overwriteGLLayerColors = enable;
	emit colorsChanged();
}

const ColorsRow& ColorLibrary::getLayerColors(bool heatmap) const{
	if(heatmap){
		return p_layersColors;
	}else
		return (m_overwriteGLLayerColors)? p_glLayersColors: p_layersColors;
}

void ColorLibrary::setOverwriteGLBackground(bool enable){
	if(m_overwriteGLBackground == enable) return;
	m_overwriteGLBackground = enable;
	emit colorsChanged();
}

bool ColorLibrary::getOverwriteGLBackground() const{
	return m_overwriteGLBackground;
}

void ColorLibrary::setGLBackgroundColor(const QColor& color){
	if(m_glBackground == color) return;
	m_glBackground = color;
	if(m_overwriteGLBackground) emit colorsChanged();
}

const QColor& ColorLibrary::getGLBackgroundColor() const{
	return (m_overwriteGLBackground)? m_glBackground: getWindow();
}

void ColorLibrary::load(){
	const QString name = m_colorSettings.value( "Default/Preset", "Default").toString();
	load(getPresets().contains(name)?name:"Default");
}

void ColorLibrary::load(const QString& preset){
	m_preset = preset;

	QString name = QString("%1/Layers").arg(m_preset) ;
	if(m_colorSettings.contains(name)){
		p_layersColors = m_colorSettings.value(name).value<QVector<rawcolor>>();
	}else
		p_layersColors = {{34,34,0,0},{166,75,0,0},{255,84,0,0}};

	name = QString("%1/GLLayers").arg(m_preset) ;
	if(m_colorSettings.contains( name ) ){
		p_glLayersColors = m_colorSettings.value(name).value<QVector<rawcolor>>();
	}else
		p_glLayersColors = {{34,34,0,0},{166,75,0,0},{255,84,0,0}};

	m_overwriteGLLayerColors = m_colorSettings.value( QString("%1/UseGlLayers").arg(m_preset) ).toBool();
	m_overwriteGLBackground = m_colorSettings.value( QString("%1/UseGlBackground").arg(m_preset) ).toBool();
	m_glBackground = m_colorSettings.value( QString("%1/GlBackground").arg(m_preset) ).value<QColor>();
}


void ColorLibrary::save(){
	save(m_preset);
}

void ColorLibrary::save(const QString& preset){
	m_colorSettings.setValue("Default/Preset", preset);
	m_colorSettings.setValue(QString("%1/Layers").arg(preset), QVariant::fromValue(p_layersColors.vec()));
	m_colorSettings.setValue(QString("%1/GLLayers").arg(preset), QVariant::fromValue(p_glLayersColors.vec()));
	m_colorSettings.setValue(QString("%1/UseGlLayers").arg(preset), m_overwriteGLLayerColors);
	m_colorSettings.setValue(QString("%1/UseGlBackground").arg(preset), m_overwriteGLBackground);
	m_colorSettings.setValue(QString("%1/GlBackground").arg(preset), m_glBackground);

}

QPalette& ColorLibrary::palette(){
	return m_palette;
}
const QPalette& ColorLibrary::palette() const{
	return m_palette;
}

void ColorLibrary::setPalette(const QPalette& p){
	m_palette = p;
}

const QColor& ColorLibrary::getAlternateBase() const{
	return palette().alternateBase().color();
}

const QColor& ColorLibrary::getBase() const{
	return palette().base().color();
}

const QColor& ColorLibrary::getHighlight() const{
	return palette().highlight().color();
}

const QColor& ColorLibrary::getLight() const{
	return palette().light().color();
}

const QColor& ColorLibrary::getMid() const{
	return palette().mid().color();
}

const QColor& ColorLibrary::getText() const{
	return palette().text().color();
}

const QColor& ColorLibrary::getWindow() const{
	return palette().window().color();
}

const QColor ColorLibrary::getWindowTransparent() const{
	return QColor(getWindow().red(), getWindow().green(), getWindow().blue(), 128);
}

const QColor& ColorLibrary::getWindowText() const{
	return palette().windowText().color();
}

