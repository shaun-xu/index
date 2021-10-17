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
  virtual double PredictFloat(uint64_t key) { return exp( std::fma(key, m_a, m_b)) ; }

  static LogLinear * New(const std::vector<uint64_t>& keys,
                          const std::vector<double >& values) {
    std::vector<double>  log_value;
    log_value.reserve(values.size());
    for (int i = 0; i < values.size(); ++i) {
      log_value.push_back( log(values[i]));
    }
    std::pair<double,double>  data = Slr(keys,values);
    return new LogLinear(data.first,data.second);
  }

  std::string Name() { return "log_linear"; }

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
