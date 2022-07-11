/*
 * FilterDefinitions.h
 *
 *  Created on: 11.08.2017
 *      Author: Vladimir Ageev (vladimir.agueev@progsys.de
 *
 * @brief  		Contains a huge amount of different filters.
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

#ifndef LIBRARIES_ATOMS_FILTERDEFINITIONS_H_
#define LIBRARIES_ATOMS_FILTERDEFINITIONS_H_

#include <Atoms/FilterList.h>

namespace Atom {
//roots
/*!
 * @ingroup Filter
 */
class Invalid: public FilterNode{
public:
	Invalid(){}
	virtual ~Invalid(){}

	bool isValid() const final{return true;}
	QVariant call(bool, int, const QString&, QObject*, Atoms*, Timeline*, Variables& ) const final{
		return QVariant();
	}
	QString display() const final{ return "Invalid";}
};

/*!
 * @ingroup Filter
 */
class Bool: public FilterNode{
	bool m_value;
public:
	Bool(bool value): m_value(value){}
	virtual ~Bool(){}

	bool isValid() const final{return true;}
	QVariant call(bool, int, const QString&, QObject*, Atoms*, Timeline*, Variables& ) const final{
		return QVariant::fromValue<bool>(m_value);
	}
	QString display() const final{ return m_value ? "true" : "false";}
};
/*!
 * @ingroup Filter
 */
class Integer: public FilterNode{
	int m_value;
public:
	Integer(int value): m_value(value){}
	virtual ~Integer(){}

	bool isValid() const final{return true;}
	QVariant call(bool, int, const QString&, QObject*, Atoms*, Timeline*, Variables& ) const final{
		return QVariant::fromValue<int>(m_value);
	}
	QString display() const final{ return QString::number(m_value);}
};
/*!
 * @ingroup Filter
 */
class Double: public FilterNode{
	double m_value;
public:
	Double(double value):  m_value(value){}
	virtual ~Double(){}

	bool isValid() const final{return true;}
	QVariant call(bool, int, const QString&, QObject*, Atoms*, Timeline*, Variables& ) const final{
		return QVariant::fromValue<double>(m_value);
	}
	QString display() const final{ return QString::number(m_value)+"f";}
};
/*!
 * @ingroup Filter
 */
class Str: public FilterNode{
	QString m_value;
public:
	Str(QString value): m_value(value){}
	virtual ~Str(){}

	bool isValid() const final{return true;}
	QVariant call(bool isAtom, int index, const QString& name, QObject* root, Atoms* data, Timeline* timeline, Variables& vars) const final{
		auto it = vars.find(m_value);
		if(it != vars.end())
			return it.value()->call(isAtom, index, name, root, data, timeline, vars);
		return QVariant::fromValue<QString>(m_value);
	}
	QString display() const final{ return m_value;}
};

/*!
 * @ingroup Filter
 */
class Brakets: public FilterNode{
	FilterNode* m_node;
public:
	Brakets(FilterNode* node): m_node(node) {}
	virtual ~Brakets(){ if(m_node) delete m_node;}
	bool isValid() const final{ return m_node && m_node->isValid();}
	int neededConnects() const final { return m_node->neededConnects(); };
	QVariant call(bool isAtom, int index, const QString& name, QObject* root, Atoms* data, Timeline* timeline, Variables& vars) const final{
		return m_node->call(isAtom, index, name, root, data, timeline, vars);
	};
	QString display() const final{ return "("+m_node->display()+")";}
};
/*!
 * @ingroup Filter
 */
class Operator: public FilterNode{
protected:
	FilterNode* m_left;
	FilterNode* m_right;
public:
	Operator(FilterNode* left, FilterNode* right): m_left(left), m_right(right){}

	bool isValid() const{
		return m_left && m_right && m_left->isValid() && m_right->isValid();
	}
	int neededConnects() const final { return m_left->neededConnects() | m_right->neededConnects(); };

	virtual ~Operator(){
		if(m_left) delete m_left;
		if(m_right) delete m_right;
	}
};

//Operator
/*!
 * @ingroup Filter
 */
class RangeResidue: public Operator{
public:
	RangeResidue(FilterNode* left, FilterNode* right): Operator(left, right){}
	virtual ~RangeResidue(){}

