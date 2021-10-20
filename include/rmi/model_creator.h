//
// Created by xxp on 2021/10/18.
//

#ifndef TS_MODEL_CREATOR_H
#define TS_MODEL_CREATOR_H

#include "models/cubic_spline.h"
#include "models/linear.h"
#include "models/linear_spline.h"
#include "models/log_linear.h"
#include "models/log_normal.h"
#include "models/model.h"
#include "models/normal.h"
#include "models/radix.h"
#include "models/radix_table.h"
#include "models/robust_linear.h"
#include "models/balanced_radix.h"

#include <functional>

namespace   rmi{
//template  <class  KeyType>
//std::function<Model *(const std::vector<KeyType> &,const std::vector<uint64_t> &)>  create_func

class   ModelCreator{
 public:
  template <class KeyType>
  static  Model  *  New(const std::string &name,
                    const std::vector<KeyType> &keys,
                    const std::vector<double> &values){
    if(name == "linear"){
      return LinearModel::New<KeyType>(keys,values);
    }
    if(name == "robust_linear"){
      return  RobustLinear::New<KeyType>(keys,values);
    }
    if (name == "linear_spline"){
      return LinearSpline::New<KeyType>(keys,values);
    }
    if (name == "cubic"){
      return CubicSpline::New<KeyType>(keys,values);
    }
    if(name == "log_linear"){
      return  LogLinear::New<KeyType>(keys,values);
    }
    if( name == "normal"){
      return  Normal::New<KeyType>(keys,values);
    }
    if(name == "log_normal"){
      return LogNormal::New<KeyType>(keys,values);
    }
    if( name == "radix"){
      return  Radix::New<KeyType>(keys,values);
    }
    if(name == "radix8"){
      return  RadixTable::New<KeyType>(keys,values, 8);
    }
    if( name == "radix18") {
      return RadixTable::New<KeyType>(keys, values, 18);
    }
    if( name == "radix22"){
      return RadixTable::New<KeyType>(keys, values, 22);
    }
    if( name == "radix26"){
      return RadixTable::New<KeyType>(keys, values, 26);
    }
    if( name == "radix28"){
      return RadixTable::New<KeyType>(keys, values, 28);
    }
    if( name == "bradix") {
      return BalancedRadix::New<KeyType>(keys,values);
    }
//    "histogram"
    return NULL;
 }
 private:
};

}

#endif  // TS_MODEL_CREATOR_H
