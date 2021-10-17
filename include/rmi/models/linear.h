//
// Created by xxp on 2021/10/15.
//

#ifndef TS_LINEAR_H
#define TS_LINEAR_H

#include <math.h>  /* fma, FP_FAST_FMA */
#include <stdio.h> /* printf */
#include "model.h"
#include "utils.h"
#include <string>

namespace rmi {

class LinearModel : public Model{
 public:
  virtual double PredictFloat(uint64_t key) { return std::fma(key, m_a, m_b); }

  static LinearModel*  New(const std::vector<uint64_t>& keys,
                          const std::vector<double >& values) {
    std::pair<double,double>  data = Slr(keys,values);
    return new LinearModel(data.first,data.second);
  }

  std::string Name() { return "linear"; }

 private:
  LinearModel(double a, double b) {
    m_a = a;
    m_b = b;
  }

  double m_a;
  double m_b;
};

}  // namespace rmi

#endif  // TS_LINEAR_H
