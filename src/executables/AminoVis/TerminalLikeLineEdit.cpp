/*
 * TerminalLikeLineEdit.cpp
 *
 *  Created on: 12.08.2017
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

#include <TerminalLikeLineEdit.h>
#include <QKeyEvent>
#include <QDebug>

TerminalLikeLineEdit::TerminalLikeLineEdit(QWidget* parent):QLineEdit(parent) {
	// TODO Auto-generated constructor stub

}

TerminalLikeLineEdit::~TerminalLikeLineEdit() {
	// TODO Auto-generated destructor stub
}

void TerminalLikeLineEdit::submit(){
	if(m_lastEntries.size() > 100) m_lastEntries.removeFirst();
	m_index = m_lastEntries.size()-1;
	if(!text().isEmpty()){
		if(m_lastEntries.isEmpty() || m_lastEntries.last() != text())
			m_lastEntries.push_back(text());
	}
}

void TerminalLikeLineEdit::keyPressEvent(QKeyEvent *event){
	if(m_index < 0) m_index = m_lastEntries.size()-1;
	else if(m_index >= m_lastEntries.size()) m_index = 0;
	if(!m_lastEntries.isEmpty()){
	    if(event->key() == Qt::Key_Up){
	        setText(m_lastEntries[m_index]);
	    	m_index--;
	    	return;
	    }else if(event->key() == Qt::Key_Down){
	        setText(m_lastEntries[m_index]);
	    	m_index++;
	    	return;
	    }
	}

    if(event->key() == Qt::Key_Return)
    	submit();

    QLineEdit::keyPressEvent(event);

}

