/*!
 * HeatmapSettingsStruct.h
 *
 *  Created on: 17.09.2017
 *      Author: Vladimir Ageev
 *
 * @brief A structs that defines possible settings for heatmap image generation
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

#ifndef EXECUTABLES_AMINOVIS_HEATMAPSETTINGSSTRUCT_H_
#define EXECUTABLES_AMINOVIS_HEATMAPSETTINGSSTRUCT_H_

#include <QString>

///heatmap screenshot settings
struct heatmapScreenshotSettings{
	/// The width of the heatmap image
	int width = 800;
	/// The height of each row of the heatmap. Total height is rowHeight * number of rows
	int rowHeight = 20;
	/// Font to be used by the labels
	QString font = "Arial";
	/// Used color preset for the heatmap
	QString colorPreset = "Default";
	/// Should the labels for the y axis be drawn (left side)
	bool yLabels = false;
	/// Should the labels for the x axis be drawn (top side)
	bool xLabels = false;
	/// Should the heatmap ignore all atoms
	bool collapseAll = false;
};

#endif /* EXECUTABLES_AMINOVIS_HEATMAPSETTINGSSTRUCT_H_ */