	QVariant call(bool isAtom, int index, const QString& name, QObject* root, Atoms* data, Timeline* timeline, Variables& vars) const final{
		QVariant a = m_left->call(isAtom, index, name, root, data, timeline, vars);
		QVariant b = m_right->call(isAtom, index, name, root, data, timeline, vars);
		if(!a.isValid() || !b.isValid() || a.type() != QVariant::Int || b.type() != QVariant::Int) return QVariant();

		if(!isAtom) return QVariant::fromValue<bool>( a.toInt() <= index && index <= b.toInt());
		return QVariant();

	};
	QString display() const final{ return m_left->display()+"_"+m_right->display();}
};
/*!
 * @ingroup Filter
 */
class RangeAtom: public Operator{
public:
	RangeAtom(FilterNode* left, FilterNode* right): Operator(left, right){}
	virtual ~RangeAtom(){}

	QVariant call(bool isAtom, int index, const QString& name, QObject* root, Atoms* data, Timeline* timeline, Variables& vars) const final{
		QVariant a = m_left->call(isAtom, index, name, root, data, timeline, vars);
		QVariant b = m_right->call(isAtom, index, name, root, data, timeline, vars);
		if(!a.isValid() || !b.isValid() || a.type() != QVariant::Int || b.type() != QVariant::Int) return QVariant();

		if(isAtom) return QVariant::fromValue<bool>( a.toInt() <= index && index <= b.toInt());
		return QVariant();

	};
	QString display() const final{ return m_left->display()+"~"+m_right->display();}
};

#define COMPARE(x)  QVariant a = m_left->call(isAtom, index, name, root, data, timeline, vars);\
					QVariant b = m_right->call(isAtom, index, name, root, data, timeline, vars);\
					if(!a.isValid() || !b.isValid()) return QVariant();\
					if(a.type() == QVariant::Int && b.type() == QVariant::Int){\
						return QVariant::fromValue<bool>(a.toInt() x b.toInt() );\
					}else if(a.type() == QVariant::Int && b.type() == QVariant::Double){\
						return QVariant::fromValue<bool>( a.toInt() x b.toDouble());\
					}else if(a.type() == QVariant::Double && b.type() == QVariant::Int){\
						return QVariant::fromValue<bool>( a.toDouble() x b.toInt());\
					}else if(a.type() == QVariant::Double && b.type() == QVariant::Double){\
						return QVariant::fromValue<bool>( a.toDouble() x b.toDouble());\
					}else if(a.type() == QVariant::String && b.type() == QVariant::String){\
						return QVariant::fromValue<bool>( a.toString().toLower() x b.toString().toLower());\
					}\
					return QVariant();\
/*!
 * @ingroup Filter
 */
class Compare: public Operator{
public:
	Compare(FilterNode* left, FilterNode* right): Operator(left, right){}
	virtual ~Compare(){}
};
/*!
 * @ingroup Filter
 */
class Equal: public Compare{
public:
	Equal(FilterNode* left, FilterNode* right): Compare(left, right){}
	virtual ~Equal(){}
	QVariant call(bool isAtom, int index, const QString& name, QObject* root, Atoms* data, Timeline* timeline, Variables& vars) const final{ COMPARE(==) };
	QString display() const final{ return m_left->display()+" == "+m_right->display();}
};
/*!
 * @ingroup Filter
 */
class NotEqual: public Compare{
public:
	NotEqual(FilterNode* left, FilterNode* right): Compare(left, right){}
	virtual ~NotEqual(){}
	QVariant call(bool isAtom, int index, const QString& name, QObject* root, Atoms* data, Timeline* timeline, Variables& vars) const final{ COMPARE(!=) };
	QString display() const final{ return m_left->display()+" != "+m_right->display();}
};
/*!
 * @ingroup Filter
 */
class Bigger: public Compare{
public:
	Bigger(FilterNode* left, FilterNode* right): Compare(left, right){}
	virtual ~Bigger(){}
	QVariant call(bool isAtom, int index, const QString& name, QObject* root, Atoms* data, Timeline* timeline, Variables& vars) const final{ COMPARE(>) };
	QString display() const final{ return m_left->display()+" > "+m_right->display();}
};
/*!
 * @ingroup Filter
 */
class BiggerEqual: public Compare{
public:
	BiggerEqual(FilterNode* left, FilterNode* right): Compare(left, right){}
	virtual ~BiggerEqual(){}
	QVariant call(bool isAtom, int index, const QString& name, QObject* root, Atoms* data, Timeline* timeline, Variables& vars) const final{ COMPARE(>=) };
	QString display() const final{ return m_left->display()+" >= "+m_right->display();}
};
/*!
 * @ingroup Filter
 */
