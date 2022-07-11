/*
 * Filters.h
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


#ifndef LIBRARIES_ATOMS_FILTERLIST_H_
#define LIBRARIES_ATOMS_FILTERLIST_H_

#include <QObject>
#include <QString>
#include <QVariant>
#include <QAbstractItemModel>
#include <Atoms/Atoms.h>
#include <Atoms/Timeline.h>
#include <Atoms/FilterNode.h>

namespace Atom {

/*!
 * @ingroup Filter
 * @brief Filter item containing the filter tree root node and some settings.
 */
struct FilterItem{
	FilterItem(FilterNode* aNode, bool isRenderViewEnabled = true, bool isTimelineViewEnabled = true):
			node(aNode), renderViewEnabled(isRenderViewEnabled), timelineViewEnabled(isTimelineViewEnabled), isLive(aNode->neededConnects() & FILTER_NODE_IS_LIVE){}
	FilterNode* node; /// Root filter tree node.
	bool renderViewEnabled; /// Should the filter be applied to the render views.
	bool timelineViewEnabled; /// Should the filter be applied to the timeline view.
	bool isLive;

	FilterNode* operator->(){ return node;}
	const FilterNode* operator->() const{ return node;}
};

/*!
 * @ingroup Filter
 * @brief Filter list model
 */
class FilterList: public QAbstractItemModel, public QList<FilterItem>{
	Q_OBJECT
public:
	/// Custom roles for QML data access
	enum CustomRoles {
		RenderViewEnabledRole = Qt::UserRole + 4,
		TimelineViewEnabledRole = Qt::UserRole + 5,
	};

	FilterList(QObject* parent = nullptr);
	virtual ~FilterList();

	virtual void clear();
	virtual void push_back(FilterNode* item) final;
	virtual void removeAt(int i) final;

	int getRefreshTriggers() const;


	//List model overwrite
    QVariant data(const QModelIndex &index, int role) const Q_DECL_OVERRIDE;
    Qt::ItemFlags flags(const QModelIndex &index) const Q_DECL_OVERRIDE;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const Q_DECL_OVERRIDE;
    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const Q_DECL_OVERRIDE;
    QModelIndex parent(const QModelIndex &index) const Q_DECL_OVERRIDE;
    bool removeRows(int row, int count, const QModelIndex &parent = QModelIndex());
    int rowCount(const QModelIndex &parent = QModelIndex()) const Q_DECL_OVERRIDE;
    int columnCount(const QModelIndex &parent = QModelIndex()) const Q_DECL_OVERRIDE;
    QHash<int, QByteArray> roleNames() const Q_DECL_OVERRIDE;
};

} /* namespace Atoms */

#endif /* LIBRARIES_ATOMS_FILTERLIST_H_ */
