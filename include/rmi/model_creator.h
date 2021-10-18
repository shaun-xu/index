//
// Created by xxp on 2021/10/18.
//

#ifndef TS_MODEL_CREATOR_H
#define TS_MODEL_CREATOR_H

#include "models/cubic_spline.h"
#include "models/linear.h"
#include "models/linear_spline.h"
#include "models/log_normal.h"
#include "models/model.h"
#include "models/normal.h"
#include "models/radix.h"
#include "models/radix_table.h"
#include "models/robust_linear.h"
#include <functional>

namespace   rmi{
//template  <class  KeyType>
//std::function<Model *(const std::vector<KeyType> &,const std::vector<uint64_t> &)>  create_func

class   ModelCreator{
 public:
  template <class KeyType>
  static  Model  *  New(const std::string &name,
                    const std::vector<KeyType> &keys,
                    const std::vector<uint64_t> &values){
    if(name == "linear"){
      return LinearModel::New(keys,values);
    }
    if(name == "robust_linear"){

    }

 }
 private:
};

}

#endif  // TS_MODEL_CREATOR_H