class Smaller: public Compare{
public:
	Smaller(FilterNode* left, FilterNode* right): Compare(left, right){}
	virtual ~Smaller(){}
	QVariant call(bool isAtom, int index, const QString& name, QObject* root, Atoms* data, Timeline* timeline, Variables& vars) const final{ COMPARE(<) };
	QString display() const final{ return m_left->display()+" < "+m_right->display();}
};
/*!
 * @ingroup Filter
 */
class SmallerEqual: public Compare{
public:
	SmallerEqual(FilterNode* left, FilterNode* right): Compare(left, right){}
	virtual ~SmallerEqual(){}
	QVariant call(bool isAtom, int index, const QString& name, QObject* root, Atoms* data, Timeline* timeline, Variables& vars) const final{ COMPARE(<=) };
	QString display() const final{ return m_left->display()+" <= "+m_right->display();}
};
/*!
 * @ingroup Filter
 */
class And: public Operator{
public:
	And(FilterNode* left, FilterNode* right): Operator(left, right){ }
	virtual ~And(){}

	QVariant call(bool isAtom, int index, const QString& name, QObject* root, Atoms* data, Timeline* timeline, Variables& vars) const final{
		QVariant a = m_left->call(isAtom, index, name, root, data, timeline, vars);
		QVariant b = m_right->call(isAtom, index, name, root, data, timeline, vars);
		if(!a.isValid() || !b.isValid() || a.type() != QVariant::Bool || b.type() != QVariant::Bool) return QVariant();

		return QVariant::fromValue<bool>(a.toBool() && b.toBool());
	}
	QString display() const final{ return m_left->display()+" & "+m_right->display();}
};
/*!
 * @ingroup Filter
 */
class Or: public Operator{
public:
	Or(FilterNode* left, FilterNode* right): Operator(left, right){ }
	virtual ~Or(){}

	QVariant call(bool isAtom, int index, const QString& name, QObject* root, Atoms* data, Timeline* timeline, Variables& vars) const final{
		QVariant a = m_left->call(isAtom, index, name, root, data, timeline, vars);
		QVariant b = m_right->call(isAtom, index, name, root, data, timeline, vars);
		if(!a.isValid() || !b.isValid() || a.type() != QVariant::Bool || b.type() != QVariant::Bool) return QVariant();

		return QVariant::fromValue<bool>(a.toBool() || b.toBool());
	}
	QString display() const final{ return m_left->display()+" & "+m_right->display();}
};


//Math
/*!
 * @ingroup Filter
 */
class Plus: public Operator{
public:
	Plus(FilterNode* left, FilterNode* right);
	virtual ~Plus();
	QVariant call(bool isAtom, int index, const QString& name, QObject* root, Atoms* data, Timeline* timeline, Variables& vars) const final;
	QString display() const final;
};
/*!
 * @ingroup Filter
 */
class Minus: public Operator{
public:
	Minus(FilterNode* left, FilterNode* right);
	virtual ~Minus();

	QVariant call(bool isAtom, int index, const QString& name, QObject* root, Atoms* data, Timeline* timeline, Variables& vars) const final;
	QString display() const final;
};
/*!
 * @ingroup Filter
 */
class Mult: public Operator{
public:
	Mult(FilterNode* left, FilterNode* right);
	virtual ~Mult();

	QVariant call(bool isAtom, int index, const QString& name, QObject* root, Atoms* data, Timeline* timeline, Variables& vars) const final;
	QString display() const final;
};
/*!
 * @ingroup Filter
 */
class Div: public Operator{
public:
	Div(FilterNode* left, FilterNode* right);
	virtual ~Div();

	QVariant call(bool isAtom, int index, const QString& name, QObject* root, Atoms* data, Timeline* timeline, Variables& vars) const final;
	QString display() const final;
};
/*!
 * @ingroup Filter
 */
class Mod: public Operator{
public:
	Mod(FilterNode* left, FilterNode* right);
	virtual ~Mod();

	QVariant call(bool isAtom, int index, const QString& name, QObject* root, Atoms* data, Timeline* timeline, Variables& vars) const final;
	QString display() const final;
};

//=========== VARS =============
//ATOM
//ATOM
/*!
 * @ingroup Filter
 */
