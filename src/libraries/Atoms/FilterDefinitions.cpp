/*
 * FilterDefinitions.cpp
 *
 *  Created on: 11.08.2017
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

#include <FilterDefinitions.h>

namespace Atom {




/// MATH
Plus::Plus(FilterNode* left, FilterNode* right): Operator(left, right){}
Plus::~Plus(){}

QVariant Plus::call(bool isAtom, int index, const QString& name, QObject* root,Atoms* data, Timeline* timeline, Variables& vars) const{
	QVariant a = m_left->call(isAtom, index, name, root, data, timeline, vars);
	QVariant b = m_right->call(isAtom, index, name, root, data, timeline, vars);
	if(!a.isValid() || !b.isValid()) return QVariant();
	switch (a.type()) {
		//case Void:{}break;
		case QVariant::Int:{
			switch (b.type()) {
				//case Void:{}break;
				case QVariant::Int:{return QVariant::fromValue<int>(    a.toInt() + b.toInt());}break;
				case QVariant::Double:{return QVariant::fromValue<double>(float(a.toInt()) + b.toDouble());}break;
				case QVariant::Bool:{return QVariant::fromValue<double>( a.toInt() + int(b.toBool()));}break;
				case QVariant::String:{return QVariant::fromValue<QString>(QString::number(a.toInt()) + b.toString());}break;
				default:break;
			}
		}break;
		case QVariant::Double:{
			switch (b.type()) {
				//case Void:{}break;
				case QVariant::Int:{return QVariant::fromValue<double>(  a.toDouble() + float(b.toInt()));}break;
				case QVariant::Double:{return QVariant::fromValue<double>(a.toDouble() + b.toDouble());}break;
				case QVariant::Bool:{return QVariant::fromValue<double>( a.toDouble() + float(b.toBool()));}break;
				case QVariant::String:{return QVariant::fromValue<QString>(QString::number(a.toDouble()) + b.toString());}break;
				default:break;
			}
		}break;
		case  QVariant::Bool:{
			switch (b.type()) {
				//case Void:{}break;
				case QVariant::Int:{return QVariant::fromValue<int>(     int(a.toBool()) + b.toInt());}break;
				case QVariant::Double:{return QVariant::fromValue<double>( float(a.toBool()) + b.toDouble());}break;
				case QVariant::Bool:{return QVariant::fromValue<int>(    int(a.toBool())  + int(b.toBool()) );}break;
				case QVariant::String:{return QVariant::fromValue<QString>( QString::number(int(a.toBool())) + b.toString());}break;
				default:break;
			}
		}break;
		case  QVariant::String:{
			switch (b.type()) {
				//case Void:{}break;
				case QVariant::Int:{return QVariant::fromValue<QString>(   a.toString() + QString::number(b.toInt()));}break;
				case QVariant::Double:{return QVariant::fromValue<QString>( a.toString() + QString::number(b.toDouble()));}break;
				case QVariant::Bool:{return QVariant::fromValue<QString>(  a.toString() + QString::number(int(b.toBool())) );}break;
				case QVariant::String:{return QVariant::fromValue<QString>(   a.toString() + b.toString());}break;
				default:break;
			}
		}break;
		default:break;
	}

	return QVariant();
}
QString Plus::display() const { return m_left->display()+" + "+m_right->display();}



Minus::Minus(FilterNode* left, FilterNode* right): Operator(left, right){}
Minus::~Minus(){}


QVariant Minus::call(bool isAtom, int index, const QString& name, QObject* root, Atoms* data, Timeline* timeline, Variables& vars) const{
	QVariant a = m_left->call(isAtom, index, name, root, data, timeline, vars);
	QVariant b = m_right->call(isAtom, index, name, root, data, timeline, vars);
	if(!a.isValid() || !b.isValid()) return QVariant();
	switch (a.type()) {
		//case Void:{}break;
		case QVariant::Int:{
			switch (b.type()) {
				//case Void:{}break;
				case QVariant::Int:{return QVariant::fromValue<int>(      a.toInt() - b.toInt());}break;
				case QVariant::Double:{return QVariant::fromValue<double>(  float(a.toInt()) - b.toDouble());}break;
				case QVariant::Bool:{return QVariant::fromValue<int>(     a.toInt() - int(b.toBool()));}break;
				case QVariant::String:{return QVariant::fromValue<QString>(  b.toString().remove( QString::number(a.toInt()) , Qt::CaseInsensitive)); }break;
				default:break;
			}
		}break;
		case QVariant::Double:{
			switch (b.type()) {
				//case Void:{}break;
				case QVariant::Int:{return QVariant::fromValue<double>(  a.toDouble() - float(b.toInt()));}break;
				case QVariant::Double:{return QVariant::fromValue<double>(a.toDouble() - b.toDouble());}break;
				case QVariant::Bool:{return QVariant::fromValue<double>( a.toDouble() - float(b.toBool()));}break;
				case QVariant::String:{return QVariant::fromValue<QString>(b.toString().remove( QString::number(a.toDouble())  , Qt::CaseInsensitive)); }break;
				default:break;
			}
		}break;
		case QVariant::Bool:{
			switch (b.type()) {
				//case Void:{}break;
				case QVariant::Int:{return QVariant::fromValue<int>(     int(a.toBool()) - b.toInt());}break;
				case QVariant::Double:{return QVariant::fromValue<double>( float(a.toBool()) - b.toDouble());}break;
				case QVariant::Bool:{return QVariant::fromValue<int>(    int(a.toBool())  - int(b.toBool()) );}break;
				case QVariant::String:{return QVariant::fromValue<QString>( b.toString().remove( a.toBool()? "true":"false" , Qt::CaseInsensitive)); }break;
				default:break;
			}
		}break;
		case QVariant::String:{
			switch (b.type()) {
				//case Void:{}break;
				case QVariant::Int:{return QVariant::fromValue<QString>(   a.toString().remove( QString::number(b.toInt()) , Qt::CaseInsensitive)); }break;
				case QVariant::Double:{return QVariant::fromValue<QString>( a.toString().remove( QString::number(b.toDouble()) , Qt::CaseInsensitive));  }break;
				case QVariant::Bool:{return QVariant::fromValue<QString>(  a.toString().remove( b.toBool()? "true":"false", Qt::CaseInsensitive)); }break;
				case QVariant::String:{return QVariant::fromValue<QString>(   a.toString().remove( b.toString() , Qt::CaseInsensitive)); }break;
				default:break;
			}
		}break;
		default:break;
	}

	return QVariant();
}
QString Minus::display() const { return m_left->display()+" - "+m_right->display();}


Mult::Mult(FilterNode* left, FilterNode* right): Operator(left, right){}
Mult::~Mult(){}

QVariant Mult::call(bool isAtom, int index, const QString& name, QObject* root, Atoms* data, Timeline* timeline, Variables& vars) const{
	QVariant a = m_left->call(isAtom, index, name, root, data, timeline, vars);
	QVariant b = m_right->call(isAtom, index, name, root, data, timeline, vars);
	if(!a.isValid() || !b.isValid()) return QVariant();
	switch (a.type()) {
		case QVariant::Bool:{
			switch (b.type()) {
				case QVariant::Bool:{return a.toBool()? b : QVariant::fromValue<bool>(0); }break;
				case QVariant::Int:{return a.toBool()? b : QVariant::fromValue<int>(0); }break;
				case QVariant::Double:{return a.toBool()? b : QVariant::fromValue<double>(0); }break;
				case QVariant::String:{return a.toBool()? b : QVariant::fromValue<QString>(""); }break;
				default:break;
			}
		}break;
		case QVariant::Int:{
			switch (b.type()) {
				case QVariant::Bool:{return b.toBool()? a : QVariant::fromValue<int>(0); }break;
				case QVariant::Int:{return  QVariant::fromValue<int>(      a.toInt() * b.toInt());}break;
				case QVariant::Double:{return QVariant::fromValue<double>(  double(a.toInt()) * b.toDouble());}break;
				case QVariant::String:{
					QString str;
					for(int i = 0; i < a.toInt(); i++){
						str.append(b.toString());
					}
					return QVariant::fromValue<QString>(str);
				}break;
				default:break;
			}
		}break;
		case QVariant::Double:{
			switch (b.type()) {
				case QVariant::Bool:{return b.toBool()? a : QVariant::fromValue<double>(0); }break;
				case QVariant::Int:{ return QVariant::fromValue<double>(  a.toDouble() * double(b.toInt()));}break;
				case QVariant::Double:{return QVariant::fromValue<double>(a.toDouble() * b.toDouble());}break;
				case QVariant::String:{
					QString str;
					for(int i = 0; i < a.toInt(); i++){
						str.append(b.toString());
					}
					return QVariant::fromValue<QString>(str);
				}break;
				default:break;
			}
		}break;
		case QVariant::String:{
			switch (b.type()) {
				//case Void:{}break;
				case QVariant::Bool:{return b.toBool()? a : QVariant::fromValue<QString>(""); }break;
				case QVariant::Int:
				case QVariant::Double:{
					QString str;
					for(int i = 0; i < b.toInt(); i++){
						str.append(a.toString());
					}
					return QVariant::fromValue<QString>(str);
				}break;
				default:break;
			}
		}break;
		default:break;
	}

	return QVariant();
}
QString Mult::display() const{ return m_left->display()+" * "+m_right->display();}

Div::Div(FilterNode* left, FilterNode* right): Operator(left, right){}
Div::~Div(){}

QVariant Div::call(bool isAtom, int index, const QString& name, QObject* root, Atoms* data, Timeline* timeline, Variables& vars) const{
	QVariant a = m_left->call(isAtom, index, name, root, data, timeline, vars);
	QVariant b = m_right->call(isAtom, index, name, root, data, timeline, vars);
	if(!a.isValid() || !b.isValid()) return QVariant();
	switch (a.type()) {
		//case Void:{}break;
		case QVariant::Int:{
			switch (b.type()) {
				case QVariant::Int:{return (b.toInt()==0)? QVariant() : QVariant::fromValue<int>(      a.toInt() / b.toInt());}break;
				case QVariant::Double:{return (b.toDouble()==0)? QVariant() : QVariant::fromValue<double>(  double(a.toInt()) / b.toDouble());}break;
				default:break;
			}
		}break;
		case QVariant::Double:{
			switch (b.type()) {
				case QVariant::Int:{ return (b.toInt()==0)? QVariant() : QVariant::fromValue<double>(  a.toDouble() / double(b.toInt()));}break;
				case QVariant::Double:{return (b.toDouble()==0)? QVariant() : QVariant::fromValue<double>(a.toDouble() / b.toDouble());}break;
				default:break;
			}
		}break;
		case QVariant::String:{
			switch (b.type()) {
				//case Void:{}break;
				case QVariant::Int:{return QVariant::fromValue<bool>(   a.toString().contains( QString::number(b.toInt()) , Qt::CaseInsensitive)); }break;
				case QVariant::Double:{return QVariant::fromValue<bool>( a.toString().contains( QString::number(b.toDouble()) , Qt::CaseInsensitive));  }break;
				case QVariant::Bool:{return QVariant::fromValue<bool>(  a.toString().contains( b.toBool()? "true":"false", Qt::CaseInsensitive)); }break;
				case QVariant::String:{return QVariant::fromValue<bool>(   a.toString().contains( b.toString() , Qt::CaseInsensitive)); }break;
				default:break;
			}
		}break;
		default:break;
	}

	return QVariant();
}
QString Div::display() const{ return m_left->display()+" / "+m_right->display();}


Mod::Mod(FilterNode* left, FilterNode* right): Operator(left, right){}
Mod::~Mod(){}

QVariant Mod::call(bool isAtom, int index, const QString& name, QObject* root, Atoms* data, Timeline* timeline, Variables& vars) const {
	QVariant a = m_left->call(isAtom, index, name, root, data, timeline, vars);
	QVariant b = m_right->call(isAtom, index, name, root, data, timeline, vars);
	if(!a.isValid() || !b.isValid()) return QVariant();
	switch (a.type()) {
		//case Void:{}break;
		case QVariant::Int:{
			switch (b.type()) {
				case QVariant::Int:{return QVariant::fromValue<int>(      a.toInt() % b.toInt());}break;
				case QVariant::Double:{return QVariant::fromValue<double>(  fmodf(float(a.toInt()),b.toDouble())); }break;
				default:break;
			}
		}break;
		case QVariant::Double:{
			switch (b.type()) {
				case QVariant::Int:{return QVariant::fromValue<double>(   fmodf(a.toDouble(), float(b.toInt())));}break;
				case QVariant::Double:{return QVariant::fromValue<double>( fmodf(a.toDouble(), b.toDouble()));}break;
				default:break;
			}
		}break;
		default:break;
	}

	return QVariant();
}
QString Mod::display() const { return m_left->display()+" % "+m_right->display();}


const QStringList TrackerFunc::getArguments() const{
	return {"tracker", "t"};
}
const QString TrackerFunc::getDiscription() const{
	return "Returns the frame position of a tracker.";
}

int TrackerFunc::minArguments() const{
	return 0;
}
int TrackerFunc::maxArguments() const{
	return 1;
}

FilterFunction* TrackerFunc::createFunc(const QVector<FilterNode*>& args) const{
	return	new TrackerFunc(args);
}

const QStringList ResidueLayerFunc::getArguments() const{
    return {"residuelayer", "rl", "residuel"};
}
const QString ResidueLayerFunc::getDiscription() const{
    return "Returns the residue layer at the given frame of a current processed residue.";
}

int ResidueLayerFunc::minArguments() const{
    return 0;
}
int ResidueLayerFunc::maxArguments() const{
    return 1;
}

FilterFunction* ResidueLayerFunc::createFunc(const QVector<FilterNode*>& args) const{
    return	new ResidueLayerFunc(args);
}


const QStringList AtomLayerFunc::getArguments() const{
    return {"atomlayer", "al", "atoml"};
}
const QString AtomLayerFunc::getDiscription() const{
    return "Returns the atom layer at the given frame of a current processed atom.";
}

FilterFunction* AtomLayerFunc::createFunc(const QVector<FilterNode*>& args) const{
    return	new AtomLayerFunc(args);
}

int AtomLayerFunc::minArguments() const{
    return 0;
}
int AtomLayerFunc::maxArguments() const{
    return 1;
}


SumFunc::SumFunc(const QVector<FilterNode*>& arguments): FilterFunction(arguments){
	if(m_arguments.size() == 3) m_arguments.push_back( new Str("x") );
}
SumFunc::~SumFunc(){}


QVariant SumFunc::call(bool isAtom, int i, const QString& name, QObject* root, Atoms* data, Timeline* timeline, Variables& vars) const {
	QVariant vstart = m_arguments[0]->call(isAtom, i, name, root, data, timeline, vars);
	QVariant vend =  m_arguments[1]->call(isAtom, i, name, root, data, timeline, vars);
	if(!vstart.isValid() || !vend.isValid() || vstart.type() != QVariant::Int || vend.type() != QVariant::Int) return QVariant();
	const int start = vstart.toInt();
	const int end = vend.toInt();
	Variables vars_copy = vars;
	double sum = 0;

	//get string var
	QVariant var =  m_arguments[3]->call(isAtom, i, name, root, data, timeline, vars);
	if(!var.isValid() || var.type() != QVariant::String) return QVariant();
	const QString strVar = var.toString();

	QVariant v;
	for(int s = start; s <= end; s++){
		vars_copy[strVar] = FilterSharedPtr(new Integer(s));
		v =  m_arguments[2]->call(isAtom, i, name, root, data, timeline, vars_copy);
		switch (v.type()) {
			case QVariant::Int:{sum += v.toInt();}break;
			case QVariant::Double:{sum += v.toDouble();}break;
			default:return QVariant();break;
		}
	}
	return (v.type() == QVariant::Int)? QVariant::fromValue<int>(int(sum)) : QVariant::fromValue<double>(sum);
}
QString SumFunc::display() const { return "Sum("+ m_arguments[0]->display()+", "+m_arguments[1]->display()+", "+m_arguments[2]->display()+", "+m_arguments[3]->display()+")";}

const QStringList SumFunc::getArguments() const{
	return {"sum"};
}
const QString SumFunc::getDiscription() const{
	return "Returns the sum of the given function and start-end range";
}

int SumFunc::minArguments() const{
	return 3;
}
int SumFunc::maxArguments() const{
	return 4;
}

FilterFunction* SumFunc::createFunc(const QVector<FilterNode*>& args) const{
	return	new SumFunc(args);
}



ProductFunc::ProductFunc(const QVector<FilterNode*>& arguments): FilterFunction(arguments){
	if(m_arguments.size() == 3) m_arguments.push_back( new Str("x") );
}
ProductFunc::~ProductFunc(){ }

QVariant ProductFunc::call(bool isAtom, int i, const QString& name, QObject* root, Atoms* data, Timeline* timeline, Variables& vars) const {
	QVariant vstart = m_arguments[0]->call(isAtom, i, name, root, data, timeline, vars);
	QVariant vend =  m_arguments[1]->call(isAtom, i, name, root, data, timeline, vars);
	if(!vstart.isValid() || !vend.isValid() || vstart.type() != QVariant::Int || vend.type() != QVariant::Int) return QVariant();
	const int start = vstart.toInt();
	const int end = vend.toInt();
	if(start > end) return QVariant();
	Variables vars_copy = vars;
	double product = 0;

	//get string var
	QVariant var =  m_arguments[3]->call(isAtom, i, name, root, data, timeline, vars);
	if(!var.isValid() || var.type() != QVariant::String) return QVariant();
	const QString strVar = var.toString();

	QVariant v;
	{ //set product so we dont multiply with 0
		vars_copy[strVar] = FilterSharedPtr(new Integer(start));
		v =  m_arguments[2]->call(isAtom, i, name, root, data, timeline, vars_copy);
		switch (v.type()) {
			case QVariant::Int:{product = v.toInt();}break;
			case QVariant::Double:{product = v.toDouble();}break;
			default:return QVariant();break;
		}
	}

	for(int s = start+1; s <= end; s++){
		vars_copy[strVar] = FilterSharedPtr(new Integer(s));
		v =  m_arguments[2]->call(isAtom, i, name, root, data, timeline, vars_copy);
		switch (v.type()) {
			case QVariant::Int:{product *= v.toInt();}break;
			case QVariant::Double:{product *= v.toDouble();}break;
			default:return QVariant();break;
		}
	}
	return (v.type() == QVariant::Int)? QVariant::fromValue<int>(int(product)) : QVariant::fromValue<double>(product);
}
QString ProductFunc::display() const { return "Product("+ m_arguments[0]->display()+", "+m_arguments[1]->display()+", "+m_arguments[2]->display()+", "+m_arguments[3]->display()+")";}

const QStringList ProductFunc::getArguments() const{
	return {"product"};
}
const QString ProductFunc::getDiscription() const{
	return "Returns the product of the given function and start-end range";
}

int ProductFunc::minArguments() const{ return 3; }
int ProductFunc::maxArguments() const{ return 4; }

FilterFunction* ProductFunc::createFunc(const QVector<FilterNode*>& args) const{
	return	new ProductFunc(args);
}

const QStringList QuantizeFunc::getArguments() const{
	return {"quantize"};
}
const QString QuantizeFunc::getDiscription() const{
	return "Rounds the value to the nearest multiple of step.";
}

int QuantizeFunc::minArguments() const { return 1;}
int QuantizeFunc::maxArguments() const { return 2;}

FilterFunction* QuantizeFunc::createFunc(const QVector<FilterNode*>& args) const{
	return new QuantizeFunc(args);
}


QVariant IfFunc::call(bool isAtom, int i, const QString& name, QObject* root, Atoms* data, Timeline* timeline, Variables& vars) const{
	if(isAtom || name == "HOH") return QVariant();
	QVariant value = m_arguments[0]->call(isAtom, i, name, root, data, timeline, vars);
	if(!value.isValid() || value.type() != QVariant::Bool) return QVariant();

	if(value.toBool())
		return m_arguments[1]->call(isAtom, i, name, root, data, timeline, vars);
	return m_arguments[2]->call(isAtom, i, name, root, data, timeline, vars);

}
QString IfFunc::display() const { return "If("+m_arguments[0]->display()+", "+m_arguments[1]->display()+", "+m_arguments[2]->display()+")";} //AtomIndex

QString IfFunc::name() const  { return "If(bool arg, <true>, <false>)"; }

const QStringList IfFunc::getArguments() const{ return {"if"}; }
const QString IfFunc::getDiscription() const {
	return "If statement. If the first argument is true, then the second argument will be called, else the third.";
}

int IfFunc::minArguments() const {return 3;};
int IfFunc::maxArguments() const {return 3;};

FilterFunction* IfFunc::createFunc(const QVector<FilterNode*>& args) const{
	return new IfFunc(args);
}

} /* namespace Atom */
