//
// Created by xxp on 2021/10/17.
//

#ifndef TS_LINEAR_SPLINE_H
#define TS_LINEAR_SPLINE_H

#include <assert.h>
#include "model.h"
#include "utils.h"
#include "../../thirdpart/libdivide/libdivide.h"

namespace  rmi{

class   LinearSpline : public Model{
 public:

  virtual double PredictFloat(uint64_t key) {
      return  fma(key, m_fSlope, m_fIntercept);
  }

  std::string  Name(){return "linear_spline";}

  template <class  KeyType>
  static   LinearSpline * New(const std::vector<KeyType>& keys,
                              const std::vector<double >& values){
    assert(keys.size() == values.size());
    if(keys.size() == 0){
      return  new LinearSpline(0,0);
    }
    double  slope =  double(values[values.size()-1]-values[0])/\
                   (double )(keys[keys.size()-1]-keys[0]);
    double  intercept = double ( values[0] - keys[0]*slope);

    return new LinearSpline(intercept, slope);
  }
 private:
  LinearSpline(double  intercept, double  slope){
    m_fIntercept = intercept;
    m_fSlope = slope;
  }
  double  m_fIntercept;  //截距
  double  m_fSlope;//斜率
};


} // end of namespace rmi

#endif  // TS_LINEAR_SPLINE_H
