//
// Created by xxp on 2021/10/15.
//

#ifndef TS_UTILS_H
#define TS_UTILS_H
#include <assert.h>
#include <stdio.h>
#include "../../thirdpart/libdivide/libdivide.h"

#include <vector>

namespace rmi{

int32_t   NumBits(uint64_t  larget_value){
  int32_t nbits = 0;
  while( (1 << (nbits+1)) - 1 <= larget_value ){
        nbits += 1;
    }
  nbits -= 1;
  assert((1 << (nbits+1)) - 1 <= larget_value);
  return nbits;
}

int32_t CommonPrefixSize( const std::vector<uint32_t> &keys){
  uint32_t  any_ones = 0;
  uint32_t    no_ones = 0;
  for( uint64_t x : keys){
    any_ones |= x;
    no_ones &= x;
  }

  uint32_t   any_zeros = !no_ones;
  uint32_t     prefix_bits = any_zeros ^ any_ones;
  return libdivide::libdivide_count_leading_zeros32(prefix_bits);
}


int32_t CommonPrefixSize( const std::vector<uint64_t> &keys){
  uint64_t   any_ones = 0;
  uint64_t    no_ones = 0;
  for( uint64_t x : keys){
    any_ones |= x;
    no_ones &= x;
  }

  uint64_t   any_zeros = !no_ones;
  uint64_t     prefix_bits = any_zeros ^ any_ones;

  return libdivide::libdivide_count_leading_zeros64(prefix_bits);
}

double  Scale(double value, double  min , double  max){
  return  (value-min)/(max-min);
}

double   Exp1(double  x){
  x = 1.0 + x / 64.0;
  x *= x;
  x *= x;
  x *= x;
  x *= x;
  x *= x;
  x *= x;
  return x;
}

std::pair<double,double>    SlrSkip(const std::vector<uint64_t>& keys,
                                const std::vector<double>& values,uint32_t skip){
  double mean_x = 0.0;
  double mean_y = 0.0;
  double c = 0.0;
  double n = 0;
  double m2 = 0.0;
  assert(  (keys.size() == values.size()));

  u_int64_t data_size = keys.size();
  for (size_t i = skip; i < data_size-skip; i++) {
    double x = keys[i];
    double y = values[i];
    n += 1;
    double dx = x - mean_x;
    mean_x += dx / double(n);
    mean_y += (y - mean_y) / double(n);
    c += dx * (y - mean_y);

    double dx2 = x - mean_x;
    m2 += dx * dx2;
    data_size += 1;
  }

  // special case when we have 0 or 1 items
  if (data_size == 0) {
    return std::pair<double,double>(0.0, 0.0);
  }

  if (data_size == 1) {
    return std::pair<double,double>(0.0, mean_y);
  }

  double cov = c / double(n - 1);
  double var = m2 / double(n - 1);
  assert(var >= 0.0);

  if (var == 0.0) {
    // variance is zero. pick the mean (only) value.
    return std::pair<double,double>(0.0, mean_y);
  }
  double beta = cov / var;
  double alpha = mean_y - beta * mean_x;
  return std::pair<double,double>(beta, alpha);
}


std::pair<double,double>    Slr(const std::vector<uint64_t>& keys,
                                       const std::vector<double>& values){
  double mean_x = 0.0;
  double mean_y = 0.0;
  double c = 0.0;
  double n = 0;
  double m2 = 0.0;
  assert(  (keys.size() == values.size()));

  u_int64_t data_size = keys.size();
  for (size_t i = 0; i < data_size; i++) {
    double x = keys[i];
    double y = values[i];
    n += 1;
    double dx = x - mean_x;
    mean_x += dx / double(n);
    mean_y += (y - mean_y) / double(n);
    c += dx * (y - mean_y);

    double dx2 = x - mean_x;
    m2 += dx * dx2;
    data_size += 1;
  }

  // special case when we have 0 or 1 items
  if (data_size == 0) {
    return std::pair<double,double>(0.0, 0.0);
  }

  if (data_size == 1) {
    return std::pair<double,double>(0.0, mean_y);
  }

  double cov = c / double(n - 1);
  double var = m2 / double(n - 1);
  assert(var >= 0.0);

  if (var == 0.0) {
    // variance is zero. pick the mean (only) value.
    return std::pair<double,double>(0.0, mean_y);
  }
  double beta = cov / var;
  double alpha = mean_y - beta * mean_x;
  return std::pair<double,double>(beta, alpha);
}


}


#endif  // TS_UTILS_H
