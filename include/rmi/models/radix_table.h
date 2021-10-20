//
// Created by xxp on 2021/10/16.
//

#ifndef TS_RADIX_TABLE_H
#define TS_RADIX_TABLE_H

#include <assert.h>
#include "model.h"
#include "utils.h"

namespace  rmi{

class   RadixTable: public Model{
 public:
  virtual uint64_t Predict(uint64_t key) {
    uint64_t as_int = key;
    uint32_t prefix = m_nPrefix;
    uint32_t bits = m_nBits;
    uint32_t num_bits =  prefix + bits > 64 ?0: 64 - (prefix + bits);
    uint32_t res = ((as_int << prefix) >> prefix) >> num_bits;
    uint64_t idx = m_vTables[res];
    return idx;
  }

  template <class KeyType>
  static  RadixTable  * New(const std::vector<KeyType>& keys,
                            const std::vector<double >& values,uint32_t bits){
    assert(keys.size() == values.size());
    if(keys.size() == 0){
      std::vector<uint32_t> tmpy;
      return  new RadixTable(0,0, tmpy);
    }
    uint32_t  prefix = CommonPrefixSize(keys);
    std::vector<uint32_t > table(1<<bits);
    uint64_t   last_radix = 0;
    uint32_t  shift_bits = prefix+bits>64?0:64-prefix-bits;

    for (int i = 0; i < keys.size(); ++i) {
      auto x = keys[i];
      auto y = values[i];
      uint32_t  current_radix = ((x<<prefix)>>prefix)>>shift_bits;
      if(current_radix == last_radix)
        continue;
      table[current_radix]=y;
      for (int j = last_radix+1; j < current_radix; ++j) {
        table[j] = y;
      }
      last_radix = current_radix;
    }
    for (int i = last_radix+1; i < table.size(); ++i) {
       table[i] = table.size();
    }

    return  new  RadixTable(prefix, bits, table);
  }

  std::string   Name(){return "radix_table";}

 private:
  RadixTable(uint32_t  prefix, uint32_t  bits , const std::vector<uint32_t> &tables){
    m_nPrefix = prefix;
    m_nBits = bits;
    m_vTables=std::move(tables);
  }

  uint32_t   m_nPrefix;
  uint32_t   m_nBits;
  std::vector<uint32_t>  m_vTables;
};

}

#endif  // TS_RADIX_TABLE_H
