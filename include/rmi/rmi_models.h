//
// Created by xxp on 2021/10/18.
//

#ifndef TS_RMIMODELS_H
#define TS_RMIMODELS_H

#include "models/model.h"
#include "model_creator.h"

namespace   rmi{

class   RMIModels{
 public:
  template <class KeyType>
  static  RMIModels New(
      const std::string &first, const std::string &second,
      const std::vector<KeyType> &keys,
                       const std::vector<uint64_t> &values,
                       uint32_t  submodels){
    Model  * first = ModelCreator::New(first, keys, values);


  }

 private:


  Model  *m_pFirstLayer;  //第一层的模型
  std::vector<Model*>   m_vSecondLayer; //第二层的模型
};


}

#endif  // TS_RMIMODELS_H