class AtomIndex: public FilterVariable{
public:
	AtomIndex(){}
	virtual ~AtomIndex(){}

	bool isValid() const final{return true;}
	QVariant call(bool isAtom, int integer, const QString&, QObject*, Atoms*, Timeline*, Variables& ) const final{
		if(isAtom) return QVariant::fromValue<int>(integer);
		return QVariant();
	}
	QString display() const final{ return "AtomIndex";} //AtomIndex

	const QStringList getArguments() const final{ return {"atomindex", "ai"}; }
	const QString getDiscription() const final{ return "Returns the current processed atom index by the filter. Starting with one."; }

	FilterVariable* createVar() const final { return new AtomIndex(); }
};
/*!
 * @ingroup Filter
 */
class AtomName: public FilterVariable{
public:
	AtomName(){}
	virtual ~AtomName(){}

	bool isValid() const final{return true;}
	QVariant call(bool isAtom, int, const QString& name, QObject*, Atoms*, Timeline*, Variables&) const final{
		if(isAtom) return QVariant::fromValue<QString>(name);
		return QVariant();
	}
	QString display() const final{ return "AtomName";} //AtomName

	const QStringList getArguments() const final{ return {"atomname", "an"}; }
	const QString getDiscription() const final{ return "Returns the current processed atom name (like 'CA') by the filter."; }

	FilterVariable* createVar() const final { return new AtomName(); }
};
/*!
 * @ingroup Filter
 */
class AtomElement: public FilterVariable{
public:
	AtomElement(){}
	virtual ~AtomElement(){}

	bool isValid() const final{return true;}
	QVariant call(bool isAtom, int index, const QString&, QObject*, Atoms* data, Timeline*, Variables& ) const final{
		if(isAtom) return QVariant::fromValue<QString>(data->getAtom(index-1).element);
		return QVariant();
	}
	QString display() const final{ return "AtomElement";} //AtomElement

	const QStringList getArguments() const final{ return {"atomelement", "ae"}; }
	const QString getDiscription() const final{ return "Returns the current processed atom element (like 'H') by the filter."; }

	FilterVariable* createVar() const final { return new AtomElement(); }
};
/*!
 * @ingroup Filter
 */
class AtomResidueIndex: public FilterVariable{
public:
	AtomResidueIndex(){}
	virtual ~AtomResidueIndex(){}

	bool isValid() const final{return true;}
	QVariant call(bool isAtom, int integer, const QString&, QObject*, Atoms* data, Timeline*, Variables& ) const final{
		if(isAtom && integer > 0 && integer <= data->numberOfAtroms()) return QVariant::fromValue<int>(data->getAtom(integer-1).groupID);
		return QVariant();
	}
	QString display() const final{ return "AtomResidueIndex";} //AtomIndex

	const QStringList getArguments() const final{ return {"atomresidueindex", "ari"}; }
	const QString getDiscription() const final{ return "Returns the residue index of the current processed atom."; }

	FilterVariable* createVar() const final { return new AtomResidueIndex(); }
};
/*!
 * @ingroup Filter
 */
class AtomResidueName: public FilterVariable{
public:
	AtomResidueName(){}
	virtual ~AtomResidueName(){}

	bool isValid() const final{return true;}
	QVariant call(bool isAtom, int integer, const QString&, QObject*, Atoms* data, Timeline*, Variables& ) const final{
		if(isAtom && integer > 0 && integer <= data->numberOfAtroms()) return QVariant::fromValue<QString>(data->getAtom(integer-1).residue);
		return QVariant();
	}
	QString display() const final{ return "AtomResidueName";} //AtomResidueName

	const QStringList getArguments() const final{ return {"atomresiduename", "arn"}; }
	const QString getDiscription() const final{ return "Returns the residue name of the current processed atom."; }

	FilterVariable* createVar() const final { return new AtomResidueName(); }
};
/*!
 * @ingroup Filter
 */
class MaxAtomLayer: public FilterVariable{
public:
	MaxAtomLayer(){}
	virtual ~MaxAtomLayer(){}

