/**
 * @file   		Geometry.h
 * @author 		Vladimir Ageev (vladimir.agueev@progsys.de)
 * @date   		Mar 20, 2017
 *
 * @brief  		A class to manage the different types of geometry and it's indices.
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

#ifndef LIBRARIES_RENDERING_GEOMETRY_H_
#define LIBRARIES_RENDERING_GEOMETRY_H_

#include <QVector>
#include <Rendering/OpenGL.h>
#include <glm/glm.hpp>
#include <Rendering/Object.h>

namespace GL {

/*!
 * @brief A interface class to prototype all of the needed public methods to buffer the data for GL use.
 */
class GeometryDataHolder{
public:
	GeometryDataHolder(GLenum drawUsage = GL_STATIC_DRAW, GLenum drawTarget = GL_ARRAY_BUFFER):
		p_drawUsage(drawUsage),p_drawTarget(drawTarget){};

	virtual ~GeometryDataHolder(){};

	///@returns the number of items inside the data.
	virtual unsigned int size() const = 0;
	///@returns the size in bytes of one item in the data.
	virtual unsigned int getElementsSize() const = 0;
	///@returns the stride offset of the data.
	virtual unsigned int getStride() const = 0;
	///@returns the total byte size of the data.
	virtual unsigned int getTotalSize() const = 0;
	///@returns the type of the item. Like GL_FLOAT or GL_INT.
	virtual GLenum getType() const = 0;
	///@returns the starting adders of the data.
	virtual const void* getPtr() const = 0;

	///@returns the Buffer Object handle of a already existing BO, if one is held.
	virtual GLuint getHandle() const {return 0;};

	///@returns a hint on how the data will be used or how often it will be updated.
	virtual GLenum getDrawUsage() const {return p_drawUsage; };
	///@returns the type of the bugger.
	virtual GLenum getDrawTarget() const {return p_drawTarget; };

	GLenum p_drawUsage = GL_STATIC_DRAW;
	GLenum p_drawTarget = GL_ARRAY_BUFFER;
};

template<typename T>
class GeometryDataHolderType: public GeometryDataHolder{
public:
	QVector<T> data;
	GeometryDataHolderType(): GeometryDataHolder(GL_STATIC_DRAW, GL_ARRAY_BUFFER) {};
	GeometryDataHolderType(GLenum drawType, GLenum drawTarget): GeometryDataHolder(drawType, drawTarget) {};
	virtual ~GeometryDataHolderType(){};

	virtual unsigned int getElementsSize() const {return 1;};
	virtual unsigned int getStride() const { return sizeof(T); };
	virtual GLenum getType() const {return GL_FLOAT;};
};

template<> unsigned int GeometryDataHolderType<int>::getElementsSize() const;
template<> unsigned int GeometryDataHolderType<float>::getElementsSize() const;
template<> unsigned int GeometryDataHolderType<double>::getElementsSize() const;
template<> unsigned int GeometryDataHolderType<glm::ivec2>::getElementsSize() const;
template<> unsigned int GeometryDataHolderType<glm::ivec3>::getElementsSize() const;
template<> unsigned int GeometryDataHolderType<glm::ivec4>::getElementsSize() const;
template<> unsigned int GeometryDataHolderType<glm::vec2>::getElementsSize() const;
template<> unsigned int GeometryDataHolderType<glm::vec3>::getElementsSize() const;
template<> unsigned int GeometryDataHolderType<glm::vec4>::getElementsSize() const;

template<> unsigned int GeometryDataHolderType<glm::ivec2>::getType() const;
template<> unsigned int GeometryDataHolderType<glm::ivec3>::getType() const;
template<> unsigned int GeometryDataHolderType<glm::ivec4>::getType() const;

template<typename T>
class GeometryListDataHolder: public GeometryDataHolderType<T>{
public:
	QVector<T> data;
	GeometryListDataHolder(): GeometryDataHolderType<T>(GL_STATIC_DRAW, GL_ARRAY_BUFFER) {};
	GeometryListDataHolder(GLenum drawType, GLenum drawTarget): GeometryDataHolderType<T>(drawType, drawTarget) {};
	GeometryListDataHolder(unsigned int reserve, GLenum drawType, GLenum drawTarget): GeometryDataHolderType<T>(drawType, drawTarget) {data.reserve(reserve);};
	GeometryListDataHolder(const QVector<T>& dataIn ,GLenum drawType = GL_STATIC_DRAW, GLenum drawTarget = GL_ARRAY_BUFFER): GeometryDataHolderType<T>(drawType, drawTarget),data(dataIn) {};
	virtual ~GeometryListDataHolder(){};

	virtual unsigned int size() const { return data.size(); };
	virtual unsigned int getTotalSize() const { return GeometryDataHolderType<T>::getStride()*size();};
	virtual const void* getPtr() const {return &data.first();};

	inline void push_back(const T& item){ data.push_back(item);};
};

