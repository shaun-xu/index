//
// Created by xxp on 2021/10/18.
//

#ifndef TS_LOG_NORMAL_H
#define TS_LOG_NORMAL_H

#include <math.h>  /* fma, FP_FAST_FMA */
#include <stdio.h> /* printf */
#include "model.h"
#include "utils.h"
#include <string>
#include "normal.h"

namespace   rmi{

class     LogNormal: public Normal{
 public:

  virtual double PredictFloat(uint64_t key) {
    return cnd_manual((double(log(key)) - m_nMean) / m_nStDev) * m_nScale;
  }

  std::string  Name(){  return "log_normal";}

  template <class  KeyType>
  static   LogNormal * New(const std::vector<KeyType>& keys,
                             const std::vector<double >& values){
    assert(keys.size() == values.size());
    if(keys.size() == 0){
      return  new LogNormal(0,0,0,0);
    }
      double   mean=0.0;
      double   stdev=0.0;
      double   scale=0.0;
      double   min=values[0];
      double   max =values[values.size()-1];
      scale = max-min;

      for (int i = 0; i < keys.size(); ++i) {
        mean+= (double )log(keys[i])/keys.size();
//        scale = std::max( (double )values[i] ,scale);
      }

      for (int i = 0; i < keys.size(); ++i) {
        stdev +=  pow( ((double)log(keys[i])-mean),2.0);
      }

      stdev /= keys.size();
      stdev = sqrt(stdev);

      return new  LogNormal(mean,  stdev,  scale, min);
    }
   protected:
  LogNormal(double  mean, double   dev, double   scale,double min):Normal(mean,dev,scale,min){

  }

};

}

#endif  // TS_LOG_NORMAL_H
