//
// Created by xxp on 2021/10/21.
//

#ifndef TS_RADIX_SPLINE_H
#define TS_RADIX_SPLINE_H

#include <algorithm>
#include <cassert>
#include <cmath>
#include <fstream>
#include <limits>
#include <map>
#include <optional>

#include "model_creator.h"
#include "models/model.h"
#include "rmi_models.h"

namespace rmi {
// A CDF coordinate.
template <class KeyType>
struct Coord {
  KeyType x;
  double y;
};

template <class KeyType>
class RadixSpline {
 public:
  RadixSpline(const std::vector<KeyType>& keys,
              const std::vector<double>& values, size_t max_error)
      : spline_max_error_(max_error) {
    min_key_ = keys[0];
    max_key_ = keys[keys.size() - 1];
    prev_key_ = 0;
    curr_num_keys_ = 0;
    curr_num_distinct_keys_ = 0;
    prev_key_ = min_key_;
    prev_position_ = 0;
    for (int i = 0; i < keys.size(); ++i) {
      AddKey(keys[i], values[i]);
    }
    if (curr_num_keys_ > 0 && spline_points_.back().x != prev_key_)
      AddKeyToSpline(prev_key_, prev_position_);

    std::cout << "final" << std::endl;
  }

  const  std::vector<Coord<KeyType>>  SplinePoints(){
    return spline_points_;
  }

  // Returns a search bound [begin, end) around the estimated position.
  SearchBound GetSearchBound(const KeyType key,double  index) const {
    const size_t estimate = GetEstimatedPosition(key,index);
    const size_t begin = (estimate < spline_max_error_) ? 0 : (estimate - spline_max_error_);
    // `end` is exclusive.
    const size_t end = (estimate + spline_max_error_ + 2 > curr_num_keys_)
                           ? curr_num_keys_
                           : (estimate + spline_max_error_ + 2);
    return SearchBound{begin, end};
  }


  double GetEstimatedPosition(const KeyType key, double  index) const {
    // Truncate to data boundaries.
    if (key <= min_key_) return 0;
    if (key >= max_key_) return curr_num_keys_ - 1;

    // Find spline segment with `key` ∈ (spline[index - 1], spline[index]].
//    const size_t index = GetSplineSegment(key);
    const Coord<KeyType> down = spline_points_[index - 1];
    const Coord<KeyType> up = spline_points_[index];

    // Compute slope.
    const double x_diff = up.x - down.x;
    const double y_diff = up.y - down.y;
    const double slope = y_diff / x_diff;

    // Interpolate.
    const double key_diff = key - down.x;
    return std::fma(key_diff, slope, down.y);
  }
 private:
  static unsigned ComputeLog(uint32_t n, bool round = false) {
    assert(n);
    return 31 - __builtin_clz(n) + (round ? ((n & (n - 1)) != 0) : 0);
  }

  static unsigned ComputeLog(uint64_t n, bool round = false) {
    assert(n);
    return 63 - __builtin_clzl(n) + (round ? ((n & (n - 1)) != 0) : 0);
  }

  static unsigned ComputeLcp(uint32_t x, uint32_t y) {
    return __builtin_clz(x ^ y);
  }

  static unsigned ComputeLcp(uint64_t x, uint64_t y) {
    return __builtin_clzl(x ^ y);
  }

  // Returns the number of shift bits based on the `diff` between the largest
  // and the smallest key. KeyType == uint32_t.
  static size_t GetNumShiftBits(uint32_t diff, size_t num_radix_bits) {
    const uint32_t clz = __builtin_clz(diff);
    if ((32 - clz) < num_radix_bits) return 0;
    return 32 - num_radix_bits - clz;
  }

  // KeyType == uint64_t.
  static size_t GetNumShiftBits(uint64_t diff, size_t num_radix_bits) {
    const uint32_t clzl = __builtin_clzl(diff);
    if ((64 - clzl) < num_radix_bits) return 0;
    return 64 - num_radix_bits - clzl;
  }

  // `int(ceil(log_2(distance)))`. I
  static size_t ComputeCost(uint32_t value) {
    assert(value);
    if (value == 1) return 1;
    return 31 - __builtin_clz(value) + ((value & (value - 1)) != 0);
  }

  void AddKey(KeyType key, size_t position) {
    assert(key >= min_key_ && key <= max_key_);
    // Keys need to be monotonically increasing.
    assert(key >= prev_key_);
    // Positions need to be strictly monotonically increasing.
    assert(position == 0 || position > prev_position_);

    PossiblyAddKeyToSpline(key, position);

    ++curr_num_keys_;
    prev_key_ = key;
    prev_position_ = position;
  }

  void AddKeyToSpline(KeyType key, double position) {
    spline_points_.push_back({key, position});
    //    AddKeyToCHT(key);
  }