template<typename T>
class GeometryReferenceDataHolder: public GeometryDataHolderType<T>{
public:
	const void* p_data;
	unsigned int p_size;
	GeometryReferenceDataHolder(void* data, unsigned int size ): GeometryDataHolderType<T>(GL_STATIC_DRAW, GL_ARRAY_BUFFER), p_data(data), p_size(size)  {};
	GeometryReferenceDataHolder(const void* data, unsigned int size, GLenum drawType, GLenum drawTarget): GeometryDataHolderType<T>(drawType, drawTarget), p_data(data), p_size(size) {};
	virtual ~GeometryReferenceDataHolder(){};

	virtual unsigned int size() const { return p_size; };
	virtual unsigned int getTotalSize() const { return GeometryDataHolderType<T>::getStride()*p_size;};
	virtual const void* getPtr() const {return p_data;};
};


class BufferObject;

template<typename T>
class GeometryBufferReferenceHolder: public GeometryDataHolderType<T>{
public:
	GLuint p_handle = 0;
	GeometryBufferReferenceHolder(GLuint handle ): GeometryDataHolderType<T>(), p_handle(handle) {};
	GeometryBufferReferenceHolder(const GL::Object* buffer ): GeometryDataHolderType<T>(), p_handle(buffer->getHandle()) {};

	virtual ~GeometryBufferReferenceHolder(){};

	virtual unsigned int size() const { return 0; };
	virtual unsigned int getTotalSize() const { return 0;};
	virtual const void* getPtr() const {return nullptr;};

	virtual GLuint getHandle() const {return p_handle;};
};


/*!
 * @brief A class to manage the different types of geometry and it's indices.
 * The location is defined by the order of the data.
 */
class Geometry {
public:
	Geometry();
	/*!
	 * @brief Constructor to easy set the geometry via a vector.
	 * @param A vector with GeometryDataHolder pointers.
	 * @code
	    //This will create a plane (vertex, normal, uv, indices)
	  	GL::Geometry geo(
			{
				new GL::GeometryListDataHolder<glm::vec4>({glm::vec4(-2,0,-2,1), glm::vec4(2,0,-2,1), glm::vec4(2,0,2,1), glm::vec4(-2,0,2,1)}),
				new GL::GeometryListDataHolder<glm::vec3>({glm::vec3(0,1,0), glm::vec3(0,1,0), glm::vec3(0,1,0), glm::vec3(0,1,0)}),
				new GL::GeometryListDataHolder<glm::vec2>({glm::vec2(0,0), glm::vec2(1,0), glm::vec2(1,1), glm::vec2(0,1)}),
				new GL::GeometryListDataHolder<int>({ 0, 1, 2, 0, 2, 3}, GL_STATIC_DRAW, GL_ELEMENT_ARRAY_BUFFER),
			}
		);
		m_geometry = new GL::Mesh(geo);
	 * @codeend
	 * @see GeometryDataHolder
	 */
	Geometry(const QVector<GeometryDataHolder*>& datas);
	///@returns true if no data is stored.
	bool empty() const;
	///The geometry is valid if data is not empty and the indices are set.
	bool isValid() const;
	operator bool() const;
	///@returns the number of data's stored.
	unsigned int size() const;

	/*!
	 * @brief Sets all of the data via a vector.
	 * If a data has the 'drawTarget' set to 'GL_ELEMENT_ARRAY_BUFFER', then it will be used as indices.
	 * Note: All of the previous data will be cleared.
	 * @param A vector with GeometryDataHolder pointers.
	 * @see GeometryDataHolder
	 */
	void setData(const QVector<GeometryDataHolder*>& datas);

	void push_back(GeometryDataHolder* data);
	/*!
	 * @brief Add geometry data, like vertices, normals, uvs, ...
	 * @param data A QVector with the data.
	 */
	template<typename T>
	void push_back(const QVector<T>& data);

	/*!
	 * @brief Set the indices. Note: Previous indices will be deleted.
	 * @param indices A pointer to a GeometryDataHolder.
	 * Note: 'drawTarget' must be set to 'GL_ELEMENT_ARRAY_BUFFER' else given pointer will be deleted.
	 * @see GeometryDataHolder
	 */
	void setIndices(GeometryDataHolder* indices);
	void setIndices(const QVector<int>& indices);

	const QVector<GeometryDataHolder*>& getData() const;
	const GeometryDataHolder* getIndices() const;
	bool hasIndices() const;

	///Deletes all the data and indices.
	void clear();

	static Geometry plane(float width, float lenght, bool color = false);
	static Geometry sfq();
	static Geometry sphere(float radius, unsigned int rings, unsigned int sectors);
	static Geometry flatBonedCylinder(unsigned int sectors, unsigned int slices);
	static Geometry cube(float size);

	virtual ~Geometry();
private:
	QVector<GeometryDataHolder*> m_datas;
	GeometryDataHolder* m_indices = nullptr;
};

template<typename T>
void Geometry::push_back(const QVector<T>& data){
	push_back(new GeometryListDataHolder<T>(data));
}

} /* namespace GL */

#endif /* LIBRARIES_RENDERING_GEOMETRY_H_ */
