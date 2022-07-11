/*
 * ProteinSurface.cpp
 *
 *  Created on: 04.05.2017
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

#include <ProteinSurface.h>
#include <Util/BucketGrid.h>

#include <QDebug>
#include <QPair>
#include <QElapsedTimer>

#define EPSILON 0.0001f

//#define DEBUG_EXTRACTION

/*!
 * @brief Cutting face definition
 */
struct cuttingFace{
    bool discarded; /// If true then this face can be ignored
    glm::vec3 center; /// The center of the plane (o), see figure 24
    glm::vec3 normal; /// Normal of the plane. Used for in-front tests. (n), see figure 24
    float radius; /// The radius of the intersection sphere (r_c)
    float normCutDistance; /// Cut distance on unit sphere
    float normRadius;
};

QDebug operator<< (QDebug d, const cuttingFace &m){
    d << "cuttingFace[center: "<<m.center<<" normal: "<<m.normal<<" P: "<<(m.center+m.normal)<<" radius: "<<m.radius<<" normCutDis: "<<m.normCutDistance<<"]";
    return d;
}

struct cutPair{
    QVector<cuttingFace>::iterator first;
    QVector<cuttingFace>::iterator second;
    float distance;
};

template<typename T>
inline T pow2(const T& a){ return a*a;}

/*!
 * @brief Filters endpoints with the given cutting faces.
 * All resulting endpoints are tested agents the filtered cutting faces, by
 * calculating the dot product with the face normal. If the dot product is positive, then
 * the endpoints is in-front the face and gets cut away. When at least one endpoints remains
 * after the tests, then the sphere and therefore the atom is classified as external,
 * otherwise as internal.
 * @param endPoints The endpoints that are tested and filtered against the cutting faces. Only the 'uncut' enspoints will remain.
 */
inline void testEndPoints(const QVector<cuttingFace>& cutPlanes, QVector<glm::vec3>& endPoints){
    //if *all* end points are in front of the planes then sphere is fully cut
    QVector<glm::vec3> filteredEndPoints;
    bool pointRemoved;
    for(const glm::vec3& p: endPoints){
        pointRemoved = false;
        for(const cuttingFace& plane: cutPlanes){
            if(plane.discarded) continue;
            //qDebug()<<"p: "<<p<<" dis: "<<(glm::dot(p-plane.center, plane.normal))<<" plane.center:"<<plane.center<<" plane.normal:"<<plane.normal;
            if(glm::dot(p-plane.center, plane.normal) > EPSILON){//point is in front of plane
                pointRemoved = true;
                break;
            }
        }
        if(!pointRemoved) //point hasn't been removed, so we save it
            filteredEndPoints.push_back(p);
    }

    endPoints = filteredEndPoints;
}

/*!
 * @brief Calculates the min and max value on the ellipse.
 * With the given values (angle and normCutDistance of plane A) the min
 * and max cut distance on the ellipse can be simply calculated.
 * @param minmax output min/max values
 */
inline void getEllipseMinMax(float (&minmax)[2], const cuttingFace& aPlane, float dot){

    /**
      Old formula
//    const float a2 = 1+aPlane.normCutDistance;
//    const float a = sqrt(a2);//i

//    const float b2 = 2-a2;
//    const float b = sqrt(b2);//j

//    const float t2 = 1.414213562f * a * b * sqrt((a2+b2-2*pow2(dot)));
//    const float t1 = a2*dot-b2*dot;
//    const float t3 = a2+b2;

//    minmax[0] = (t1-t2)/t3;
//    minmax[1] = (t1+t2)/t3;
*/

    // slightly improved formula
    const float r = aPlane.normRadius;
    const float y_mean = dot * (aPlane.normCutDistance);
    const float halfRange = sqrt(1 - pow2(dot));

    minmax[0] = y_mean - halfRange * r;
    minmax[1] = y_mean + halfRange * r;
}

/*!
 * @brief The the four possible configurations of two cutting faces, can be deducted by observing the ellipse.
 * The ellipse divides the space
 * into five sets, see figure 26, each corresponding to one possible configuration form
 * figure 25.
 * @brief cutPlanesPair Output pairs of cutting faces that intersect.
 */
