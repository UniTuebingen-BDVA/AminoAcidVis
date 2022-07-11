/*
 * Filters.cpp
 *
 *  Created on: 10.08.2017
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

#include <FilterList.h>
#include <QMap>
#include <QSharedPointer>
namespace Atom {


FilterList::FilterList(QObject* parent ): QAbstractItemModel(parent){}
FilterList::~FilterList(){
	for(FilterItem& f: *this)
		delete f.node;
}

void FilterList::clear(){
	beginRemoveRows(QModelIndex(), 0, size());
	QList<FilterItem>::clear();
	endRemoveRows();

}

void FilterList::push_back(FilterNode* item){
	if(item){
		beginInsertRows(QModelIndex(), size(), size());
		QList<FilterItem>::push_back(FilterItem(item));
		endInsertRows();
	}
}

void FilterList::removeAt(int i){
	beginRemoveRows(QModelIndex(), i, i);
	FilterItem item = at(i);
	QList<FilterItem>::removeAt(i);
	delete item.node;
	endRemoveRows();
}

int FilterList::getRefreshTriggers() const{
	int triggers = 0;
	for(const FilterItem& f: *this)
		triggers |= f->neededConnects();

	return triggers;
}

//List model overwrite
QVariant FilterList::data(const QModelIndex &index, int role) const{
    if (!index.isValid())
        return QVariant();

    switch (role) {
		case Qt::DisplayRole: return QVariant::fromValue(static_cast<FilterItem*>(index.internalPointer())->node->display());break;
		case RenderViewEnabledRole: return QVariant::fromValue(static_cast<FilterItem*>(index.internalPointer())->renderViewEnabled);break;
		case TimelineViewEnabledRole: return QVariant::fromValue(static_cast<FilterItem*>(index.internalPointer())->timelineViewEnabled);break;
		//case Qt::DecorationRole: return QVariant::fromValue(static_cast<Filter*>(index.internalPointer())->name());break;
	}

    return QVariant();
}
Qt::ItemFlags FilterList::flags(const QModelIndex &index) const{
    if (!index.isValid())
        return 0;

    return QAbstractItemModel::flags(index);
}
QVariant FilterList::headerData(int , Qt::Orientation , int ) const{
	return QVariant();
}
QModelIndex FilterList::index(int row, int column, const QModelIndex &parent) const{
	if(parent.isValid()) return QModelIndex();
	const FilterItem& item = at(row) ;
	return createIndex(row, column, (void*)&item );
}
QModelIndex FilterList::parent(const QModelIndex &) const{
	return QModelIndex();
}
bool FilterList::removeRows(int row, int count, const QModelIndex &parent){
	if(parent.isValid() || count <= 0)
		return false;
	beginRemoveRows(QModelIndex(), row, row+count);
	auto itbegin = begin()+row;
	auto itend = begin()+row+count;
	for(auto it = itbegin; it < itend; it++)
		delete it->node;
	QList<FilterItem>::erase(itbegin, itend);
	endRemoveRows();
	return true;
}
int FilterList::rowCount(const QModelIndex &parent) const{
	if(parent.isValid()) return 0;
	return size();
}
int FilterList::columnCount(const QModelIndex &) const{
	return 1;
}
QHash<int, QByteArray> FilterList::roleNames() const{
    QHash<int, QByteArray> roles = QAbstractItemModel::roleNames();
    roles[RenderViewEnabledRole] = "renderViewEnabled";
    roles[TimelineViewEnabledRole] = "timelineViewEnabled";
    return roles;
}


} /* namespace Atoms */
