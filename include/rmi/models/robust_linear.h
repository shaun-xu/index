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

class   RobustLinear:public  Model{
 public:

  virtual double PredictFloat(uint64_t key) { return fma(key, m_nA, m_nB); }

  template <class  KeyType>
  static  RobustLinear * New(const std::vector<KeyType>& keys,const std::vector<double>& values){
    assert(keys.size() == values.size());
    if(keys.size() == 0){
      std::vector<uint32_t> tmpy;
      return  new RobustLinear(0,0);
    }

    uint32_t   bnd = std::max(double(1),  keys.size() * 0.0001);
    std::pair<double,double>  result = SlrSkip(keys,values, bnd);
    return new RobustLinear(result.first,result.second);
  }

  std::string Name() { return "robust_linear"; }
  virtual  uint32_t   Size(){
    return   sizeof(m_nA)+sizeof(m_nB);//+sizeof(uint32_t)*m_vTables.size() ;
  }

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
