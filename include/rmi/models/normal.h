//
// Created by xxp on 2021/10/18.
//

#ifndef TS_NORMAL_H
#define TS_NORMAL_H

#include <math.h>  /* fma, FP_FAST_FMA */
#include <stdio.h> /* printf */
#include "model.h"
#include "utils.h"
#include <string>

namespace   rmi{

struct    StNormal{
  double   Mean;
  double   StDev;
  double   Scale;
};

class   Normal : public  Model{
 public:

  virtual double PredictFloat(uint64_t key) {
    return Phi((double(key) - m_nMean) / m_nStDev) * m_nScale;
  }

  std::string  Name(){  return "normal";}

  template <class  KeyType>
  static   Normal *  New(const std::vector<KeyType>& keys,
                         const std::vector<double >& values){

    assert(keys.size() == values.size());
    if(keys.size() == 0){
      return  new Normal(0,0,0);
    }
    double   mean=0.0;
    double   stdev=0.0;
    double   scale=0.0;

    for (int i = 0; i < keys.size(); ++i) {
      mean+= (double )keys[i]/keys.size();
      scale = std::max( (double )values[i] ,scale);
    }

    for (int i = 0; i < keys.size(); ++i) {
      stdev +=  pow( ((double)keys[i]-mean),2.0);
    }

    stdev /= keys.size();
    stdev = sqrt(stdev);

    return new  Normal(mean,  stdev,  scale);
  }

 protected:

  double Phi(double x){
    return 1.0 / (1.0 + Exp1(-1.65451 * x));
  }

  Normal(double  mean, double   dev, double   scale){
    m_nMean = mean;
    m_nScale = scale;
    m_nStDev = dev;
  }

  double   m_nMean;
  double   m_nStDev;
  double   m_nScale;
};

}

#endif  // TS_NORMAL_H
