/**
 * @file   		BucketGrid.h
 * @author 		Vladimir Ageev (vladimir.agueev@progsys.de)
 * @date   		04.05.2017
 *
 * @brief  		Class that can store data in a uniform 3D grid.
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

#ifndef LIBRARIES_ATOMS_BUCKETGRID_H_
#define LIBRARIES_ATOMS_BUCKETGRID_H_

#include <QVector>

#include <Util/AABB.h>
#include <QDebug>
#define QT_FORCE_ASSERTS true

template<typename T>
class BucketGrid {
public:
	BucketGrid(const aabb& box, float radius): m_box(box), m_blockSize(radius){
		//aabb must be valid
		Q_ASSERT(m_box.max != m_box.min);
		Q_ASSERT(m_box.max.x > m_box.min.x && m_box.max.y > m_box.min.y && m_box.max.z > m_box.min.z);
		//make aabb a bit bigger so all points are inside it and not in the boundary
		m_box.max+=0.01f;
		m_box.min-=0.01f;
		m_size = glm::uvec3(1,1,1) + glm::uvec3((m_box.max-m_box.min)/m_blockSize );

		//Allocate memory
		m_data = new QVector<T> **[m_size.x]();
	    for (int i = 0; i < m_size.x; i++)
	    {
	    	m_data[i] = new QVector<T> *[m_size.y]();
	        for (int j = 0; j < m_size.y; j++)
	        	m_data[i][j] = new QVector<T> [m_size.z]();
	    }

	}

	unsigned int getWidth() const {return m_size.x;}
	unsigned int getHeight() const {return m_size.y;}
	unsigned int getLength() const {return m_size.z;}
	const glm::uvec3& getSize() const {return m_size;}

	const QVector<T>& get(const glm::vec3& pos){
		Q_ASSERT(pos.x < m_box.max.x || pos.y < m_box.max.y || pos.z < m_box.max.z);
		Q_ASSERT(pos.x > m_box.min.x || pos.y > m_box.min.y || pos.z > m_box.min.z);
		const glm::uvec3 gridpos(
				((pos.x - m_box.min.x)/m_blockSize),
				((pos.y - m_box.min.y)/m_blockSize),
				((pos.z - m_box.min.z)/m_blockSize)

		);
		return get( gridpos );
	}

	const QVector<T>& get(const glm::uvec3& pos){
		return m_data[pos.x][pos.y][pos.z];
	}

	const QVector<T>& get(unsigned int x, unsigned int y, unsigned int z){
		return m_data[x][y][z];
	}

	void insert(const T& data, const glm::vec3& pos){
		Q_ASSERT(pos.x < m_box.max.x || pos.y < m_box.max.y || pos.z < m_box.max.z);
		Q_ASSERT(pos.x > m_box.min.x || pos.y > m_box.min.y || pos.z > m_box.min.z);
		const glm::uvec3 gridpos(
				((pos.x - m_box.min.x)/m_blockSize),
				((pos.y - m_box.min.y)/m_blockSize),
				((pos.z - m_box.min.z)/m_blockSize)

		);
		//qDebug()<<"insert1 "<<data<<" to "<<gridpos<<" from "<<pos<< " min "<<m_box.min<<" max "<<m_box.max<<" size "<<m_size<<" blockSize "<<m_blockSize;
		insert(data, gridpos );
	}

	void insert(const T& data, const glm::uvec3& pos){
		//qDebug()<<"insert2 "<<data<<" to "<<pos;
		m_data[pos.x][pos.y][pos.z].push_back(data);
	}

	void insert(const T& data, unsigned int x, unsigned int y, unsigned int z){
		m_data[x][y][z].push_back(data);
	}

	bool getSurroundings(const glm::vec3& pos, QVector<const QVector<T>*>& out, int radius = 1){
		const glm::uvec3 gridpos(
				((pos.x - m_box.min.x)/m_blockSize),
				((pos.y - m_box.min.y)/m_blockSize),
				((pos.z - m_box.min.z)/m_blockSize)

		);
		return getSurroundings( gridpos , out, radius);
	}

	inline void get(QVector<const QVector<T>*>& out, int x, int y, int z){
		if(x >= 0 && x < m_size.x && y >= 0 && y < m_size.y && z >= 0 && z < m_size.z){
			//qDebug()<<__LINE__<<": Buket Get: "<<"("<<x<<", "<<y<<", "<<z<<") = "<<m_data[x][y][z];
			const QVector<T>& v = m_data[x][y][z];
			if(!v.empty())  out.push_back(&v);
		}
	}

	bool getSurroundings(const glm::uvec3& pos, QVector<const QVector<T>*>& out, int radius = 1){
		if(radius <= 0){
			get( out, pos.x, pos.y, pos.z );
			return true;
		}
		const int xLimits[2] = {(int)pos.x - radius ,(int)pos.x + radius};
		const int yLimits[2] = {(int)pos.y - radius ,(int)pos.y + radius};
		const int zLimits[2] = {(int)pos.z - radius ,(int)pos.z + radius};
		if(xLimits[0] < 0 && xLimits[1] >= m_size.x && yLimits[0] < 0 && yLimits[1] >= m_size.y && zLimits[0] < 0 && zLimits[1] >= m_size.z)
			return false;
		//Boundary
		for(int y = yLimits[0]; y <= yLimits[1]; y++)
			for(int z = zLimits[0]; z <= zLimits[1]; z++){
				get( out, xLimits[0], y, z  );
				get( out, xLimits[1], y, z  );
			}

		//Inbetween
		for(int x = xLimits[0]+1; x <= xLimits[1]-1; x++){
			//Boundary
			for(int z = zLimits[0]; z <= zLimits[1]; z++){
				get( out, x, yLimits[0], z  );
				get( out, x, yLimits[1], z  );
			}
			//Inbetween
			for(int y = yLimits[0]+1; y <= yLimits[1]-1; y++){
				get( out, x, y, zLimits[0]  );
				get( out, x, y, zLimits[1]  );
			}
		}

		return true;
	}

	void clear(){
		for(int x = 0; x < m_size.x; x++)
			for(int y = 0; y < m_size.y; y++)
				for(int z = 0; z < m_size.z; z++)
					m_data[x][y][z].clear();
	}

	virtual ~BucketGrid(){
		for (int i = 0; i < m_size.x; i++)
		{
		    for (int j = 0; j < m_size.y; j++)
		        delete[] m_data[i][j];
		    delete[] m_data[i];
		}
		delete[] m_data;
	}

private:
	glm::ivec3 m_size;
	aabb m_box;
	float m_blockSize;
	QVector<T> ***m_data = nullptr; //xyz

};

#endif /* LIBRARIES_ATOMS_BUCKETGRID_H_ */