inline bool getIntersectionPair(QVector<cuttingFace>::iterator it1, QVector<cuttingFace>::iterator it2, QVector<cutPair>& cutPlanesPair){
    float minmax[2];
    const float dot = glm::dot(it1->normal, it2->normal);
    getEllipseMinMax(minmax, *it1, dot);
    //see Listing 1
    if( !(minmax[0] != minmax[0])){
        if(it2->normCutDistance <= minmax[0]){
            if(dot >= -it1->normCutDistance){ // case (b)[discard B plane]
                it2->discarded = true;
            }//else // case(d)
        }else if(it2->normCutDistance >= minmax[1]){
            if(dot > it1->normCutDistance){ // case (b)[discard A plane]
                it1->discarded = true;
            }else{ // case (c)[Done]
                return true;
            }
        }else{ // case (a)
            cutPlanesPair.push_back( {it1, it2, ((it2->normCutDistance - minmax[0])/(minmax[1]-minmax[0]))-0.5f } );
        }
    }
    return false;
}

inline bool extractSurface(
        const QVector<Atoms::atom>& model,const TrajectoryStream::xtcFrame& frame, Atoms::layerFrame& layerframe, float propeRadius,
        BucketGrid<int>& grid, QVector<cuttingFace>& cutPlanes, QVector<cutPair>& cutPlanesPair, QVector<glm::vec3>& endPoints, int i, int layerCount
        #ifdef DEBUG_EXTRACTION
        , bool doDebug = false
        #endif
        ){

    const glm::vec3& posC = frame.positions[i]; // position of the sphere A in world space
    const float radiusC = model[i].radius + propeRadius; // the extended sphere radius of A

    if(layerCount > 42) {
        qDebug()<<__LINE__<<": ERROR: Reached maximum layer!";
        return false;
    }
    //clear data
    cutPlanes.clear();
    endPoints.clear();

    bool neighbor = false;
    int radius = -1;
    const int maxRadius = (radiusC); //glm::max(grid.getWidth(), glm::max(grid.getHeight(), grid.getLength()));//
    while(radius < maxRadius){
        radius++;

#ifdef DEBUG_EXTRACTION
        QElapsedTimer timer;
        if(doDebug){
            qDebug()<<"====== LOOP ======";
            qDebug()<<"["<<__LINE__<<"]: "<<"radius: "<<radius<<" i: "<<i<<" maxRadius: "<<maxRadius<<" layerCount: "<<layerCount<<" neighbor: "<<neighbor;
            timer.start();
        }

#endif
        /*
        QVector<cuttingFace> cutPlanesClean;
        for(const cuttingFace& p: cutPlanes)
            if(!p.discarded) cutPlanesClean.push_back(p);
        cutPlanes = cutPlanesClean;
        */

        const int cutPlaneStart = cutPlanes.size();

        QVector<const QVector<int>*> neighbors;
        if(!grid.getSurroundings(posC, neighbors, radius)) goto isOutside;
        if(neighbors.empty()) continue;

#ifdef DEBUG_EXTRACTION
        if(doDebug){
            qDebug()<<"["<<__LINE__<<"]: Extract neighbors:"<<" time: "<<(timer.nsecsElapsed()/1000000.f)<<" neighbors size: "<<neighbors.size();
            timer.start();
        }
#endif
        for(const QVector<int>* chunkPtr: neighbors){
            const QVector<int>& chunk = *chunkPtr;
            for(int index: chunk){

                if(i == index || layerframe.layers[index] < layerCount-1) continue; //we don't cut with itself or with spheres already with a layer
                const float radiusB = model[index].radius + propeRadius; //extended sphere radius of sphere B
                const glm::vec3 BtoC(frame.positions[index]-posC); //vector between both spheres

                //Possible configurations of two spheres inside the sphere cloud.
                const float distance = glm::length(BtoC);
                if(radiusB >= distance+radiusC){ //r_b > distance + r_a neighbor fully eats the sphere
                    goto isInside; // case c or d
                }
                if( distance >= (radiusB+radiusC) || radiusC >= distance+radiusB  ){ //ball out if radius or is inside
                    continue; // case a or b
                }
                //else intersection
                const float cutDistance  =  (pow2(distance)-pow2(radiusB)+pow2(radiusC))/(2.f*distance);//(distance² - r_b² + r_a²) / (2distance)
                cutPlanes.push_back( { //create new cutting face
                                       false,
                                       posC+ glm::normalize(BtoC)*cutDistance,
                                       glm::normalize(BtoC),
                                       (float)sqrt(pow2(radiusC)-pow2(cutDistance)),
                                       -cutDistance/radiusC,
                                       (float)sqrt(1 - pow2(-cutDistance/radiusC))
                                     } );
            }
        }//for loop end

#ifdef DEBUG_EXTRACTION
        if(doDebug){
            qDebug()<<"["<<__LINE__<<"]: Sphere intersection:"<<" time: "<<(timer.nsecsElapsed()/1000000.f)<< " cutPlanes size: "<<cutPlanes.size()<<" cutPlaneStart: "<<cutPlaneStart<<" new: "<<(cutPlanes.size()-cutPlaneStart);
            timer.start();
        }
#endif

        if(cutPlaneStart == cutPlanes.size()) continue; //no new planes added
        else neighbor = true;

        cutPlanesPair.clear();
        //filter the found planes

        for(auto it1 = cutPlanes.begin()+cutPlaneStart; it1 < cutPlanes.end(); it1++){//A plane
            if(it1->discarded) continue;

            //filter with previous planes
            for(auto it2 = cutPlanes.begin(); it2 < cutPlanes.begin()+cutPlaneStart; it2++){ //B plane
                if(it2->discarded) continue;
                if(getIntersectionPair(it1,it2, cutPlanesPair)) goto isInside;
            }

            //filter with itself
            for(auto it2 = it1+1; it2 <  cutPlanes.end(); it2++){ //B plane
                if(it2->discarded) continue;
                if(getIntersectionPair( it1,it2, cutPlanesPair)) goto isInside;
            }
        }
#ifdef DEBUG_EXTRACTION
        if(doDebug){
            qDebug()<<"["<<__LINE__<<"]: Filter cut planes:"<<" time: "<<(timer.nsecsElapsed()/1000000.f) <<" cutPlanesPair size: "<<cutPlanesPair.size();
            timer.start();
        }
#endif

        if(cutPlanesPair.empty()) continue;
        //cut each filtered plane pair to find the end points
        for(const cutPair& pair: cutPlanesPair){
            if(pair.first->discarded || pair.second->discarded) continue;
            const glm::vec3 u = glm::normalize(glm::cross(pair.second->normal, pair.first->normal)); //intersection line direction vector between the two planes
            const glm::vec3 v = glm::normalize(glm::cross(u, pair.first->normal));

            const float f = pair.distance*pair.first->radius*2.f;

            if( fabsf(f) > pair.first->radius){
                qDebug()<<"F is bigger than the radius!";
                continue;
            }

            const float l = sqrt( pow2(pair.first->radius) - pow2(f));//sqrt(radius² - f²) pythagoras
            const glm::vec3 G = pair.first->center+v*f;

            endPoints.push_back(G+u*l);
            endPoints.push_back(G-u*l);

        }
#ifdef DEBUG_EXTRACTION
        if(doDebug){
            qDebug()<<"["<<__LINE__<<"]: Find end points:"<<" time: "<<(timer.nsecsElapsed()/1000000.f)<<" endPoints size: "<<endPoints.size();
            timer.start();
        }
#endif
        if(endPoints.empty()) continue;

        testEndPoints(cutPlanes,endPoints);
#ifdef DEBUG_EXTRACTION
        if(doDebug){
            qDebug()<<"["<<__LINE__<<"]: Test end points:"<<" time: "<<(timer.nsecsElapsed()/1000000.f) <<" endPoints size: "<<endPoints.size();
        }
#endif
        if(endPoints.empty()){//sphere fully cut
            goto isInside;
        }
    } //while loop

    if(!neighbor) return false;

    //the two possible cases for a atom
isOutside:
    layerframe.layers[i] = layerCount-1;
    return true;
isInside:
    layerframe.layers[i] = layerCount;
    return true;

}

