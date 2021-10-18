//
// Created by xxp on 2021/10/15.
//

#ifndef TS_ROBUST_LINEAR_H
#define TS_ROBUST_LINEAR_H

#include <math.h>  /* fma, FP_FAST_FMA */
#include <stdio.h> /* printf */
#include "linear.h"
#include "utils.h"
#include <vector>

namespace   rmi{

class   RobustLinear{
 public:

  virtual double PredictFloat(uint64_t key) { return std::fma(key, m_nA, m_nB); }

  RobustLinear * New(const std::vector<uint64_t>& keys,const std::vector<double>& values){
    assert(keys.size() == values.size());
    uint32_t   bnd = std::max(double(1),  keys.size() * 0.0001);
    std::pair<double,double>  result = SlrSkip(keys,values, bnd);
    return new RobustLinear(result.first,result.second);
  }

  std::string Name() { return "robust_linear"; }

 private:
  RobustLinear(double a, double  b){
    m_nA = a;
    m_nB = b;
  }

  double  m_nA;
  double  m_nB;
};


}


#endif  // TS_ROBUST_LINEAR_H