	bool isValid() const final{return true;}
	int neededConnects() const final { return  FILTER_NODE_CONNECT_STARTFRAME_CHANGE | FILTER_NODE_CONNECT_ENDFRAME_CHANGE; };
	QVariant call(bool isAtom, int index, const QString& , QObject*, Atoms* data, Timeline* timeline, Variables&) const final{
		if(!isAtom || !data->numberOfAtroms()) return QVariant();
		float max = 0;
		for(int i = timeline->getStartFrame(); i < timeline->getEndFrame(); i++){
			const float layer = data->getLayer(i).layers[index-1];
			if(layer > max) max = layer;
		}
		return QVariant::fromValue<double>(max);

	}
	QString display() const final{ return "MaxAtomLayer";} //"ResidueIndex"

	const QStringList getArguments() const final{ return {"maxatomlayer", "maxal"}; }
	const QString getDiscription() const final{ return "Returns the maximum atom layer between the set start and end frame of the current processed atom."; }

	FilterVariable* createVar() const final { return new MaxAtomLayer(); }
};
/*!
 * @ingroup Filter
 */
class MinAtomLayer: public FilterVariable{
public:
	MinAtomLayer(){}
	virtual ~MinAtomLayer(){}

	bool isValid() const final{return true;}
	int neededConnects() const final { return  FILTER_NODE_CONNECT_STARTFRAME_CHANGE | FILTER_NODE_CONNECT_ENDFRAME_CHANGE; };
	QVariant call(bool isAtom, int index, const QString&, QObject*, Atoms* data, Timeline* timeline, Variables&) const final{
		if(!isAtom || !data->numberOfAtroms()) return QVariant();
		float min = std::numeric_limits<float>::max();
		for(int i = timeline->getStartFrame(); i < timeline->getEndFrame(); i++){
			const float layer = data->getLayer(i).layers[index-1];
			if(layer < min) min = layer;
		}
		return QVariant::fromValue<double>(min);
	}
	QString display() const final{ return "MinAtomLayer";} //"ResidueIndex"

	const QStringList getArguments() const final{ return {"minatomlayer", "minal"}; }
	const QString getDiscription() const final{ return "Returns the minimum atom layer between the set start and end frame of the current processed atom."; }

	FilterVariable* createVar() const final { return new MinAtomLayer(); }
};
/*!
 * @ingroup Filter
 */
class IsSide: public FilterVariable{
public:
	IsSide(){}
	virtual ~IsSide(){}

	bool isValid() const final{return true;}
	QVariant call(bool isAtom, int, const QString& name, QObject*, Atoms*, Timeline*, Variables& ) const final{
		if(isAtom){
			if(name == "O" || name == "H" || name == "H2" || name == "H3" || name == "C" || name == "CA" || name == "N" || name == "HA") return QVariant::fromValue<bool>(false);
			else return QVariant::fromValue<bool>(true);
		}
		return QVariant();
	}
	QString display() const final{ return "isSide";} //AtomIndex

	const QStringList getArguments() const final{ return {"isside"}; }
	const QString getDiscription() const final{ return "Is true if the current processed atom belongs to the side chain."; }

	FilterVariable* createVar() const final { return new IsSide(); }
};
/*!
 * @ingroup Filter
 */
class IsBackbone: public FilterVariable{
public:
	IsBackbone(){}
	virtual ~IsBackbone(){}

	bool isValid() const final{return true;}
	QVariant call(bool isAtom, int, const QString& name, QObject*, Atoms*, Timeline*, Variables& ) const final{
		if(isAtom){
			if(name == "N" || name == "CA" || name == "C") return QVariant::fromValue<bool>(true);
			else return QVariant::fromValue<bool>(false);
		}
		return QVariant();
	}
	QString display() const final{ return "isBackbone";} //AtomIndex

	const QStringList getArguments() const final{ return {"isbackbone"}; }
	const QString getDiscription() const final{ return "Is true if the current processed atom belongs to the backbone chain."; }

	FilterVariable* createVar() const final { return new IsBackbone(); }
};
/*!
 * @ingroup Filter
 */
class SelectedAtomIndex: public FilterVariable{
public:
	SelectedAtomIndex(){}
	virtual ~SelectedAtomIndex(){}

	bool isValid() const final{return true;}
	int neededConnects() const final { return  FILTER_NODE_CONNECT_ATOM_SELECTED_CHANGE; };
	QVariant call(bool, int, const QString&, QObject*, Atoms* data, Timeline*, Variables& ) const final{
		if(data->getSelectedAtom() >= 0)
			return QVariant::fromValue<int>(data->getSelectedAtom());
		return QVariant();
	}
	QString display() const final{ return "SelectedAtomIndex";}

