//
// Created by xxp on 2021/10/17.
//

#ifndef TS_BALANCED_RADIX_H
#define TS_BALANCED_RADIX_H

#include <math.h>  /* fma, FP_FAST_FMA */
#include <stdio.h> /* printf */
#include "model.h"
#include "utils.h"
#include <string>
#include <limits>

namespace  rmi{

class   BalancedRadix : public Model{
 public:
  virtual uint64_t Predict(uint64_t key) {
    uint64_t  input = (key<<m_nPrefix)>>(64-m_nBits);
    if(m_bHigh){
      return  std::min(input, m_nMax);
    }
    if(input < m_nMax)
      return 0;
    return   input-m_nMax;
  }

  static  bool  Test(){
    std::vector<uint64_t>   keys(65000);
    std::vector<double>   values(65000);

    for (int i = 0; i <values.size() ; ++i) {
        keys[i]=i;
        values[i]=i;
    }
    BalancedRadix  * model = BalancedRadix::New<uint64_t>(keys,values);
    uint64_t delta=0;
    int max = 0;
    for (int i = 0; i <values.size() ; ++i) {
      uint64_t   pos = model->Predict(keys[i]);
      int current= abs(int (pos-values[i]) );
      if(current > max){
        max = current;
      }
      delta += current;
    }
    delete  model;
  }

  template <class KeyType>
  static  BalancedRadix * New(const std::vector<KeyType>& keys,
                              const std::vector<double >& values){

    uint64_t   larget = values[values.size()-1];
    uint32_t   bits = NumBits(larget);
    uint32_t   range  = bits+2>64?64:bits+2;
    uint32_t   common_prefix = CommonPrefixSize(keys);
    BalancedRadix  best_model;
    double   best_score =  std::numeric_limits<double>::max();
    for (int i = bits; i < range; ++i) {
      uint64_t  bit_max = (   1<<(i+1)) -1;
      BalancedRadix  high( common_prefix, i, larget-1, true );
      double high_score=high.CalScore<KeyType>(keys,values, larget);
      if(high_score<best_score){
        best_score = high_score;
        best_model = high;
      }
      BalancedRadix  low(common_prefix,  i, larget-bit_max, false);
      double  low_score = low.CalScore<KeyType>(keys,values, larget);
      if(low_score < best_score){
        best_score = low_score;
        best_model = low;
      }
    }
    return new BalancedRadix(best_model);
  }

  template <class KeyType>
  double   CalScore(const std::vector<KeyType>& keys,
                      const std::vector<double >& values, uint32_t max_bin){
    std::vector<uint32_t >  counts(max_bin);
    for (int i = 0; i < keys.size(); ++i) {
      uint64_t   pos = Predict(keys[i]);
      counts[pos]+=1;
    }
    double expected = (double )keys.size()/ (double )max_bin;
    double sum =0.0;
    for (int i = 0; i < counts.size(); ++i) {
       sum += std::pow(  double (counts[i])-expected , 2.0)/expected;
    }
    return  sum;
  }

 private:
  BalancedRadix(){

  }
  BalancedRadix(uint32_t prefix, uint32_t  bits,  uint64_t  max,bool high){
    m_nPrefix = prefix;
    m_nBits = bits;
    m_bHigh = high;
    m_nMax = max;
  }

  uint32_t  m_nPrefix;
  uint32_t  m_nBits;
  uint64_t  m_nMax;
  bool  m_bHigh;
};

}

#endif  // TS_BALANCED_RADIX_H
