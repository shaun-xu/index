//
// Created by xxp on 2021/10/18.
//

#ifndef TS_RMIMODELS_H
#define TS_RMIMODELS_H

#include "models/model.h"

namespace   rmi{

class   RMIModels{
 public:


 private:
  Model  *m_pFirstLayer;  //第一层的模型
  std::vector<Model*>   m_vSecondLayer; //第二层的模型
};


}

#endif  // TS_RMIMODELS_H
