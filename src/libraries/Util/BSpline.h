/*
 * BSpline.h
 *
 *  Created on: 02.06.2017
 *      Author: Vladimir Ageev (vladimir.agueev@progsys.de
 */

#ifndef LIBRARIES_UTIL_BSPLINE_H_
#define LIBRARIES_UTIL_BSPLINE_H_

#include <QVector>
#include <QDebug>

//cox-de boor recusion formula
float cbf(int i, int j, float t, const QVector<float>& knots){
	if(j == 0){
		return knots[i] <= t && t < knots[i+1];
	}else{
		return
				((t-knots[i])/(knots[i+j]-knots[i]))*cbf(i,j-1,t, knots) +
				((knots[i+j+1]-t)/(knots[i+j+1]-knots[i+1]))*cbf(i+1,j-1,t, knots);
	}
}



/*!
 * @brief A simple implementation of a B-Spline with uniform knots.
 *
 * @see https://www.ibiblio.org/e-notes/Splines/basis.html
 * @see https://www.youtube.com/watch?v=r6UcF0S0HvQ
 */
template<int p, int k, typename T>
class BSpline {
public:
	BSpline(){
		m_knots.reserve(getNumberOfKnots());
		for(float i = 0; i <= getNumberOfKnots(); i+=1.f)
			m_knots.push_back(i);
	}

	inline int getNumberOfControlPoints() const{ return p;}
	inline int getNumberOfKnots() const{ return p+k+1;}
	inline int getDegree() const{ return k;}
	inline float getStartKnot() const{ return m_knots[getDegree()];}
	inline float getEndKnot() const{ return m_knots[getNumberOfKnots()-getDegree()];}

	T get(float t, const QVector<T>& points){
		return get(t, points.begin());
	}

	T get(float t, const typename QVector<T>::const_iterator& start){
		T sum;
		for(int i = 0; i <= p; i++)
			sum += cbf(i,k,t,m_knots)*(*(start+i));
		return sum;
	}

	T getNormelized(float t, const QVector<T>& points){
		return getNormelized(t, points.begin());
	}

	T getNormelized(float t, const typename QVector<T>::const_iterator& start){
		return get( getStartKnot() + (getEndKnot()-getStartKnot())*t, start);
	}


	virtual ~BSpline(){}
private:

	QVector<float> m_knots;
};


/*!
 * @brief A fast implementation of a B-Spline with uniform knots.
 *
 * The basis values get precalculated for the given number of samples.
 *
 * @see BSpline
 */
template<int p, int k, int samples, typename T>
class FastBSpline {
public:
	FastBSpline(){

		QVector<float> knots;
		knots.reserve(getNumberOfKnots());
		/*
		float val = 0;
		for(int i  = 0; i < getDegree()-1; i++)
			knots.push_back(val);

		val += 1.f;
		for(int i  = 0; i < getNumberOfKnots()-getDegree()*2+3; i++){
			knots.push_back(val);
			val += 1.f;
		}

		for(int i  = 0; i < getDegree()-1; i++)
			knots.push_back(val);
		*/
		//uniform knot vector

		for(float i = 0; i <= getNumberOfKnots(); i+=1.f)
			knots.push_back(i);

		//all bases get precalculated
		const float startKnot = knots[getDegree()];
		const float endKnot = knots[getNumberOfKnots()-getDegree()-1];
		const float growRate = (endKnot-startKnot)/((float)samples);

		float t = startKnot;
		for(int s = 0; s < samples; s++){
			for(int i = 0; i < p; i++){
				m_samples[s][i] = cbf(i,k, t, knots);
			}
			t += growRate;
			//if(t > endKnot) qDebug()<<"t is too big "<<t<<" e"<<endKnot;
		}
	}

	inline int getNumberOfControlPoints() const{ return p;}
	inline int getNumberOfKnots() const{ return p+k+1;}
	inline int getDegree() const{ return k;}

	void get(QVector<T>& out, const QVector<T>& points){
		get(out, points.begin());
	}

	void get(QVector<T>& out, const typename QVector<T>::const_iterator& start){
		for(int s = 0; s < samples; s++){
			T sum;
			for(int i = 0; i < p; i++)
				sum += m_samples[s][i]*(*(start+i));
			out.push_back(sum);
		}
	}


	virtual ~FastBSpline(){}
private:
	//cox-de boor recusion formula
	float cbf(int i, int j, float t, const QVector<float>& knots){
		if(j == 0){
			return knots[i] <= t && t < knots[i+1];
		}else{
			return
					((t-knots[i])/(knots[i+j]-knots[i]))*cbf(i,j-1,t, knots) +
					((knots[i+j+1]-t)/(knots[i+j+1]-knots[i+1]))*cbf(i+1,j-1,t, knots);
		}
	}
	float m_samples[samples][p];
};

#endif /* LIBRARIES_UTIL_BSPLINE_H_ */