void debugFindEndPoints(QString& debugOut,const QVector<Atoms::atom>& model,const TrajectoryStream::xtcFrame& frame, float propeRadius, int condidate, int a, int b){
    qDebug()<<"==== TEST Extract Surface ====";

    const glm::vec3& posC = frame.positions[condidate];
    const float radiusC = model[condidate].radius + propeRadius;

    const glm::vec3& posA = frame.positions[a];
    const float radiusA = model[a].radius + propeRadius;

    const glm::vec3& posB = frame.positions[b];
    const float radiusB = model[b].radius + propeRadius;

    qDebug()<<"* C "<< condidate<<" pos:"<<posC<<" r:"<<radiusC;
    qDebug()<<"* A "<< a        <<"pos:"<<posA<<" r:"<<radiusA;
    qDebug()<<"* B "<< b        <<" pos:"<<posB<<" r:"<<radiusB;

    qDebug()<<"==== Sphere intersection ====";

    QVector<cuttingFace> cutPlanes;
    //intersection a
    {
        const glm::vec3 AtoC(posA-posC); //vector between both spheres
        const float distance = glm::length(AtoC);
        if(radiusA >= distance+radiusC){ //r_b > distance + r_a neighbor fully eats the sphere
            debugOut+= "INSIDE!\nA eats C!\n";
            return;
        }
        if( distance >= (radiusA+radiusC) || radiusC >= distance+radiusA  ){//ball out if radius or is inside
            debugOut+= "OUTSIDE!\nA doesn't intersect C!\n";
            return;
        }
        const float cutDistance  =  (pow2(distance)-pow2(radiusA)+pow2(radiusC))/(2.f*distance);//(distance² - r_b² + r_a²) / (2distance)
        qDebug()<<"A cutDistance: "<<cutDistance;
        cutPlanes.push_back( {
                                 false,
                                 posC+ glm::normalize(AtoC)*cutDistance,
                                 glm::normalize(AtoC),
                                 (float)sqrt(pow2(radiusC)-pow2(cutDistance)),
                                 -cutDistance/radiusC,
								 (float)sqrt(1 - pow2(-cutDistance/radiusC))
                             });
        qDebug()<<"A intersection:"<< cutPlanes.last();
    }
    //intersection c
    {
        const glm::vec3 BtoC(posB-posC); //vector between both spheres
        const float distance = glm::length(BtoC);
        if(radiusB >= distance+radiusC){ //r_b > distance + r_a neighbor fully eats the sphere
            debugOut+= "INSIDE!\nB eats C!\n";
            return;
        }
        if( distance >= (radiusB+radiusC) || radiusC >= distance+radiusB  ){ //ball out if radius or is inside
            debugOut+= "OUTSIDE!\nB doesn't intersect C!\n";
            return;
        }
        const float cutDistance  =  (pow2(distance)-pow2(radiusB)+pow2(radiusC))/(2.f*distance);//(distance² - r_b² + r_a²) / (2distance)
        cutPlanes.push_back( {
                                 false,
                                 posC+ glm::normalize(BtoC)*cutDistance,
                                 glm::normalize(BtoC),
                                 (float)sqrt(pow2(radiusC)-pow2(cutDistance)),
                                 -cutDistance/radiusC,
								 (float)sqrt(1 - pow2(-cutDistance/radiusC))
                             } );
        qDebug()<<"B intersection:"<<  cutPlanes.last();
    }

    qDebug()<<"==== Filter cut planes ====";

    QVector<cutPair> cutPlanesPair;

    {
        auto it1 = cutPlanes.begin();
        auto it2 = cutPlanes.begin()+1;

        float minmax[2];
        const float dot = glm::dot(it1->normal, it2->normal);
        {
            cuttingFace& aPlane = *it1;

            const float a2 = 1+aPlane.normCutDistance;
            const float a = sqrt(a2);

            const float b2 = 2-a2;
            const float b = sqrt(b2);

            const float t2 = 1.414213562f * a * b * sqrt((a2+b2-2*pow2(dot)));
            const float t1 = a2*dot-b2*dot;
            const float t3 = a2+b2;

            minmax[0] = (t1-t2)/t3;
            minmax[1] = (t1+t2)/t3;

            qDebug()<<"Ellipse:"<<" a2:"<<a2<<" a:"<<a<<" b2:"<<b2<<" b:"<<b;
            qDebug()<<"EQ:"<<" t1:"<<t1<<" t2:"<<t2<<" t3:"<<t3;
        }

        //getEllipseMinMax(minmax, *it1,*it2, dot);

        qDebug()<<"min: "<<minmax[0]<<" max: "<<minmax[1]<<" dot: "<<dot;

        if(minmax[0] != minmax[0]){
            qDebug()<<"Error";
        }else{
            if(it2->normCutDistance <= minmax[0]){
                if(dot >= -it1->normCutDistance){ // case (b)[discard B plane]
                    it2->discarded = true;
                }//else // case(d)
            }else if(it2->normCutDistance >= minmax[1]){
                if(dot > it1->normCutDistance){ // case (b)[discard A plane]
                    it1->discarded = true;
                }else{ // case (c)[Done]
                    debugOut+= "INSIDE!\nCut planes eat each other!\n";
                    return;
                }
            }else{ // case (a)
                cutPlanesPair.push_back( {it1, it2, ((it2->normCutDistance - minmax[0])/(minmax[1]-minmax[0]))-0.5f } );
            }
        }
    }

    if(cutPlanes[0].discarded){
        debugOut+= "OUTSIDE!\nPlane A discarded!\n";
        return;
    }
    if(cutPlanes[1].discarded){
        debugOut+= "OUTSIDE!\nPlane B discarded!\n";
        return;
    }

    if(cutPlanesPair.empty()){
        debugOut+= "OUTSIDE!\nNo intersection between planes!\n";
        return;
    }

    qDebug()<<"==== Find end points ====";
    QVector<glm::vec3> endPoints;
    {
        const cutPair& pair = cutPlanesPair.first();
        const glm::vec3 u = glm::normalize(glm::cross(pair.second->normal, pair.first->normal)); //intersection line direction vector between the two planes
        const glm::vec3 v = glm::normalize(glm::cross(u, pair.first->normal));

        const float f = pair.distance*pair.first->radius*2.f;

        if( fabsf(f) > pair.first->radius){
            qDebug()<<"F is bigger then the radius!";
        }

        const float l = sqrt( pow2(pair.first->radius) - pow2(f));//sqrt(radius² - f²) pythagoras
        const glm::vec3 G = pair.first->center+v*f;

        endPoints.push_back(G+u*l);
        endPoints.push_back(G-u*l);

        qDebug()<<"f in: "<< pair.distance<<"*"<<pair.first->radius<<"*2.f";
        qDebug()<<"G in: "<<pair.first->center<<"+"<<v<<"*"<<f;
        qDebug()<<"pair.distance: "<<pair.distance<<"f: "<<f<<" l: "<<l<<" G: "<<G<<" u: "<<u<<" v: "<<v;
        qDebug()<<"Point 1:"<<endPoints[0];
        qDebug()<<"Point 2:"<<endPoints[1];
    }
}

