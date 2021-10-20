//
// Created by xxp on 2021/10/15.
//

#ifndef TS_MODEL_H
#define TS_MODEL_H

#include <math.h> /* floor */

#include <algorithm>
#include <cstdint>
#include <string>

namespace rmi {

//模型目前只支持两种类型
enum ModelDataType {
  UINT32 = 0,
  UINT64,
};

enum ModelRestriction { None = 0, MustBeTop, MustBeBottom };

class Model {
 public:
  virtual double PredictFloat(uint64_t key) {return  0.0;}

  virtual uint64_t Predict(uint64_t key) {
    return std::max(0.0, floor(PredictFloat(key)) );
  }

  virtual  std::string   Name()=0;

  bool NeedsBoundCheck() { return true; }

  ModelRestriction Restriction() { return ModelRestriction::None; }

  uint64_t ErrorBound() { return 0; }

  bool SetToConstanceModel(uint64_t con) { return false; }
};

}  // namespace rmi

#endif  // TS_MODEL_H
