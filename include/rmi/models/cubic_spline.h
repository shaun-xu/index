//
// Created by xxp on 2021/10/17.
//

#ifndef TS_CUBIC_SPLINE_H
#define TS_CUBIC_SPLINE_H
#include <assert.h>
#include "model.h"
#include "utils.h"
#include <algorithm>
#include "math.h"
#include "linear.h"

#include "../../thirdpart/libdivide/libdivide.h"

namespace  rmi{

struct    stCubic{
  double   A;
  double   B;
  double   C;
  double   D;
};



class   CubicSpline : public  Model{
 public:
  virtual double PredictFloat(uint64_t key) {
    // out=ax^3+bx^2+cx+d
    double   tmp = fma(m_nA,(double )key,m_nB);
    tmp= fma(tmp,key,m_nC);
    tmp = fma(tmp,key,m_nD);
    return tmp;
  }

  template <class KeyType>
  static  CubicSpline * New(const std::vector<KeyType>& keys,
                          const std::vector<double >& values){
    assert(keys.size() == values.size());
    if(keys.size() == 0){
      return  new CubicSpline(0,0,0, 0);
    }
    //cubic模型包含了线性模型，比较两者最优的
    LinearModel * liner = LinearModel::New(keys, values);
    stCubic  cubic = Cubic(keys,values);
    CubicSpline  cub_spline(cubic.A,cubic.B,cubic.C,cubic.D);
    double  cub_err =0.0;
    double  line_err = 0.0;
    for (int i = 0; i < keys.size(); ++i) {
      double   c_pred = cub_spline.PredictFloat( keys[i]);
      double   l_pred =  liner->PredictFloat(keys[i]);
      cub_err +=  abs(  c_pred- values[i]);
      line_err +=  abs(l_pred-values[i]);
    }
    if(cub_err >= line_err){
        return new CubicSpline(0.0,0.0, liner->m_a,liner->m_b);
    }
    return  new CubicSpline(cubic.A, cubic.B, cubic.C, cubic.D);
  }

  static  stCubic  Cubic(const std::vector<uint64_t>& keys,
                 const std::vector<double >& values){
    uint64_t xmin=keys[0];
    uint64_t ymin=values[0];
    uint64_t xmax = keys[ keys.size()-1];
    uint64_t ymax = values[values.size()-1];
    double  x1,y1=0.0;
    double  x2,y2=1.0;
    double  m1=0.0;
    double  m2=0.0;

    for (int i = 1; i < keys.size(); ++i) {
      if(Scale(keys[i], xmin, xmax)>0.0){
        double  sxn = Scale(keys[i],xmin,xmax);
        double  syn = Scale(values[i],ymin,ymax);
        m1= (syn - y1) / (sxn - x1);
        break;
      }
    }

    for (int i = keys.size()-1; i >0 ; --i) {
      if(Scale(keys[i],xmin,xmax)<1.0){
        double  sxp = Scale(keys[i],xmin,xmax);
        double  syp = Scale(values[i],ymin,ymax);
        m2= (y2 - syp) / (x2 - sxp);
        break;
      }
    }

    if( pow(m1,2.0)+pow(m2,2.0)> 9.0){
        double  tau = 3.0/sqrt(pow(m1,2.0)+pow(m2,2.0));
        m1 *= tau;
        m2 *= tau;
    }
//    // from sympy, the first (a) term is:
//    // '(m1 + m2 - 2)/(xmax - xmin)**3'
    double a=(m1+m2-2)/pow( xmax-xmin,3.0);
//    // the second (b) term is:
//    // '-(xmax*(2*m1 + m2 - 3) + xmin*(m1 + 2*m2 - 3))/(xmax - xmin)**3'
    double b= (-(xmax*(2*m1 + m2 - 3) + xmin*(m1 + 2*m2 - 3)))/pow(xmax-xmin, 3.0);
//    // the third (c) term is:
//    // '(m1*xmax**2 + m2*xmin**2 + xmax*xmin*(2*m1 + 2*m2 - 6))
//    //  /(xmax - xmin)**3'
    double  c = (m1*pow(xmax,2)+m2*pow(xmin,2)+xmax*xmin*(2*m1+2*m2-6))/pow(xmax-xmin,3);
//    // the fourth (d) term is:
//    // '-xmin*(m1*xmax**2 + xmax*xmin*(m2 - 3) + xmin**2)/(xmax - xmin)**3'
    double  d=(-xmin*(m1*pow(xmax,2)+xmax*xmin*(m2-3)+pow(xmin,2)))/pow(xmax-xmin,3);
    a *= ymax - ymin;
    b *= ymax - ymin;
    c *= ymax - ymin;
    d *= ymax - ymin;
    d += ymin;
    return stCubic{A:a,B:b,C:c,D:d};
  }

  std::string Name() { return "cubic_spline"; }

 private:
  CubicSpline(double a, double  b, double  c ,double  d){
    m_nA= a;
    m_nB=b;
    m_nC=c;
    m_nD=d;
  }

  double   m_nA;
  double   m_nB;
  double   m_nC;
  double   m_nD;
};

}

#endif  // TS_CUBIC_SPLINE_H