int extractSurface(const QVector<Atoms::atom>& model,const TrajectoryStream::xtcFrame& frame, Atoms::layerFrame& layerframe, float probeRadius){
    //first we build a grid to faster find atoms
    int layerCount = 1;
    layerframe.maxLayer = -1;
    layerframe.layers.fill(0,frame.positions.size());
    int numatoms = std::count(layerframe.layers.begin(), layerframe.layers.end(), 0.0f);
    qDebug()<<"Number of atoms: " << numatoms;

    //we build a grid to quickly find the neighbors
    BucketGrid<int> grid( frame.box, 2.f);
    for(int i = 0; i < frame.positions.size(); i++){
        if(model[i].residue == "HOH" || model[i].residue.toLower() == "water") continue;
        grid.insert(i,frame.positions[i]);
    }

    //needed data
    QVector<cuttingFace> cutPlanes;
    QVector<cutPair> cutPlanesPair;
    QVector<glm::vec3> endPoints;
    //next we cut each ball with its neighbors to see if something is left of it
    //if yes -> surface atom
    //if no -> not a surface atom
    while(true){
        bool end = true;

        for(int i = 0; i < frame.positions.size(); i++){
            if(model[i].residue == "HOH" || model[i].residue.toLower() == "water" || layerframe.layers[i] < layerCount-1) continue;
            if(extractSurface(
                        model, frame, layerframe, probeRadius,
                        grid,cutPlanes,cutPlanesPair,endPoints,i, layerCount
                        )) { end = false;}
        }
//#define COMPUTE_SURFACE_ONLY
#ifdef COMPUTE_SURFACE_ONLY
//         end after 1st layer, count number of atoms
        int numatoms = std::count(layerframe.layers.begin(), layerframe.layers.end(), 0.0f);
        qDebug()<<"Number of surface atoms: " << numatoms;
        break;
#endif
        if(end) break;
        layerCount++;
    }
    layerframe.maxLayer = layerCount-1;
    return layerCount-1;
}

