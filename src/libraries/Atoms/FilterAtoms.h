/**
 * AtomsListModel.h
 *
 *  Created on: 09.08.2017
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

#ifndef LIBRARIES_ATOMS_FILTERATOMS_H_
#define LIBRARIES_ATOMS_FILTERATOMS_H_

#include <QMap>
#include <QVariant>
#include <QVector>
#include <Atoms/Atoms.h>
#include <QSortFilterProxyModel>
#include <Atoms/Timeline.h>
#include <Atoms/FilterList.h>

/*!
 * @ingroup Filter
 * @brief Holds the result value of a filter item as QVariant.
 * @see Atom::FilterItem
 */
struct ResultFilterItem{
	ResultFilterItem(const Atom::FilterItem* aitem, QVariant aResult):item(aitem), result(aResult){}

	const Atom::FilterItem* item; /// The filter item itself
	QVariant result; /// The result of the filter item

	operator QVariant(){return result;}

	inline QVariant::Type type() const{return result.type();}
	inline bool toBool() const{return result.toBool();}
	inline int toInt() const{return result.toInt();}
	inline float toFloat() const{return result.toFloat();}
	inline double toDouble() const{return result.toDouble();}
	inline QString toString() const{return result.toString();}
};

/*!
 * @ingroup Filter
 * @brief The filter proxy for the atoms model which also manages the filters.
 *
 * All filters are not calculated on each call, instead they are precalculated and stored as a ResultFilterItem inside a map.
 * @see ResultFilterItem
 * @see Atom::FilterList
 * @see Atoms
 */
class FilterAtomsListModel : public QSortFilterProxyModel{
	Q_OBJECT
public:
	FilterAtomsListModel(Atoms* atomsListModel = nullptr, Timeline* timeline = nullptr, QObject *parent = 0);
	virtual ~FilterAtomsListModel();


	inline Atom::FilterList& getFilterList(){ return m_filters; };
	inline const Atom::FilterList& getFilterList() const{ return m_filters; };

	/*!
	 * @brief Contains the results of the filters for all residues that apply.
	 * @returns A QMap with the residue index as key and a QList with the results of the filters as value.
	 */
	inline const QMap<int,QList<ResultFilterItem>>& getFilterResidueResults() const{ return m_resultsResidue; };
	inline QMap<int,QList<ResultFilterItem>>& getFilterResidueResults(){ return m_resultsResidue; };
	/*!
	 * @brief Contains the results of the filters for all atoms that apply.
	 * @returns A QMap with the atom index as key and a QList with the results of the filters as value.
	 */
	inline const QMap<int,QList<ResultFilterItem>>& getFilterAtomResults() const{ return m_resultsAtom; };
	inline QMap<int,QList<ResultFilterItem>>& getFilterAtomResults(){ return m_resultsAtom; };

	inline const QVector<Atom::FilterVariable*>& getRegistredVariables() const { return m_registredVariables; };
	inline const QVector<Atom::FilterFunction*>& getRegistredFunctions() const { return m_registredFunctions; };

	inline bool isEnabled() const { return m_enabled;}

public slots:
	/*!
	 * @brief Refresh this filter proxy model.
	 * This will recalculate the results for the residue and atom filters.
	 * Most functions will do this automatically.
	 * @see getFilterResidueResults
	 * @see getFilterAtomResults
	 */
	Q_INVOKABLE void refresh();
	/*!
	 * @brief Removes all filters.
	 * This will refresh this filter proxy model.
	 */
	Q_INVOKABLE void clearFilters();
	/*!
	 * @brief Add new filter as a string.
	 * This will refresh this filter proxy model.
	 */
	Q_INVOKABLE void addFilters(const QString& arguments);
	/*!
	 * @brief Remove a filter.
	 * This will refresh this filter proxy model.
	 * @param index The index of the filter
	 */
	Q_INVOKABLE void removeFilter(int index);
	/*!
	 * @brief Enable or disable if a filter should be applied to the timeline view.
	 * This will refresh this filter proxy model.
	 * @param index The index of the filter that should be enabled or disabled.
	 * @param value If true filter will be disabled
	 */
	Q_INVOKABLE void disableTimeline(int index, bool value);
	/*!
	 * @brief Enable or disable if a filter should be applied to the render views.
	 * This will refresh this filter proxy model.
	 * @param index The index of the filter that should be enabled or disabled.
	 * @param value If true filter will be disabled
	 */
	Q_INVOKABLE void disableRenderView(int index, bool value);
	/*!
	 * @brief Copies a filters display value (which is equal to it's argument) to the clipboard.
	 * @param index The index of the filter
	 */
	Q_INVOKABLE void clipboard(int index);
	/*!
	 * @brief Sets if the filters should be applied.
	 * @param enable if true then filters will be applied
	 */
	Q_INVOKABLE void setEnabled(bool enable);

	Q_INVOKABLE int getRowFromSource(int sourceRow) const;
signals:
	void filterHasChanged();
	void disableTimelineChanged();
	void disableRenderViewChanged();
protected:
	bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const;
	void updateRefrechTriggers(int triggers);
	void calculateFilters();
	Atoms* m_data;
	Timeline* m_timeline;
	Atom::FilterList m_filters;

	bool m_enabled = true;

	QVector<Atom::FilterVariable*> m_registredVariables;
	QVector<Atom::FilterFunction*> m_registredFunctions;

	QMap<int,QList<ResultFilterItem>> m_resultsResidue;
	QMap<int,QList<ResultFilterItem>> m_resultsAtom;
};

#endif /* LIBRARIES_ATOMS_FILTERATOMS_H_ */
