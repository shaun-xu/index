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

  virtual double PredictFloat(uint64_t key) { return std::fma(key, m_a, m_b); }

  RobustLinear * New(const std::vector<KeyType>& keys,const std::vector<size_t>& values){

      return NULL;
  }

  std::string Name() { return "robust_linear"; }

 private:

};


}


#endif  // TS_ROBUST_LINEAR_H
