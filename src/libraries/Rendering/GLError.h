/**
 * @file   		GLError.h
 * @author 		Vladimir Ageev (vladimir.agueev@progsys.de)
 * @date   		Nov 20, 2016
 *
 * @brief  		Wrapper around glGetError() to return the errors in a readable fashion.
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

#ifndef LIBRARIES_WINGEDEDGE_GLERROR_H_
#define LIBRARIES_WINGEDEDGE_GLERROR_H_

#if ( defined(_MSC_VER) && !defined(__INTEL_COMPILER) )
	#define FUNCTION_NAME __FUNCTION__
#elif __MINGW32__
	#define FUNCTION_NAME __PRETTY_FUNCTION__
#elif __linux__
	#define FUNCTION_NAME __PRETTY_FUNCTION__
#else
	#define FUNCTION_NAME "noName"
#endif//

bool check_gl_error(const char *file, int line);

/// Checks if glGetError() returns any errors. They get printed out if any occur.
#define checkGLError() check_gl_error(FUNCTION_NAME,__LINE__)

#endif /* LIBRARIES_WINGEDEDGE_GLERROR_H_ */