void debugExtractSurface(const QVector<Atoms::atom>& model,const TrajectoryStream::xtcFrame& frame, Atoms::layerFrame& layerframe, float probeRadius, int atomID){
    //first we build a grid to faster find atoms
    int layerCount = 1;
    layerframe.layers.fill(0,frame.positions.size());

    //we build a grid to quickly find the neighbors
    BucketGrid<int> grid( frame.box, 2.f);
    for(int i = 0; i < frame.positions.size(); i++)
        grid.insert(i,frame.positions[i]);


    //needed data
    QVector<cuttingFace> cutPlanes;
    QVector<cutPair> cutPlanesPair;
    QVector<glm::vec3> endPoints;
    qDebug()<<"["<<__LINE__<<"]: "<<"Extract for "<<atomID;
    QElapsedTimer timer;
    timer.restart();
    extractSurface(
                model, frame, layerframe, probeRadius,
                grid,cutPlanes,cutPlanesPair,endPoints,atomID, layerCount
            #ifdef DEBUG_EXTRACTION
                , true
            #endif
                );
    const float time = timer.nsecsElapsed()/1000000.f;
    qDebug()<<"["<<__LINE__<<"]: "<<"Time "<<time<<" layer: "<<layerframe.layers[atomID];
}


ExtractSurfaceThread::ExtractSurfaceThread(Atoms* data, int startFrame, int endFrame, float propeRadius, QObject* parent):
    QThread(parent), m_data(data), m_start(startFrame), m_end(endFrame), m_propeRadius(propeRadius), m_remainingFrames(m_end-m_start){}

