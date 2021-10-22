//
// Created by xxp on 2021/10/20.
//

#ifndef TS_RMI_SPLINE_H
#define TS_RMI_SPLINE_H
#include <algorithm>
#include <cassert>
#include <cmath>
#include <limits>
#include <map>
#include <optional>
#include <fstream>

#include "model_creator.h"
#include "models/model.h"
#include "rmi_models.h"
#include "model_creator.h"
#include "radix_spline.h"

namespace   rmi{

template <class KeyType>
class   RMISpline  {
 public:

  // Returns a search bound [begin, end) around the estimated position.
  SearchBound GetSearchBound(const KeyType key)  {
    uint32_t  splint_index = GetSplinePoint(key);
    if(splint_index >0  && splint_index< m_vSplinePoints.size()-1){
      assert( m_vSplinePoints[splint_index-1].x < key && m_vSplinePoints[splint_index+1].x >key);
    }
    return  m_pSecond->GetSearchBound(key, splint_index);
  }


  static  RMISpline *  New(const std::string &rmi_top,
                        const std::string &rmi_leaf,
                        uint32_t sub_models,
                        const std::vector<KeyType>  &keys,
                        const std::vector<double> &values,
             size_t spline_max_error){
    //1:首先构造radix_spline
    RadixSpline<KeyType>    * radix =  new RadixSpline<KeyType>(keys,values,spline_max_error);
    const std::vector<Coord<KeyType>>  &spline_points =radix->SplinePoints();
    std::vector<KeyType>  train_keys(spline_points.size());
    std::vector<double>  train_values(spline_points.size());

    for (int i = 0; i < train_values.size(); ++i) {
      train_values[i]=i;
      train_keys[i]=spline_points[i].x;
    }
    //2: 在spline points的基础上构造 rmi结构体。
//    if(  rmi_leaf != ""){
    std::vector<double>     first_layer_data(train_values);
    double  datasize = (double)first_layer_data.size();
    for (int i = 0; i < datasize; ++i) {
      first_layer_data[i] = (double)train_values[i]*(sub_models)/(double )datasize;
    }
    // 3层模式
    RMIModels<KeyType>* model = RMIModels<KeyType>::New(
        rmi_top, rmi_leaf, train_keys, train_values, sub_models,first_layer_data);
    return  new RMISpline<KeyType>(spline_points, model,  radix);
  }
 private:
  uint32_t   GetSplinePoint(const KeyType key){
    SearchBound  first_bound = m_pFirstLayer->GetSearchBound(key);

    if (first_bound.end - first_bound.begin < 32) {
      // Do linear search over narrowed range.
      uint32_t current = first_bound.begin;
      while (m_vSplinePoints[current].x < key  && current<first_bound.end) ++current;
      return current;
    }

    // Do binary search over narrowed range.
    const auto lb = std::lower_bound(
        m_vSplinePoints.begin() + first_bound.begin, m_vSplinePoints.begin() + first_bound.end, key,
        [](const Coord<KeyType>& coord, const KeyType key) {
          return coord.x < key;
        });
    return std::distance(m_vSplinePoints.begin(), lb);
  }

  RMISpline(const std::vector<Coord<KeyType>>  &spline_points,RMIModels<KeyType>* first, RadixSpline<KeyType> *second ){
    m_vSplinePoints = spline_points;
    m_pFirstLayer = first;
    m_pSecond= second;
    m_pFirstLayer->DumpLayerErr();
  }

  std::vector<Coord<KeyType>>   m_vSplinePoints;
  RMIModels<KeyType>*     m_pFirstLayer;
  RadixSpline<KeyType>    *m_pSecond;

};

}

#endif  // TS_RMI_SPLINE_H