	const QStringList getArguments() const final{ return {"selectedatomindex", "sai"}; }
	const QString getDiscription() const final{ return "Returns the index of the selected atom. If none is selected then value is invalid. "; }

	FilterVariable* createVar() const final { return new SelectedAtomIndex(); }
};
/*!
 * @ingroup Filter
 */
class Frame: public FilterVariable{
public:
	Frame(){}
	virtual ~Frame(){}


	int neededConnects() const final { return  FILTER_NODE_IS_LIVE; };
	bool isValid() const final{return true;}
	QVariant call(bool isAtom, int index, const QString& name, QObject* root, Atoms* data, Timeline* timeline, Variables& vars ) const final{
		QVariant v = vars.value("frame", FilterSharedPtr(new Invalid()))->call(isAtom, index, name, root, data, timeline, vars);
		return v;
	}
	QString display() const final{ return "Frame";} //AtomIndex

	const QStringList getArguments() const final{ return {"frame"}; }
	const QString getDiscription() const final{ return "Returns the current used frame by the caller."; }

	FilterVariable* createVar() const final { return new Frame(); }
};


//RESIDUE
/*!
 * @ingroup Filter
 */
class ResidueIndex: public FilterVariable{
public:
	ResidueIndex(){}
	virtual ~ResidueIndex(){}

	bool isValid() const final{return true;}
	QVariant call(bool isAtom, int integer, const QString&, QObject*, Atoms*, Timeline*, Variables&) const final{
		if(!isAtom) return QVariant::fromValue<int>(integer);
		return QVariant();
	}
	QString display() const final{ return "ResidueIndex";} //"ResidueIndex"

	const QStringList getArguments() const final{ return {"residueindex", "ri"}; }
	const QString getDiscription() const final{ return "Returns the current processed residue index by the filter. Starting with one. "; }

	FilterVariable* createVar() const final { return new ResidueIndex(); }
};
/*!
 * @ingroup Filter
 */
class ResidueName: public FilterVariable{
public:
	ResidueName(){}
	virtual ~ResidueName(){}

	bool isValid() const final{return true;}
	QVariant call(bool isAtom, int, const QString& name, QObject*, Atoms*, Timeline*, Variables&) const final{
		if(!isAtom) return QVariant::fromValue<QString>(name);
		return QVariant();
	}
	QString display() const final{ return "ResidueName";} //"ResidueIndex"

	const QStringList getArguments() const final{ return {"residuename", "rn"}; }
	const QString getDiscription() const final{ return "Returns the current processed residue name (like 'CYS') by the filter. "; }

	FilterVariable* createVar() const final { return new ResidueName(); }
};
/*!
 * @ingroup Filter
 */
class MaxResidueLayer: public FilterVariable{
public:
	MaxResidueLayer(){}
	virtual ~MaxResidueLayer(){}

	bool isValid() const final{return true;}
	int neededConnects() const final { return  FILTER_NODE_CONNECT_STARTFRAME_CHANGE | FILTER_NODE_CONNECT_ENDFRAME_CHANGE; };
	QVariant call(bool isAtom, int index, const QString& name, QObject*, Atoms* data, Timeline* timeline, Variables&) const final{
		if(!isAtom){
			if(name == "HOH") return QVariant();
			float maxRL = 0;
			for(int i = timeline->getStartFrame(); i < timeline->getEndFrame(); i++){
				const float v = data->getGroupLayerAvarage(index-1, i);
				if(v > maxRL) maxRL = v;
			}
			return QVariant::fromValue<double>(maxRL);
		}
		return QVariant();
	}
	QString display() const final{ return "MaxResidueLayer";} //"ResidueIndex"

	const QStringList getArguments() const final{ return {"maxresiduelayer", "maxrl"}; }
	const QString getDiscription() const final{ return "Returns the maximum residue layer between the set start and end frame of the current processed residue."; }

	FilterVariable* createVar() const final { return new MaxResidueLayer(); }
};
/*!
 * @ingroup Filter
 */
class MinResidueLayer: public FilterVariable{
public:
	MinResidueLayer(){}
	virtual ~MinResidueLayer(){}

