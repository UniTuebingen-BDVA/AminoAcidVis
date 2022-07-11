/**
 * @file   		Mesh.h
 * @author 		Vladimir Ageev (vladimir.agueev@progsys.de)
 * @date   		Mar 16, 2017
 *
 * @brief  		Contains the Mesh class, which makes it easy to create and manage VBO geometry.
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

#ifndef LIBRARIES_RENDERING_MESH_H_
#define LIBRARIES_RENDERING_MESH_H_

#include <Rendering/VertexArrayObject.h>
#include <Rendering/Geometry.h>
#include <Rendering/BufferObject.h>

#include <glm/glm.hpp>
#include <QList>
#include <QVector>
#include <functional>

namespace GL {

/*!
 * @brief This class creates and manages a VertexArrayObject.
 */
class Mesh: public VertexArrayObject {
	bool m_elementsDraw = false;
public:
	/*!
	 * @brief Creates a VertexArrayObject with the given geometry.
	 * @param geo The geometry that will be used to create the VertexArrayObject. Indexes must be set.
	 * @param mode Specifies with what kind of primitives the geometry will be rendered. For example GL_TRIANGLES.
	 */
	Mesh(const Geometry& geo, GLenum mode);
	/*!
	 * @brief Creates a VertexArrayObject with the given geometry.
	 * @param geo The geometry that will be used to create the VertexArrayObject. Indexes must be set.
	 * @param drawSetting A extra settings functions that, if defined, will be called before glDrawElements.
	 * @param mode Specifies with what kind of primitives the geometry will be rendered. For example GL_TRIANGLES.
	 */
	Mesh(const Geometry& geo, const std::function<void()>& drawSetting = std::function<void()>(), GLenum mode = GL_TRIANGLES);

	QVector<GL::BufferObject*> VBO;

	unsigned int draw_size = 0;
	///A extra settings functions that, if defined, will be called before glDrawElements.
	std::function<void()> drawSetting;

	///Draw the VertexArrayObject
    virtual void draw() const;
    virtual void draw(GLenum force_mode) const;

    virtual unsigned int size() const;
    ///@returns the number of primitives
    virtual unsigned int elementsSize() const;

    template<typename T>
    void updateBuffer(unsigned int bufferID, unsigned int offset,const T& value){
    	glBindVertexArray(getHandle());
    	VBO[bufferID]->updateBuffer<T>(offset, value);
    	//lBindBuffer( GL_ARRAY_BUFFER, VBO[bufferID] );
    	//glBufferSubData(GL_ARRAY_BUFFER, offset*sizeof(T), sizeof(T), &value);
    }

    template<typename T>
    void updateBuffer(unsigned int bufferID, unsigned int offset, unsigned int size,const void* data){
    	glBindVertexArray(getHandle());
    	VBO[bufferID]->updateBuffer<T>(offset, size, data);
    	//glBindBuffer( GL_ARRAY_BUFFER, VBO[bufferID] );
    	//glBufferSubData(GL_ARRAY_BUFFER, offset*sizeof(T), size*sizeof(T), data);
    }

	virtual ~Mesh();
};

} /* namespace GL */

#endif /* LIBRARIES_RENDERING_MESH_H_ */
