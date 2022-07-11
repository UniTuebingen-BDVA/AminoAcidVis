/*
 * Mesh.cpp
 *
 *  Created on: 16.03.2017
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

#include <Rendering/Mesh.h>
#include <Rendering/GLError.h>

#include <QDebug>

namespace GL {


Mesh::Mesh(const Geometry& geo, GLenum mode): Mesh(geo, std::function<void()>(), mode){}

Mesh::Mesh(const Geometry& geo, const std::function<void()>& drawSetting, GLenum mode): VertexArrayObject(mode) {
	this->drawSetting = drawSetting;
	if(!geo) return;



	glGenVertexArrays(1, &m_handle);
	glBindVertexArray(m_handle);

	//glGenBuffers
	for(GeometryDataHolder* data: geo.getData())
		if(!data->getHandle()) VBO.push_back(new GL::BufferObject());
	if(geo.hasIndices()) VBO.push_back(new GL::BufferObject());

	//glGenBuffers(geo.size()+1, &VBO.front());
	checkGLError();

	auto itVBO = VBO.begin();
	for(unsigned int i = 0; i < geo.size(); i++){
		GeometryDataHolder* data = geo.getData()[i];
		if(data->getHandle()){ // points to an already existing buffer
			glBindBuffer(data->getDrawTarget(), data->getHandle());
			glVertexAttribPointer(i, data->getElementsSize(), data->getType(), GL_FALSE, data->getStride(), 0);
			glEnableVertexAttribArray (i);
		}else{
			(*itVBO)->bindData(data);
			checkGLError();
			itVBO++;
			glVertexAttribPointer(i, data->getElementsSize(), data->getType(), GL_FALSE, data->getStride(), 0);
			checkGLError();
			glEnableVertexAttribArray (i);
		}
		if(checkGLError()) qDebug()<<"Error at data "<<QString::number(i);
	}
	//copy indices
	if(geo.hasIndices()){
		m_elementsDraw = true;
		//copy indices
		draw_size = geo.getIndices()->size();
		(*itVBO)->bindData(geo.getIndices());
		glVertexAttribPointer(geo.size(), geo.getIndices()->getElementsSize(), geo.getIndices()->getType(), GL_FALSE, geo.getIndices()->getStride(), 0);
		glEnableVertexAttribArray (geo.size());
		if(checkGLError()) qDebug()<<"Error at indices data! ";
	}else
		draw_size = geo.getData()[0]->size();

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}

void Mesh::draw() const{;
	glBindVertexArray(m_handle);
	if(drawSetting) drawSetting();
	if(m_elementsDraw) glDrawElements(mode, size(), GL_UNSIGNED_INT, NULL);
	else glDrawArrays(mode, 0, size());
	glBindVertexArray(0);
}
void Mesh::draw(GLenum force_mode) const{
	glBindVertexArray(m_handle);
	if(drawSetting) drawSetting();
	if(m_elementsDraw) glDrawElements(force_mode, size(), GL_UNSIGNED_INT, NULL);
	else glDrawArrays(force_mode, 0, size());
	glBindVertexArray(0);
}

unsigned int Mesh::size() const{
	return draw_size;
}

unsigned int Mesh::elementsSize() const{
	switch (mode) {
		case GL_POINTS: return size();
		case GL_LINES: return size()/2;
		case GL_TRIANGLES: return size()/3;
		default: return size();
	}
}

Mesh::~Mesh() {
	if(m_handle)glDeleteVertexArrays(1, &m_handle );
	for(GL::BufferObject* bo: VBO)
		delete bo;
}

} /* namespace GL */
