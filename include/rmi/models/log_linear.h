//
// Created by xxp on 2021/10/15.
//

#ifndef TS_LOG_LINEAR_H
#define TS_LOG_LINEAR_H

#include <math.h>  /* fma, FP_FAST_FMA */
#include <stdio.h> /* printf */
#include "linear.h"
#include "utils.h"
#include <vector>

namespace   rmi{
class   LogLinear : public Model{
 public:
  virtual double PredictFloat(uint64_t key) {
    return Exp1( fma(key, m_a, m_b)) ;
  }

  template <class  KeyType>
  static LogLinear * New(const std::vector<KeyType>& keys,
                          const std::vector<double >& values) {
    assert(keys.size() == values.size());
    if(keys.size() == 0){
      return  new LogLinear(0,0);
    }

    std::vector<double>  log_value;
    log_value.resize(values.size());
    for (int i = 0; i < values.size(); ++i) {
      log_value[i]=( log(values[i]));
    }

    std::pair<double,double>  data = Slr(keys,log_value);
    return new LogLinear(data.first,data.second);
  }

  std::string Name() { return "log_linear"; }
  virtual  uint32_t   Size(){
    return   sizeof(m_a)+sizeof(m_b);//+sizeof(m_nC)+sizeof(m_nD) ;
  }


 private:

  LogLinear(double  a, double  b){
    m_a = a;
    m_b = b;
  }

  double  m_a;
  double  m_b;

};

}

#endif  // TS_LOG_LINEAR_H
