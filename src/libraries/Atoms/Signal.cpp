/*
 * Signal.cpp
 *
 *  Created on: 14.08.2017
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

#include <Signal.h>
#include <QDebug>
#include <math.h>

namespace Atom {

void quantizeSignal(const QVector<float>& signalIn, QVector<float>& signalOut, float step, int lenght ){
	if(signalIn.isEmpty() || step <= 0 || lenght <= 0 ) return;
	if(lenght > signalIn.size()) lenght = signalIn.size();

	const float halfStep = step/2.f;
	signalOut.resize(signalIn.size());

	int start = 0;
	while(start < signalIn.size()){
		int end = start+lenght;
		if(end >= signalIn.size()) end = signalIn.size()-1;

		//mean
		float mean = 0.0f;
		int n = 0;
		for(int i = start; i < end; i++){
			const float delta = signalIn[i] - mean;
			mean += delta/++n;
		}

		//write
		const float val = floor((mean+halfStep)/step)*step; qDebug()<<"val: "<<val<<mean<<" range: "<<start<<end<<signalIn.size();
		for(int i = start; i < end; i++)
			signalOut[i] = val;
		start += lenght;
	}
}

} /* namespace Atom */
