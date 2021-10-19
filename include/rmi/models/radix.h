//
// Created by xxp on 2021/10/16.
//

#ifndef TS_RADIX_H
#define TS_RADIX_H

#include <assert.h>

#include "model.h"
#include "utils.h"

namespace  rmi{
class   Radix : public Model{
 public:
  template <class  KeyType>
  static  Radix  * New(const std::vector<KeyType>& keys,
                       const std::vector<double >& values){
    assert(keys.size() == values.size());
    uint64_t  max_pos = (uint64_t )values[values.size()-1];
    int32_t   bits = NumBits( max_pos);
    int32_t   common_prefix = CommonPrefixSize(keys);
    return new Radix(common_prefix, bits);
  }

  std::string   Name(){return "radix";}

 private:
  Radix(size_t  common_prefix, size_t  bits){
    m_nCommonPrefix = common_prefix;
    m_nBits = bits;
  }

  size_t    m_nCommonPrefix;
  size_t    m_nBits;
};

};



#endif  // TS_RADIX_H
