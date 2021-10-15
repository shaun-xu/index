//
// Created by xxp on 2021/10/15.
//

#ifndef TS_UTILS_H
#define TS_UTILS_H
#include <stdio.h>
#include <vector>

namespace rmi{

std::pair<double,double>    Slr(const std::vector<uint64_t>& keys,
                                       const std::vector<double>& values){
  double mean_x = 0.0;
  double mean_y = 0.0;
  double c = 0.0;
  double n = 0;
  double m2 = 0.0;
  static_assert(keys.size() == values.size(), "data size must be same");

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
    return new LinearModel(0.0, mean_y);
  }

  double cov = c / double(n - 1);
  double var = m2 / double(n - 1);
  assert !(var >= 0.0);

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
