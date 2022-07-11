/*
 * AtomsListModel.cpp
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

#include <Atoms/FilterAtoms.h>
#include <FilterDefinitions.h>
#include <QApplication>
#include <QClipboard>


FilterAtomsListModel::FilterAtomsListModel(Atoms* atomsListModel, Timeline* timeline, QObject *parent ): QSortFilterProxyModel(parent), m_data(atomsListModel), m_timeline(timeline){
	setSourceModel(atomsListModel);

	//====== VARIABLES ======
	//ATOM
	m_registredVariables.push_back(new Atom::Frame());
	m_registredVariables.push_back(new Atom::AtomIndex());
	m_registredVariables.push_back(new Atom::AtomName());
	m_registredVariables.push_back(new Atom::AtomElement());
	m_registredVariables.push_back(new Atom::AtomResidueIndex());
	m_registredVariables.push_back(new Atom::AtomResidueName());
    m_registredVariables.push_back(new Atom::MaxAtomLayer());
    m_registredVariables.push_back(new Atom::MinAtomLayer());
	m_registredVariables.push_back(new Atom::IsSide());
	m_registredVariables.push_back(new Atom::IsBackbone());
	m_registredVariables.push_back(new Atom::SelectedAtomIndex());

	//RESIDUE
	m_registredVariables.push_back(new Atom::ResidueIndex());
	m_registredVariables.push_back(new Atom::ResidueName());
	m_registredVariables.push_back(new Atom::MaxResidueLayer());
	m_registredVariables.push_back(new Atom::MinResidueLayer());


	//====== FUNCTIONS ======
	m_registredFunctions.push_back(new Atom::AtomLayerFunc());
	m_registredFunctions.push_back(new Atom::IfFunc());
	m_registredFunctions.push_back(new Atom::TrackerFunc());
	m_registredFunctions.push_back(new Atom::ResidueLayerFunc());
	m_registredFunctions.push_back(new Atom::SumFunc());
	m_registredFunctions.push_back(new Atom::ProductFunc());
	m_registredFunctions.push_back(new Atom::QuantizeFunc());

}

FilterAtomsListModel::~FilterAtomsListModel(){
	for(Atom::FilterVariable* f: m_registredVariables) delete f;
	for(Atom::FilterFunction* f: m_registredFunctions) delete f;
}

void FilterAtomsListModel::refresh(){
	if(m_filters.empty()) return;
	calculateFilters();
	invalidateFilter();
	emit filterHasChanged();
}
void FilterAtomsListModel::clearFilters(){
	m_filters.clear();
	updateRefrechTriggers(0);
	m_resultsResidue.clear();
	m_resultsAtom.clear();
	invalidateFilter();
	emit filterHasChanged();
}



struct Token{
	QString str;
	int type;
};

struct TreeNode{
	int left;
	Token op; //operator
	int right;
};

inline Token& setTokenType(Token& t){
	if(t.str.contains(QRegExp("^[-]?\\d+(([.]\\d+[f]?)|[f])$"))){
		t.type = 2;
		if(t.str.endsWith('f')) t.str = t.str.left(t.str.length()-1);
	}else if(t.str.contains(QRegExp("^[-]?\\d+$"))){
		t.type = 1;
	}else
		t.type = 0;
	return t;
}

Atom::FilterNode* buildTree(QList<Token>::iterator start, QList<Token>::iterator end, const QVector<Atom::FilterVariable*>& registredVariables, const QVector<Atom::FilterFunction*>& registredFunctions);

void buildArguments(QList<Token>::iterator start, QList<Token>::iterator end, QVector<Atom::FilterNode*>& arguments, const QVector<Atom::FilterVariable*>& registredVariables, const QVector<Atom::FilterFunction*>& registredFunctions){
	int braketInclosed = 0; //Contents of brakes is ignored
	for(QList<Token>::iterator it = start; it <= end; it++){

		if(it->type == 7){
			if(it->str == "(") braketInclosed++;
			else if(it->str == ")") braketInclosed--;
		}
		qDebug()<<"buildArguments: "<< it->str<<braketInclosed;

		if(it->type == 8 && braketInclosed == 0){
			arguments.push_back(buildTree(start, it-1, registredVariables, registredFunctions));
			start = it+1;
		}
	}
	arguments.push_back(buildTree(start, end, registredVariables, registredFunctions ));
}

bool areSameBrackets(QList<Token>::iterator start, QList<Token>::iterator end){
	int count = 0;
	for(QList<Token>::iterator it = start; it <= end; it++){
		if(it->type == 7){
			if(it->str == "(") count++;
			else if(it->str == ")") count--;
		}

		if(count == 0 && it != end) return false;
	}

	return true;
}

#define FUNC_ONE_ARG(functionname, classname)	if(arguments.size() == 0)\
													return new Atom::classname();\
												if(arguments.size() == 1)\
													return new Atom::classname(arguments[0]);\
												qDebug()<<__LINE__<<" Invalid number of arguments for function '"<<functionname<<"'! (max 1)";\
												for(Atom::FilterNode* n: arguments)\
													if(n) delete n;\

Atom::FilterNode* buildTree(QList<Token>::iterator start, QList<Token>::iterator end, const QVector<Atom::FilterVariable*>& registredVariables, const QVector<Atom::FilterFunction*>& registredFunctions){
	//if(start<=end) qDebug()<<"start: "<<start->str<<" end: "<<end->str;
	if(start == end){ //vars
		if(start->type == 0){
			const QString& val = start->str.toLower();
			if(val == "true")
				return new Atom::Bool(true);
			else if(val == "false")
				return new Atom::Bool(false);

			for(Atom::FilterVariable* f: registredVariables)
				if(f->getArguments().contains(val)) return f->createVar();

			return new Atom::Str(start->str);
		}else if(start->type == 1){
			return new Atom::Integer(start->str.toInt());
		}else if(start->type == 2){
			return new Atom::Double(start->str.toDouble());
		}
	}else if(start->type == 7 && start->str == "(" && end->type == 7 && end->str == ")" && areSameBrackets(start, end)){//Brackets
		qDebug()<<__LINE__<<"  Brackets";
		return new Atom::Brakets(buildTree(start+1, end-1, registredVariables, registredFunctions));
	}else if(end-start >= 2 && start->type == 0 && (start+1)->type == 7 &&  (end)->type == 7 && areSameBrackets((start+1), end)){ //function
		const QString& functionName = start->str.toLower();

		QVector<Atom::FilterNode*> arguments;
		buildArguments( (start+2), (end-1), arguments, registredVariables, registredFunctions);

		for(Atom::FilterFunction* f: registredFunctions){
			if(f->getArguments().contains(functionName)){
				if(arguments.size() > f->maxArguments() || arguments.size() < f->minArguments()){
					qDebug()<<__LINE__<<" Invalid number of arguments for function "<<functionName<<" there are "<< QString::number(arguments.size()) <<", but minimum:"<<QString::number(f->minArguments())<<"maximum:"<<QString::number(f->maxArguments())<<"'!";\
					break;
				}else
					return f->createFunc(arguments);
			}
		}

		qDebug()<<__LINE__<<"Unable to process function "<<functionName;
		for(Atom::FilterNode* n: arguments)
			if(n) delete n;

	}else if(start < end){
		//find tree split target
		QList<Token>::iterator splitit = start;
		int splitTokenType = 900;
		int braketInclosed = 0; //Contents of brakes is ignored
		for(QList<Token>::iterator it = start; it <= end; it++){
			if(it->type == 7){
				if(it->str == "(") braketInclosed++;
				else if(it->str == ")") braketInclosed--;
			}
			if(braketInclosed != 0) continue;

			if(it->type > 2 && it->type < 7 && splitTokenType > it->type ){
				splitTokenType = it->type;
				splitit = it;
			}
		}

		if(braketInclosed != 0){
			qDebug()<<__LINE__<<" Brackets are uneven!";
			return nullptr;
		}

		qDebug()<<__LINE__<<"splitit: "<<splitit->str;

		switch (splitTokenType) {
			case 3:{
				if(splitit->str == "&" || splitit->str == "&&")
					return new Atom::And(buildTree(start, splitit-1, registredVariables, registredFunctions), buildTree(splitit+1, end, registredVariables, registredFunctions));
				else if(splitit->str == "|" || splitit->str == "||")
					return new Atom::Or(buildTree(start, splitit-1, registredVariables, registredFunctions), buildTree(splitit+1, end, registredVariables, registredFunctions));
			}break;
			case 4:{
				if(splitit->str == "==" || splitit->str == "=")
					return new Atom::Equal(buildTree(start, splitit-1, registredVariables, registredFunctions), buildTree(splitit+1, end, registredVariables, registredFunctions));
				else if( splitit->str == "!=")
					return new Atom::NotEqual(buildTree(start, splitit-1, registredVariables, registredFunctions), buildTree(splitit+1, end, registredVariables, registredFunctions));
				else if( splitit->str == ">")
					return new Atom::Bigger(buildTree(start, splitit-1, registredVariables, registredFunctions), buildTree(splitit+1, end, registredVariables, registredFunctions));
				else if( splitit->str == ">=")
					return new Atom::BiggerEqual(buildTree(start, splitit-1, registredVariables, registredFunctions), buildTree(splitit+1, end, registredVariables, registredFunctions));
				else if( splitit->str == "<")
					return new Atom::Smaller(buildTree(start, splitit-1, registredVariables, registredFunctions), buildTree(splitit+1, end, registredVariables, registredFunctions));
				else if( splitit->str == "<=")
					return new Atom::SmallerEqual(buildTree(start, splitit-1, registredVariables, registredFunctions), buildTree(splitit+1, end, registredVariables, registredFunctions));
			}break;
			case 5:{
				if(splitit->str == "+"){
					return new Atom::Plus(buildTree(start, splitit-1, registredVariables, registredFunctions), buildTree(splitit+1, end, registredVariables, registredFunctions));
				}else if( splitit->str == "-"){
					return new Atom::Minus(buildTree(start, splitit-1, registredVariables, registredFunctions), buildTree(splitit+1, end, registredVariables, registredFunctions));
				}else if( splitit->str == "*"){
					return new Atom::Mult(buildTree(start, splitit-1, registredVariables, registredFunctions), buildTree(splitit+1, end, registredVariables, registredFunctions));
				}else if( splitit->str == "/"){
					return new Atom::Div(buildTree(start, splitit-1, registredVariables, registredFunctions), buildTree(splitit+1, end, registredVariables, registredFunctions));
				}else if( splitit->str == "%")
					return new Atom::Mod(buildTree(start, splitit-1, registredVariables, registredFunctions), buildTree(splitit+1, end, registredVariables, registredFunctions));
			}break;
			case 6:{
				if( splitit->str == "_")
					return new Atom::RangeResidue(buildTree(start, splitit-1, registredVariables, registredFunctions), buildTree(splitit+1, end, registredVariables, registredFunctions));
				else if( splitit->str == "~")
					return new Atom::RangeAtom(buildTree(start, splitit-1, registredVariables, registredFunctions), buildTree(splitit+1, end, registredVariables, registredFunctions));
			}break;
		}
	}
	qDebug()<<"NULLPTR";
	return nullptr;
}


void FilterAtomsListModel::addFilters(const QString& arguments){

	QStringList commads = arguments.split(";");
	for(const QString& commandUnclean: commads){
		QString command = commandUnclean.trimmed();

		QList<Token>  tokens;
		Token t = {"", -1 }; //0 = str, 1 = int, 2 = float

		//parse the imput into tokens
		for(QString::iterator it = command.begin(); it < command.end();  it++){
			switch (it->unicode()) {
				case ' ': case '\n': case '\t': case '?':{ continue; } break;
				case '|':case '&': { //type 3
					if(!t.str.isEmpty()){
						tokens.push_back(setTokenType(t));
						t = {"", -1 };
					}
					tokens.push_back({QString(*it), 3});
					t = {"", -1 };
				} break;
				case '<':case '>':case '=':case '!': { //type 4
					if(!t.str.isEmpty()){
						tokens.push_back(setTokenType(t));
						t = {"", -1 };
					}
					if(it < command.end()-1 && (*(it+1)) == '='){
						tokens.push_back({QString(*it)+'=', 4});
						it++;
					}else{
						tokens.push_back({QString(*it), 4});
					}
					t = {"", -1 };

				} break;
				case '-':{ //type 5
					if(t.str.isEmpty() && it < command.end()-1 && (*(it+1)) != ' ' && (*(it+1)) != '\n' && (*(it+1)) != '\t' && (*(it+1)) != '('){
						 t.str.push_back(*it);
					}else{
						if(!t.str.isEmpty()){
							tokens.push_back(setTokenType(t));
							t = {"", -1 };
						}
						tokens.push_back({QString(*it), 5});
					}
				} break;

				case '+':case '/':case '%': case '*':{ //type 5
					if(!t.str.isEmpty()){
						tokens.push_back(setTokenType(t));
						t = {"", -1 };
					}
					tokens.push_back({QString(*it), 5});
				} break;
				case '_': case '~':{ //type 6
					if(!t.str.isEmpty()){
						tokens.push_back(setTokenType(t));
						t = {"", -1 };
					}
					tokens.push_back({QString(*it), 6});
				} break;
				case '(': case ')':{ //type 7
					if(!t.str.isEmpty()){
						tokens.push_back(setTokenType(t));
						t = {"", -1 };
					}
					tokens.push_back({QString(*it), 7});
				} break;
				case ',':{ //type 8
					if(!t.str.isEmpty()){
						tokens.push_back(setTokenType(t));
						t = {"", -1 };
					}
					tokens.push_back({QString(*it), 8});
				} break;


				default: { t.str.push_back(*it); } break;
			}
		}//for loop end

		if(!t.str.isEmpty()) tokens.push_back(setTokenType(t));
		if(tokens.isEmpty()) continue;

		for(const Token& t : tokens)
			qDebug()<<__LINE__<<" Token: "<<t.str<<" , "<<t.type;
		//build tree
		Atom::FilterNode* node = buildTree(tokens.begin(), tokens.end()-1, m_registredVariables, m_registredFunctions);
		qDebug()<<"Testing command...";
		if(node){
			if(!node->isValid()){
				qDebug()<<__LINE__<<" Given filter is invalid: "<<command;
				delete node;
			}else{
				qDebug()<<__LINE__<<" Command accepted: "<<command<<" => "<<node->display();
				m_filters.push_back(node);
			}
		}else{
			qDebug()<<__LINE__<<" Error parsing: "<<command;
		}
	}
	updateRefrechTriggers(m_filters.getRefreshTriggers());
	calculateFilters();
	invalidateFilter();
	emit filterHasChanged();
}

void FilterAtomsListModel::removeFilter(int index){
	if(index < 0 || index >= m_filters.size())  return;

	m_filters.removeAt(index);
	updateRefrechTriggers(m_filters.getRefreshTriggers());
	calculateFilters();
	invalidateFilter();
	emit filterHasChanged();
}


void FilterAtomsListModel::disableTimeline(int index, bool value){
	if(index < 0 || index >= m_filters.size())  return;
	if(m_filters[index].timelineViewEnabled != value){
		m_filters[index].timelineViewEnabled = value;
		invalidateFilter();
		emit disableTimelineChanged();
	}
}
void FilterAtomsListModel::disableRenderView(int index, bool value){
	if(index < 0 || index >= m_filters.size())  return;
	if(m_filters[index].renderViewEnabled != value){
		m_filters[index].renderViewEnabled = value;
		emit disableRenderViewChanged();
	}
}

void FilterAtomsListModel::clipboard(int index){
	if(index < 0 || index >= m_filters.size())  return;
	QApplication::clipboard()->setText(m_filters[index]->display(), QClipboard::Clipboard);
}

void FilterAtomsListModel::setEnabled(bool enable){
	if(m_enabled != enable){
		m_enabled = enable;
		invalidateFilter();
		emit filterHasChanged();
	}
}

int FilterAtomsListModel::getRowFromSource(int sourceRow) const{
	qDebug()<<__LINE__<<": "<<sourceRow<<" -> "<<FilterAtomsListModel::mapFromSource(createIndex(sourceRow,0,nullptr)).row();
	return FilterAtomsListModel::mapFromSource(createIndex(sourceRow,0,nullptr)).row();
}

bool FilterAtomsListModel::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const{
	if(!m_enabled) return true;

	QModelIndex index0 = sourceModel()->index(sourceRow, 0, sourceParent);
	const bool isAtom = sourceModel()->data(index0, Atoms::isAtomRole).toBool();
	const int index = sourceModel()->data(index0, Atoms::ItemRowRole).toInt();
	//const QString& str = sourceModel()->data(index0, Qt::DisplayRole).toString();

	if(isAtom){
		if(m_resultsAtom.isEmpty()) return true;
		for( const ResultFilterItem& v : m_resultsAtom.value( index, QList<ResultFilterItem>() ))
			if(v.item->timelineViewEnabled && v.type() == QVariant::Bool && v.toBool()) return false;
	}else{
		if(m_resultsResidue.isEmpty()) return true;
		for( const ResultFilterItem& v : m_resultsResidue.value( index, QList<ResultFilterItem>() ))
			if(v.item->timelineViewEnabled && v.type() == QVariant::Bool && v.toBool()) return false;
	}
	return true;
}

void FilterAtomsListModel::updateRefrechTriggers(int triggers){
	//disconnect all
	disconnect(m_timeline, SIGNAL(anyTrackerChanged()), this, SLOT(refresh()));
	disconnect(m_timeline, SIGNAL(onStartFrameChanged()), this, SLOT(refresh()));
	disconnect(m_timeline, SIGNAL(onEndFrameChanged()), this, SLOT(refresh()));
	disconnect(m_data, SIGNAL(selectionChanged()), this, SLOT(refresh()));
	//reconnect
	if(triggers & FILTER_NODE_CONNECT_FRAME_CHANGE)
		connect(m_timeline, SIGNAL(anyTrackerChanged()), this, SLOT(refresh()));
	if(triggers & FILTER_NODE_CONNECT_STARTFRAME_CHANGE)
		connect(m_timeline, SIGNAL(onStartFrameChanged()), this, SLOT(refresh()));
	if(triggers & FILTER_NODE_CONNECT_ENDFRAME_CHANGE)
		connect(m_timeline, SIGNAL(onEndFrameChanged()), this, SLOT(refresh()));
	if(triggers & FILTER_NODE_CONNECT_ATOM_SELECTED_CHANGE)
		connect(m_data, SIGNAL(selectionChanged()), this, SLOT(refresh()));
}

void FilterAtomsListModel::calculateFilters(){
	Atom::Variables vars;
	m_resultsResidue.clear();
	m_resultsAtom.clear();
	//residue
	for(int i = 0; i < m_data->numberOfGroups(); i++){
		if(!m_resultsResidue.contains(i)) m_resultsResidue[i] = QList<ResultFilterItem>();
		for(const Atom::FilterItem& f: m_filters){
            const QVariant v = f->call(false, i+1, m_data->getGroupName(i+1), const_cast<FilterAtomsListModel*>(this), m_data, m_timeline, vars);
			if(v.isValid() || f.isLive){
				m_resultsResidue[i].push_back(ResultFilterItem(&f,v));
				//if(v.type() == QVariant::Bool && v.toBool()) break; // if its true then it will be filtered out, so the rest dons't interest us
			}
		}
	}

	//atoms
	for(int i = 0; i < m_data->numberOfAtroms(); i++){
		if(!m_resultsAtom.contains(i)) m_resultsAtom[i] = QList<ResultFilterItem>();
		for(const Atom::FilterItem& f: m_filters){
            const QVariant v = f->call(true, i+1, m_data->getAtomName(i), const_cast<FilterAtomsListModel*>(this), m_data, m_timeline, vars);
			if(v.isValid() || f.isLive){
				m_resultsAtom[i].push_back(ResultFilterItem(&f,v));
				//if(v.type() == QVariant::Bool && v.toBool()) break; // if its true then it will be filtered out, so the rest dons't interest us
			}
		}
	}

}