ExtractSurfaceThread::~ExtractSurfaceThread(){

}

float ExtractSurfaceThread::getProgress() const{
    return m_progress;
}

//Benchmark
float ExtractSurfaceThread::getAverageTime() const{
    return m_averageTime;
}

int ExtractSurfaceThread::getRemainingFrames() const{
    return m_remainingFrames;
}



void ExtractSurfaceThread::run(){
    if(!m_data || m_end < m_start || m_start < 0 || m_propeRadius < 0) {
        qDebug()<<__LINE__<<" Warning you are trying to start a extract surface thread with invalid parameters!";
        return;
    }
    TrajectoryStream stream(nullptr,m_data->numberOfAtroms(), &m_data->getOffsets(), m_data->getStream().getFileName());// = m_data->getStream().duplicate(this, 0);

    QElapsedTimer timer;
    try {
        for(int i = m_start; i <= m_end && i < (int)m_data->numberOfFrames() && !isInterruptionRequested(); i++){
            timer.restart();
            extractSurface(m_data->getAtoms(),  stream.getFrame(i), m_data->getLayer(i), m_propeRadius);
            //Benchmark
            const float time = timer.nsecsElapsed()/1000000.f;
            const int count = i-m_start;
            if(count == 0) m_averageTime = time;
            else{
                m_averageTime = (m_averageTime*count + time)/(float)(count+1);
            }
            //progress
            m_remainingFrames = m_end - i;
            m_progress = count/(float)(m_end-m_start);
        }
    } catch (std::bad_alloc& e) {
        qDebug()<<"Out of available memory! "<<e.what();
        m_data->getLayer(m_start).maxLayer = -1;
        m_data->getLayer(m_start).layers.clear();
        m_data->getLayer(m_start+1).maxLayer = -1;
        m_data->getLayer(m_start+1).layers.clear();
    }
}