	bool isValid() const final{return true;}
	int neededConnects() const final { return  FILTER_NODE_CONNECT_STARTFRAME_CHANGE | FILTER_NODE_CONNECT_ENDFRAME_CHANGE; };
	QVariant call(bool isAtom, int index, const QString& name, QObject*, Atoms* data, Timeline* timeline, Variables&) const final{
		if(!isAtom){
			if(name == "HOH") return QVariant();
			float minRL = 9999;
			for(int i = timeline->getStartFrame(); i < timeline->getEndFrame(); i++){
				const float v = data->getGroupLayerAvarage(index-1, i);
				if(v < minRL) minRL = v;
			}
			return QVariant::fromValue<double>(minRL);
		}
		return QVariant();
	}
	QString display() const final{ return "MinResidueLayer";} //"ResidueIndex"

	const QStringList getArguments() const final{ return {"minresiduelayer", "minrl"}; }
	const QString getDiscription() const final{ return "Returns the minimum residue layer between the set start and end frame of the current processed residue."; }

	FilterVariable* createVar() const final { return new MinResidueLayer(); }
};

//=========== Functions =============
/*!
 * @ingroup Filter
 */
class TrackerFunc: public FilterFunction{
public:
	TrackerFunc(){};
	TrackerFunc(const QVector<FilterNode*>& arguments): FilterFunction(arguments){
		if(m_arguments.empty()) m_arguments.push_back(new Integer(0));
	}
	virtual ~TrackerFunc(){}

	int neededConnects() const final { return FilterFunction::neededConnects() | FILTER_NODE_CONNECT_FRAME_CHANGE; };
	QVariant call(bool isAtom, int i, const QString& name, QObject* root, Atoms* data, Timeline* timeline, Variables& vars) const final{
		QVariant v = m_arguments[0]->call(isAtom, i, name, root, data, timeline, vars);
		if(!v.isValid() || v.type() != QVariant::Int) return QVariant();
		const int targetTimelineIndex = v.toInt();
		if(targetTimelineIndex >= 0 && targetTimelineIndex < timeline->getSize())
			return QVariant::fromValue<int>(timeline->getFrame(targetTimelineIndex));

		return QVariant();
	}
	QString display() const final{ return "Tracker("+m_arguments[0]->display()+")";} //AtomIndex

	QString name() const final { return "Tracker(int index)"; }

	const QStringList getArguments() const final;
	const QString getDiscription() const final;

	virtual int minArguments() const final;
	virtual int maxArguments() const final;

	virtual FilterFunction* createFunc(const QVector<FilterNode*>&) const final;
};
/*!
 * @ingroup Filter
 */
class ResidueLayerFunc: public FilterFunction{
public:
    ResidueLayerFunc(){};
    ResidueLayerFunc(const QVector<FilterNode*>& arguments): FilterFunction(arguments){
        if(m_arguments.empty()) m_arguments.push_back(new Integer(0));
    }
    virtual ~ResidueLayerFunc(){}

    QVariant call(bool isAtom, int i, const QString& name, QObject* root, Atoms* data, Timeline* timeline, Variables& vars) const final{
        if(isAtom || name == "HOH") return QVariant();
        QVariant v = m_arguments[0]->call(isAtom, i, name, root, data, timeline, vars);
        if(!v.isValid() || v.type() != QVariant::Int) return QVariant();
        const int f = v.toInt();

        if(f >= 0 && f < timeline->getMaxFrame())
            return QVariant::fromValue<double>(data->getGroupLayerAvarage(i, f));

        return QVariant();
    }
    QString display() const final{ return "ResidueLayer("+m_arguments[0]->display()+")";} //AtomIndex

    QString name() const final { return "ResidueLayer(int frame)"; }

    const QStringList getArguments() const final;
    const QString getDiscription() const final;

    virtual int minArguments() const final;
    virtual int maxArguments() const final;

    virtual FilterFunction* createFunc(const QVector<FilterNode*>&) const final;
};/*!
 * @ingroup Filter
 */
class AtomLayerFunc: public FilterFunction{
public:
    AtomLayerFunc(){};
    AtomLayerFunc(const QVector<FilterNode*>& arguments): FilterFunction(arguments){
        if(m_arguments.empty()) m_arguments.push_back(new Integer(0));
    }
    virtual ~AtomLayerFunc(){}

