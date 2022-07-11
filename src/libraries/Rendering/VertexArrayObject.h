/**
 * @file   		VertexArrayObject.h
 * @author 		Vladimir Ageev (vladimir.agueev@progsys.de)
 * @date   		20.03.2017
 *
 * @brief  		Class that holds a vertex array object. Mainly used for inheritance.
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


#ifndef VERTEX_ARRAY_OBJECT_H
#define VERTEX_ARRAY_OBJECT_H

#include <Rendering/Object.h>

namespace GL{
class VertexArrayObject: public Object {
public:
	VertexArrayObject(GLenum mode = GL_TRIANGLES);

	VertexArrayObject* setMode(GLenum mode);
	GLenum getMode() const;

    virtual void draw() const = 0;
    virtual void draw(GLenum force_mode) const = 0;

    virtual ~VertexArrayObject();
protected:
	GLenum mode = 0;
};

} /*namespace GL*/

#endif // VERTEX_ARRAY_OBJECT_H
