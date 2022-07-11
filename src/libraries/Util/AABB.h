/**
 * @file   		AABB.h
 * @author 		Vladimir Ageev (vladimir.agueev@progsys.de)
 * @date   		04.05.2017
 *
 * @brief  		A simple Axis Aligned Bounding Box (AABB).
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
#ifndef LIBRARIES_UTIL_AABB_H_
#define LIBRARIES_UTIL_AABB_H_

#include <glm/glm.hpp>

/*!
 * A Axis Aligned Bounding Box (AABB). Its a box which each side is aligned to it's x,y or z axis and completely contains all given atoms centers.
 * Note, the atoms are only viewed as points, there radius is ignored.
 */
struct aabb{
	glm::vec3 min; /// The minimum value of the aabb.
	glm::vec3 max; /// The maximum value of the aabb.
};

#endif /* LIBRARIES_UTIL_AABB_H_ */
