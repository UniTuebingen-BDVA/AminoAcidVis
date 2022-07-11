/*
 * Object.h
 *
 *  Created on: 05.06.2017
 *      Author: Vladimir Ageev (vladimir.agueev@progsys.de
 */

#ifndef LIBRARIES_RENDERING_OBJECT_H_
#define LIBRARIES_RENDERING_OBJECT_H_

#include <Rendering/OpenGL.h>

namespace GL {

class Object {
public:
	Object();
	GLuint getHandle() const;
	virtual ~Object();
protected:
	GLuint m_handle = GL_INVALID_VALUE;
};

} /* namespace GL */

#endif /* LIBRARIES_RENDERING_OBJECT_H_ */
