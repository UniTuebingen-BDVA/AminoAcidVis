/*
 * Geometry.cpp
 *
 *  Created on: 20.03.2017
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

#include <Geometry.h>
#include <QDebug>
#include <QtAlgorithms>

namespace GL {


template<> unsigned int GeometryDataHolderType<int>::getElementsSize() const {return 1;};
template<> unsigned int GeometryDataHolderType<float>::getElementsSize() const {return 1;};
template<> unsigned int GeometryDataHolderType<double>::getElementsSize() const {return 1;};
template<> unsigned int GeometryDataHolderType<glm::ivec2>::getElementsSize() const {return 2;};
template<> unsigned int GeometryDataHolderType<glm::ivec3>::getElementsSize() const {return 3;};
template<> unsigned int GeometryDataHolderType<glm::ivec4>::getElementsSize() const {return 4;};
template<> unsigned int GeometryDataHolderType<glm::vec2>::getElementsSize() const {return 2;};
template<> unsigned int GeometryDataHolderType<glm::vec3>::getElementsSize() const {return 3;};
template<> unsigned int GeometryDataHolderType<glm::vec4>::getElementsSize() const {return 4;};

template<> unsigned int GeometryDataHolderType<glm::ivec2>::getType() const {return GL_INT;};
template<> unsigned int GeometryDataHolderType<glm::ivec3>::getType() const {return GL_INT;};
template<> unsigned int GeometryDataHolderType<glm::ivec4>::getType() const {return GL_INT;};


Geometry::Geometry(){}

Geometry::Geometry(const QVector<GeometryDataHolder*>& datas) {
	setData(datas);
}

bool Geometry::empty() const{
	return m_datas.empty();
}
bool Geometry::isValid() const{
	return !m_datas.empty();
}
Geometry::operator bool() const{
	return isValid();
}

unsigned int Geometry::size() const{
	return m_datas.size();
}


void Geometry::setData(const QVector<GeometryDataHolder*>& datas){
	clear();
	m_datas.reserve(datas.size());
	for(GeometryDataHolder* d: datas){
		Q_CHECK_PTR(d);
		if(d->getDrawTarget() == GL_ELEMENT_ARRAY_BUFFER){
			if(m_indices) delete m_indices;
			m_indices = d;
		}else
			m_datas.push_back(d);
	}
}

void Geometry::push_back(GeometryDataHolder* data){
	Q_CHECK_PTR(data);
	if(data->getDrawTarget() != GL_ELEMENT_ARRAY_BUFFER)
		m_datas.push_back(data);
	else{
		qDebug()<<"["<<__LINE__<<"] Given geometry data can't have target 'GL_ELEMENT_ARRAY_BUFFER'! Data deleted!";
		delete data;
	}
}

void Geometry::setIndices(GeometryDataHolder* indices){
	Q_CHECK_PTR(indices);
	if(indices && indices->getDrawTarget() == GL_ELEMENT_ARRAY_BUFFER)
		m_indices = indices;
	else{
		qDebug()<<"["<<__LINE__<<"] Given indices data must have target 'GL_ELEMENT_ARRAY_BUFFER'! Data deleted!";
		delete indices;
	}
}
void Geometry::setIndices(const QVector<int>& indices){
	setIndices(new GL::GeometryListDataHolder<int>(indices, GL_STATIC_DRAW, GL_ELEMENT_ARRAY_BUFFER));
}

const QVector<GeometryDataHolder*>& Geometry::getData() const{
	return m_datas;
}

const GeometryDataHolder* Geometry::getIndices() const{
	return m_indices;
}

bool Geometry::hasIndices() const{
	return m_indices;
}

void Geometry::clear(){
	for(GeometryDataHolder* d: m_datas)
		delete d;
	m_datas.clear();
	if(m_indices) delete m_indices;
	m_indices = nullptr;
}

Geometry::~Geometry() {
	for(GeometryDataHolder* d: m_datas)
		delete d;
	if(m_indices) delete m_indices;
}

#include <cmath>
#define M_PI 3.14159265358979323846
#define M_PI_2 1.57079632679489661923
#define M_PI_4 0.785398163397448309616


Geometry Geometry::plane(float width, float lenght, bool color ){
	const float hw = width/2.f;
	const float hl = lenght/2.f;

	if(color)
	return Geometry(
				{
					new GL::GeometryListDataHolder<glm::vec4>({glm::vec4(-hw,0,-hl,1), glm::vec4(hw,0,-hl,1), glm::vec4(hw,0,hl,1), glm::vec4(-hw,0,hl,1)}),
					new GL::GeometryListDataHolder<glm::vec3>({glm::vec3(0,1,0), glm::vec3(0,1,0), glm::vec3(0,1,0), glm::vec3(0,1,0)}),
					new GL::GeometryListDataHolder<glm::vec2>({glm::vec2(0,0), glm::vec2(1,0), glm::vec2(1,1), glm::vec2(0,1)}),
					new GL::GeometryListDataHolder<int>({ 0, 1, 2, 0, 2, 3}, GL_STATIC_DRAW, GL_ELEMENT_ARRAY_BUFFER),
				}
		);
	else
	return Geometry(
				{
					new GL::GeometryListDataHolder<glm::vec4>({glm::vec4(-hw,0,-hl,1), glm::vec4(hw,0,-hl,1), glm::vec4(hw,0,hl,1), glm::vec4(-hw,0,hl,1)}),
					new GL::GeometryListDataHolder<glm::vec2>({glm::vec2(0,0), glm::vec2(1,0), glm::vec2(1,1), glm::vec2(0,1)}),
					new GL::GeometryListDataHolder<int>({ 0, 1, 2, 0, 2, 3}, GL_STATIC_DRAW, GL_ELEMENT_ARRAY_BUFFER),
				}
		);
}

Geometry Geometry::sfq(){
	return Geometry(
			{
				new GL::GeometryListDataHolder<glm::vec4>({glm::vec4(-1,-1,0,1), glm::vec4(1,-1,0,1), glm::vec4(1,1,0,1), glm::vec4(-1,1,0,1)}),
				new GL::GeometryListDataHolder<glm::vec2>({glm::vec2(0,0), glm::vec2(1,0), glm::vec2(1,1), glm::vec2(0,1)}),
				new GL::GeometryListDataHolder<int>({ 0, 1, 2, 0, 2, 3}, GL_STATIC_DRAW, GL_ELEMENT_ARRAY_BUFFER),
			}
	);
}

Geometry Geometry::sphere(float radius, unsigned int rings, unsigned int sectors){
	float const R = 1./(float)(rings-1);
	float const S = 1./(float)(sectors-1);
	unsigned int r, s;

	GeometryListDataHolder<glm::vec4>* vertices = new GeometryListDataHolder<glm::vec4>();
	GeometryListDataHolder<glm::vec3>* normals = new GeometryListDataHolder<glm::vec3>();
	GeometryListDataHolder<glm::vec2>* uvs = new GeometryListDataHolder<glm::vec2>();
	GeometryListDataHolder<int>* indices = new GeometryListDataHolder<int>(GL_STATIC_DRAW, GL_ELEMENT_ARRAY_BUFFER);

	vertices->data.resize(rings * sectors );
	normals->data.resize(rings * sectors );
	uvs->data.resize(rings * sectors );
	indices->data.resize(rings * sectors * 4);

    auto v = vertices->data.begin();
    auto n = normals->data.begin();
    auto t = uvs->data.begin();
    auto i = indices->data.begin();

    for(r = 0; r < rings; r++)
    	for(s = 0; s < sectors; s++) {
            float const y = sin( -M_PI_2 + M_PI * r * R );
            float const x = cos(2*M_PI * s * S) * sin( M_PI * r * R );
            float const z = sin(2*M_PI * s * S) * sin( M_PI * r * R );

            *v++ = glm::vec4(x * radius,y * radius,z * radius, 1);
            *n++ = glm::vec3(x,y,z);
            *t++ = glm::vec2(s*S,r*R);

            //indeces
            if(r < rings-1){
				int curRow = r * sectors;
				int nextRow = (r+1) * sectors;
				int nextS = (s+1) % sectors;

				*i++ = curRow + s;
				*i++ = nextRow + s;
				*i++ = nextRow + nextS;

				*i++ = curRow + s;
				*i++ = nextRow + nextS;
				*i++ = curRow + nextS;
            }
    }


    return  Geometry({vertices,normals,uvs,indices});
}


Geometry Geometry::flatBonedCylinder(unsigned int sectors, unsigned int slices){
	GeometryListDataHolder<glm::vec3>* vertices = new GeometryListDataHolder<glm::vec3>(slices*sectors,GL_STATIC_DRAW, GL_ARRAY_BUFFER);
	GeometryListDataHolder<int>* ids = new GeometryListDataHolder<int>(slices*sectors,GL_STATIC_DRAW, GL_ARRAY_BUFFER);
	GeometryListDataHolder<int>* indices = new GeometryListDataHolder<int>(GL_STATIC_DRAW, GL_ELEMENT_ARRAY_BUFFER);

	QVector<glm::vec3> pos;

	for(unsigned int i=0; i<sectors; i++) {
		const float theta = (((float)i)*2.0*M_PI)/sectors;
		pos.push_back(glm::vec3(cos(theta),0,sin(theta)));
	}

	unsigned int current = 0;
	vertices->data += pos;

	for(unsigned int i=0; i<sectors; i++) ids->push_back(0);

	for(unsigned int s = 0; s < slices; s++){
		vertices->data += pos;

		const unsigned int next = current+sectors;
		for(unsigned int i=0; i<sectors-1; i++) {
			ids->push_back(s+1);

			indices->push_back(current+i);
			indices->push_back(current+i+1);
			indices->push_back(next+i);

			indices->push_back(current+i+1);
			indices->push_back(next+i+1);
			indices->push_back(next+i);
		}
		//connect last with front
		ids->push_back(s+1);

		indices->push_back(current+sectors-1);
		indices->push_back(current);
		indices->push_back(next);

		indices->push_back(current+sectors-1);
		indices->push_back(next+sectors-1);
		indices->push_back(next);

		current += sectors;

	}

	return  Geometry({vertices,ids,indices});
}

Geometry Geometry::cube(float size){
	const float hs = size/2.f;
	return Geometry(
					{
						new GL::GeometryListDataHolder<glm::vec4>({
							//y+
							glm::vec4(-hs,hs,-hs,1), glm::vec4(hs,hs,-hs,1), glm::vec4(hs,hs,hs,1),
							glm::vec4(-hs,hs,-hs,1), glm::vec4(hs,hs,hs,1), glm::vec4(-hs,hs,hs,1),
							//y-
							glm::vec4(-hs,-hs,-hs,1), glm::vec4(hs,-hs,hs,1), glm::vec4(hs,-hs,-hs,1),
							glm::vec4(-hs,-hs,-hs,1), glm::vec4(-hs,-hs,hs,1), glm::vec4(hs,-hs,hs,1),

							//z+
							glm::vec4(-hs,-hs,hs,1), glm::vec4(hs,-hs,hs,1), glm::vec4(hs,hs,hs,1),
							glm::vec4(-hs,-hs,hs,1), glm::vec4(hs,hs,hs,1), glm::vec4(-hs,hs,hs,1),
							//z-
							glm::vec4(-hs,-hs,-hs,1), glm::vec4(hs,hs,-hs,1), glm::vec4(hs,-hs,-hs,1),
							glm::vec4(-hs,-hs,-hs,1), glm::vec4(-hs,hs,-hs,1), glm::vec4(hs,hs,-hs,1),
						}),


						new GL::GeometryListDataHolder<glm::vec3>({
							//y+
							glm::vec3(0,1,0), glm::vec3(0,1,0), glm::vec3(0,1,0),
							glm::vec3(0,1,0), glm::vec3(0,1,0), glm::vec3(0,1,0),

							//y-
							glm::vec3(0,-1,0), glm::vec3(0,-1,0), glm::vec3(0,-1,0),
							glm::vec3(0,-1,0), glm::vec3(0,-1,0), glm::vec3(0,-1,0),

							//z+
							glm::vec3(0,0,1), glm::vec3(0,0,1), glm::vec3(0,0,1),
							glm::vec3(0,0,1), glm::vec3(0,0,1), glm::vec3(0,0,1),

							//y-
							glm::vec3(0,0,-1), glm::vec3(0,0,-1), glm::vec3(0,0,-1),
							glm::vec3(0,0,-1), glm::vec3(0,0,-1), glm::vec3(0,0,-1),
						}),

						new GL::GeometryListDataHolder<glm::vec2>({
							//y+
							glm::vec2(0,0), glm::vec2(1,0), glm::vec2(1,1),
							glm::vec2(0,0), glm::vec2(1,1), glm::vec2(0,1),

							//y-
							glm::vec2(0,0), glm::vec2(1,1), glm::vec2(1,0),
							glm::vec2(0,0), glm::vec2(0,1), glm::vec2(1,1),

							//z+
							glm::vec2(0,0), glm::vec2(1,0), glm::vec2(1,1),
							glm::vec2(0,0), glm::vec2(1,1), glm::vec2(0,1),

							//z-
							glm::vec2(0,0), glm::vec2(1,1), glm::vec2(1,0),
							glm::vec2(0,0), glm::vec2(0,1), glm::vec2(1,1),
						})
					}
			);
}

} /* namespace GL */
