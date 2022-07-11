/*
 * FilterNode.h
 *
 *  Created on: 11.08.2017
 *      Author: Vladimir Ageev
 *
 * @brief  		Contains the abstract base class of a filter node.
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

#ifndef LIBRARIES_ATOMS_FILTERNODE_H_
#define LIBRARIES_ATOMS_FILTERNODE_H_

#include <QObject>
#include <QString>
#include <QVariant>
#include <QSharedPointer>
#include <QMap>
#include <QList>
#include <QStringList>
#include <Atoms/Atoms.h>
#include <Atoms/Timeline.h>

#define FILTER_NODE_CONNECT_FRAME_CHANGE 0x00000001
#define FILTER_NODE_CONNECT_STARTFRAME_CHANGE 0x00000002
#define FILTER_NODE_CONNECT_ENDFRAME_CHANGE 0x00000004
#define FILTER_NODE_CONNECT_ATOM_SELECTED_CHANGE 0x00000008
#define FILTER_NODE_IS_LIVE 0x00000010

namespace Atom {

/*!
 * @defgroup Filter
 * @brief Filters are a powerful tool to manipulate the data you see, to hide certain items or change its appearance.
 */

class FilterNode;

typedef QSharedPointer<FilterNode> FilterSharedPtr;
typedef QMap<QString, QSharedPointer<FilterNode>> Variables;

/*!
 * @ingroup Filter
 * @brief Abstract base class for all filter definitions.
 */
class FilterNode{
public:
	FilterNode(){};
	virtual ~FilterNode(){}

	/*!
	 * @brief Searches the expression tree for invalid or undefined definitions.
	 * If false, then the expression shall not be used!
	 * @returns true if expression is valid and save to use.
	 */
	virtual bool isValid() const = 0;

	/*!
	 * @brief Should return the needed Qt connections that should trigger an update.
	 * If you have multiple then just 'Bitwise OR' them, for example 'FILTER_NODE_CONNECT_FRAME_CHANGE | FILTER_NODE_CONNECT_ENDFRAME_CHANGE'.
	 */
	virtual int neededConnects() const { return 0; };
	/*!
	 * @brief Calculates the result of the expression. May terminate if isValid() is false.
	 * @returns QVariant
	 * @see isValid()
	 */
	virtual QVariant call(bool isAtom, int index, const QString& name, QObject* root, Atoms* data, Timeline* timeline, Variables& vars) const = 0;
	/*!
	 * @brief Converts the expression to a human readable string.
	 */
	virtual QString display() const = 0;

	virtual const QStringList getArguments() const { return {}; }
	virtual const QString getDiscription() const { return "No description given."; }
};

class FilterVariable: public FilterNode{
public:
	FilterVariable(){};
	virtual ~FilterVariable(){}

	virtual FilterVariable* createVar() const = 0;
};

class FilterFunction: public FilterNode{
protected:
	QVector<FilterNode*> m_arguments;
public:
	FilterFunction(){};
	FilterFunction(const QVector<FilterNode*>& arguments): m_arguments(arguments){};
	virtual ~FilterFunction(){
		for(FilterNode* f: m_arguments) delete f;
	}
	bool isValid() const{
		for(FilterNode* f: m_arguments)
			if(!f || !f->isValid()) return false;
		return true;
	}

	virtual QString name() const { return "NoName"; }

	int neededConnects() const {
		int connections = 0;
		for(FilterNode* f: m_arguments)
			connections |= f->neededConnects();
		return connections;
	};

	virtual int minArguments() const { return 0; }
	virtual int maxArguments() const { return 0; }

	virtual FilterFunction* createFunc(const QVector<FilterNode*>&) const = 0;
};



} /* namespace Atom */

#endif /* LIBRARIES_ATOMS_FILTERNODE_H_ */
