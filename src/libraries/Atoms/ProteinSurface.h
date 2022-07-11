/**
 * @file   		ProteinSurface.h
 * @author 		Vladimir Ageev
 * @date   		04.05.2017
 *
 * @brief  		Contains the extract surface function and it's worker thread.
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


#ifndef LIBRARIES_ATOMS_PROTEINSURFACE_H_
#define LIBRARIES_ATOMS_PROTEINSURFACE_H_

#include <Atoms/Atoms.h>
#include <QThread>

/*!
 * @brief Extracts the SAS layers for a given model and frame.
 *
 * The algorithm described in simple words: Each atom is represent
 * as a sphere with the radii of the VdW radius, plus the given probe radius by
 * the user, hence it’s called Extended Spheres. The result is a sphere cloud. Afterwards,
 * each sphere is intersected with all other spheres in the cloud. These intersections
 * cut out a portion of the sphere, like an apple, where you bite a chunk of. If
 * after all the intersections any of the original surface (in the case of the apple its red
 * skin) is remaining, then the sphere is classified as external, otherwise as internal.
 * To now receive the SAS layers, the external spheres are removed and the process is
 * repeated again, until no spheres are left. In each new iteration a counter is increased
 * and its number gets assigned to the external spheres, giving each atom its layer in
 * the protein.
 * @param model The protein model.
 * @param frame One frame of the trajectory.
 * @param layerframe Output layer data for each atom inside the model.
 * @param propeRadius The radius used for the extended spheres.
 * @returns Maximum extracted layer
 */
int extractSurface(const QVector<Atoms::atom>& model,const TrajectoryStream::xtcFrame& frame, Atoms::layerFrame& layerframe, float propeRadius);

///@brief Used for debugging.
void debugExtractSurface(const QVector<Atoms::atom>& model,const TrajectoryStream::xtcFrame& frame, Atoms::layerFrame& layerframe, float propeRadius, int atomID);
///@brief Used for debugging.
void debugFindEndPoints(QString& debugOut, const QVector<Atoms::atom>& model,const TrajectoryStream::xtcFrame& frame, float propeRadius, int condidate, int a, int b);

/*!
 * @brief Each thread receives a window of the trajectory to process.
 * Frames of the trajectory are divided into n-parts and processed
 * in individual threads. This way each thread has a consistent workload by
 * being independent form the other threads.
 */
class ExtractSurfaceThread : public QThread
{
    Q_OBJECT
public:
	ExtractSurfaceThread(Atoms* data, int startFrame, int endFrame, float propeRadius, QObject *parent = nullptr);
    virtual ~ExtractSurfaceThread();
    float getProgress() const;

    //Benchmark
    float getAverageTime() const;
    int getRemainingFrames() const;
protected:
    void run();
private:
    Atoms* m_data = nullptr;
    int m_start;
    int m_end;
    float m_propeRadius;
    int m_remainingFrames;

    float m_progress = 0;
    //Benchmark
    float m_averageTime = -99999;
};

#endif /* LIBRARIES_ATOMS_PROTEINSURFACE_H_ */