    QVariant call(bool isAtom, int i, const QString& name, QObject* root, Atoms* data, Timeline* timeline, Variables& vars) const final{
        if(!isAtom) return QVariant();
        QVariant v = m_arguments[0]->call(isAtom, i-1, name, root, data, timeline, vars);
        if(!v.isValid() || v.type() != QVariant::Int) return QVariant();
        const int f = v.toInt();

        if(f >= 0 && f < timeline->getMaxFrame())
            return QVariant::fromValue<double>(data->getAtomLayer(i-1, f, false, false));

        return QVariant();
    }
    QString display() const final{ return "AtomLayer("+m_arguments[0]->display()+")";} //AtomIndex

    QString name() const final { return "AtomLayer(int frame)"; }

    const QStringList getArguments() const final;
    const QString getDiscription() const final;

    virtual int minArguments() const final;
    virtual int maxArguments() const final;

    virtual FilterFunction* createFunc(const QVector<FilterNode*>&) const final;
};
/*!
 * @ingroup Filter
 */
class SumFunc: public FilterFunction{
public:
	SumFunc(){};
	SumFunc(const QVector<FilterNode*>& arguments);
	virtual ~SumFunc();

	QVariant call(bool isAtom, int i, const QString& name, QObject* root, Atoms* data, Timeline* timeline, Variables& vars) const final;
	QString display() const final;

	QString name() const final { return "Sum(int start, int end, num func, string var = 'x')"; }

	const QStringList getArguments() const final;
	const QString getDiscription() const final;

	virtual int minArguments() const final;
	virtual int maxArguments() const final;

	virtual FilterFunction* createFunc(const QVector<FilterNode*>&) const final;
};
/*!
 * @ingroup Filter
 */
class ProductFunc: public FilterFunction{
public:
	ProductFunc(){};
	ProductFunc(const QVector<FilterNode*>& arguments);
	virtual ~ProductFunc();

	QVariant call(bool isAtom, int i, const QString& name, QObject* root, Atoms* data, Timeline* timeline, Variables& vars) const final;
	QString display() const final;

	QString name() const final { return "Product(int start, int end, num func, string var = 'x')"; }

	const QStringList getArguments() const final;
	const QString getDiscription() const final;

	virtual int minArguments() const final;
	virtual int maxArguments() const final;

	virtual FilterFunction* createFunc(const QVector<FilterNode*>&) const final;
};
/*!
 * @ingroup Filter
 */
class QuantizeFunc: public FilterFunction{
public:
	QuantizeFunc(){};
	QuantizeFunc(const QVector<FilterNode*>& arguments): FilterFunction(arguments){
		if(m_arguments.size() == 1)
			m_arguments.push_back(new Double(0.5));
	}
	virtual ~QuantizeFunc(){}

	QVariant call(bool isAtom, int i, const QString& name, QObject* root, Atoms* data, Timeline* timeline, Variables& vars) const final{
		if(isAtom || name == "HOH") return QVariant();
		QVariant value = m_arguments[0]->call(isAtom, i, name, root, data, timeline, vars);
		QVariant step = m_arguments[1]->call(isAtom, i, name, root, data, timeline, vars);
		if(!value.isValid() || !step.isValid()) return QVariant();

		const double v = value.toDouble();
		const double s = step.toDouble();
		const double hs = s/2.0;
		return QVariant::fromValue<double>(floor((v+hs)/s)*s);
	}
	QString display() const final{ return "Quantize("+m_arguments[0]->display()+", "+m_arguments[1]->display()+")";} //AtomIndex

	QString name() const final { return "Quantize(float value, float step = 0.5)"; }

	const QStringList getArguments() const final;
	const QString getDiscription() const final;

	virtual int minArguments() const final;
	virtual int maxArguments() const final;

	virtual FilterFunction* createFunc(const QVector<FilterNode*>&) const final;
};
/*!
 * @ingroup Filter
 */
class IfFunc: public FilterFunction{
public:
	IfFunc(){};
	IfFunc(const QVector<FilterNode*>& arguments): FilterFunction(arguments){}
	virtual ~IfFunc(){}

	QVariant call(bool isAtom, int i, const QString& name, QObject* root, Atoms* data, Timeline* timeline, Variables& vars) const final;
	QString display() const final;

	QString name() const final;

	const QStringList getArguments() const final;
	const QString getDiscription() const final;

	virtual int minArguments() const final;
	virtual int maxArguments() const final;

	virtual FilterFunction* createFunc(const QVector<FilterNode*>&) const final;
};



} /* namespace Atom */

#endif /* LIBRARIES_ATOMS_FILTERDEFINITIONS_H_ */
