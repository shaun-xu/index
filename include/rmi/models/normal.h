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
    return cnd_manual((double(key) - m_nMean) / m_nStDev) * m_nScale+m_nStart;
  }

  std::string  Name(){  return "normal";}

  template <class  KeyType>
  static   Normal *  New(const std::vector<KeyType>& keys,
                         const std::vector<double >& values){

    assert(keys.size() == values.size());
    if(keys.size() == 0){
      return  new Normal(0,0,0,0);
    }
    double   mean=0.0;
    double   stdev=0.0;
    double   scale=0.0;
    double   min=values[0];
    double   max =values[values.size()-1];
    scale = max-min;

    for (int i = 0; i < keys.size(); ++i) {
      mean+= (double )keys[i]/keys.size();
//      scale = std::max( (double )values[i] ,scale);
    }

    for (int i = 0; i < keys.size(); ++i) {
      stdev +=  pow( ((double)keys[i]-mean),2.0);
    }

    stdev /= keys.size();
    stdev = sqrt(stdev);

    return new  Normal(mean,  stdev,  scale,min);
  }

 protected:

#ifndef Pi
#define Pi 3.141592653589793238462643
#endif

  double cnd_manual(double x)
  {
    double L, K, w ;
    /* constants */
    double const a1 = 0.31938153, a2 = -0.356563782, a3 = 1.781477937;
    double const a4 = -1.821255978, a5 = 1.330274429;

    L = fabs(x);
    K = 1.0 / (1.0 + 0.2316419 * L);
    w = 1.0 - 1.0 / sqrt(2 * Pi) * exp(-L *L / 2) * (a1 * K + a2 * K *K + a3 * pow(K,3) + a4 * pow(K,4) + a5 * pow(K,5));

    if (x < 0 ){
      w= 1.0 - w;
    }
    return w;
  }

  double Phi(double x){
    return 1.0 / (1.0 + exp(-1.65451 * x));
  }

  Normal(double  mean, double   dev, double   scale, double  start){
    m_nMean = mean;
    m_nScale = scale;
    m_nStDev = dev;
    m_nStart = start;
  }
  double  m_nStart;
  double   m_nMean;
  double   m_nStDev;
  double   m_nScale;
};

}

#endif  // TS_NORMAL_H