  enum Orientation { Collinear, CW, CCW };
  static constexpr double precision = std::numeric_limits<double>::epsilon();

  static Orientation ComputeOrientation(const double dx1, const double dy1,
                                        const double dx2, const double dy2) {
    const double expr = std::fma(dy1, dx2, -std::fma(dy2, dx1, 0));
    if (expr > precision)
      return Orientation::CW;
    else if (expr < -precision)
      return Orientation::CCW;
    return Orientation::Collinear;
  };

  void SetUpperLimit(KeyType key, double position) {
    upper_limit_ = {key, position};
  }
  void SetLowerLimit(KeyType key, double position) {
    lower_limit_ = {key, position};
  }
  void RememberPreviousCDFPoint(KeyType key, double position) {
    prev_point_ = {key, position};
  }

  // Implementation is based on `GreedySplineCorridor` from:
  // T. Neumann and S. Michel. Smooth interpolating histograms with error
  // guarantees. [BNCOD'08]
  void PossiblyAddKeyToSpline(KeyType key, double position) {
    if (curr_num_keys_ == 0) {
      // Add first CDF point to spline.
      AddKeyToSpline(key, position);
      ++curr_num_distinct_keys_;
      RememberPreviousCDFPoint(key, position);
      return;
    }

    if (key == prev_key_) {
      // No new CDF point if the key didn't change.
      return;
    }

    // New CDF point.
    ++curr_num_distinct_keys_;

    if (curr_num_distinct_keys_ == 2) {
      // Initialize `upper_limit_` and `lower_limit_` using the second CDF
      // point.
      SetUpperLimit(key, position + spline_max_error_);
      SetLowerLimit(key, (position < spline_max_error_)
                             ? 0
                             : position - spline_max_error_);
      RememberPreviousCDFPoint(key, position);
      return;
    }

    // `B` in algorithm.
    const Coord<KeyType>& last = spline_points_.back();

    // Compute current `upper_y` and `lower_y`.
    const double upper_y = position + spline_max_error_;
    const double lower_y =
        (position < spline_max_error_) ? 0 : position - spline_max_error_;

    // Compute differences.
    assert(upper_limit_.x >= last.x);
    assert(lower_limit_.x >= last.x);
    assert(key >= last.x);
    const double upper_limit_x_diff = upper_limit_.x - last.x;
    const double lower_limit_x_diff = lower_limit_.x - last.x;
    const double x_diff = key - last.x;

    assert(upper_limit_.y >= last.y);
    assert(position >= last.y);
    const double upper_limit_y_diff = upper_limit_.y - last.y;
    const double lower_limit_y_diff = lower_limit_.y - last.y;
    const double y_diff = position - last.y;

    // `prev_point_` is the previous point on the CDF and the next candidate to
    // be added to the spline. Hence, it should be different from the `last`
    // point on the spline.
    assert(prev_point_.x != last.x);

    // Do we cut the error corridor?
    if ((ComputeOrientation(upper_limit_x_diff, upper_limit_y_diff, x_diff,
                            y_diff) != Orientation::CW) ||
        (ComputeOrientation(lower_limit_x_diff, lower_limit_y_diff, x_diff,
                            y_diff) != Orientation::CCW)) {
      // Add previous CDF point to spline.
      AddKeyToSpline(prev_point_.x, prev_point_.y);

      // Update limits.
      SetUpperLimit(key, upper_y);
      SetLowerLimit(key, lower_y);
    } else {
      assert(upper_y >= last.y);
      const double upper_y_diff = upper_y - last.y;
      if (ComputeOrientation(upper_limit_x_diff, upper_limit_y_diff, x_diff,
                             upper_y_diff) == Orientation::CW) {
        SetUpperLimit(key, upper_y);
      }

      const double lower_y_diff = lower_y - last.y;
      if (ComputeOrientation(lower_limit_x_diff, lower_limit_y_diff, x_diff,
                             lower_y_diff) == Orientation::CCW) {
        SetLowerLimit(key, lower_y);
      }
    }

    RememberPreviousCDFPoint(key, position);
  }
 private:
  KeyType min_key_;
  KeyType max_key_;
  size_t spline_max_error_;
  std::vector<Coord<KeyType>> spline_points_;

  size_t curr_num_keys_;
  size_t curr_num_distinct_keys_;
  KeyType prev_key_;
  double prev_position_;
  // Current upper and lower limits on the error corridor of the spline.
  Coord<KeyType> upper_limit_;
  Coord<KeyType> lower_limit_;
  // Previous CDF point.
  Coord<KeyType> prev_point_;
};

}  // namespace rmi

#endif  // TS_RADIX_SPLINE_H
