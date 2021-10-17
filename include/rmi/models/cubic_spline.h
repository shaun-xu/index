//
// Created by xxp on 2021/10/17.
//

#ifndef TS_CUBIC_SPLINE_H
#define TS_CUBIC_SPLINE_H
#include <assert.h>
#include "model.h"
#include "utils.h"
#include "../../thirdpart/libdivide/libdivide.h"

namespace  rmi{

class   CubicSpline : public  Model{
 public:
  virtual double PredictFloat(uint64_t key) {return  0.0;}

  static  CubicSpline * New(const std::vector<uint64_t>& keys,
                          const std::vector<double >& values){
    assert(keys.size() == values.size());


  }

 private:

};

}

#endif  // TS_CUBIC_SPLINE_H
