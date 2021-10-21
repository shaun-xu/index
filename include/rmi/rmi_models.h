//
// Created by xxp on 2021/10/18.
//

#ifndef TS_RMIMODELS_H
#define TS_RMIMODELS_H

#include <algorithm>

#include "model_creator.h"
#include "models/model.h"
#include <sstream>


namespace rmi {

struct SearchBound {
  size_t begin;
  size_t end;  // Exclusive.
};

template <class KeyType>
class RMIModels {
 public:
  // Returns the estimated position of `key`.
  double GetEstimatedPosition(const KeyType key) const {
    //    // Truncate to data boundaries.
    //    if (key <= min_key_) return 0;
    //    if (key >= max_key_) return num_keys_ - 1;
    //
    //    // Find spline segment with `key` ∈ (spline[index - 1],
    //    spline[index]]. const size_t index = GetSplineSegment(key); const
    //    Coord<KeyType> down = spline_points_[index - 1]; const Coord<KeyType>
    //    up = spline_points_[index];
    //
    //    // Compute slope.
    //    const double x_diff = up.x - down.x;
    //    const double y_diff = up.y - down.y;
    //    const double slope = y_diff / x_diff;
    //
    //    // Interpolate.
    //    const double key_diff = key - down.x;
    //    return std::fma(key_diff, slope, down.y);
  }

  void  DumpLayerErr(){
    std::ostringstream  os;
    for (int i = 0; i < m_vError.size(); ++i) {
      if(i%20==0 && i!=0){
        std::cout<<os.str()<<std::endl;
        os.str("");
      }
      os<<i<<"_"<<m_vCounts[i]<<"_"<<m_vError[i]<<",";
    }
    std::cout<<os.str()<<std::endl;
  }


  // Returns a search bound [begin, end) around the estimated position.
  SearchBound GetSearchBound(const KeyType key) const {
    uint32_t _model = m_pFirstLayer->Predict(key);
    uint32_t _model_predict =
        _model > m_vSecondLayer.size() - 1
            ? m_vSecondLayer.size() - 1
            : _model;  // std::min(_model,  m_vSecondLayer.size()-1);
    uint64_t _pos = m_vSecondLayer[_model_predict]->Predict(key);
    const size_t estimate = std::min(_pos, m_nMaxPos);
    const size_t begin = (estimate < m_vError[_model_predict])
                             ? 0
                             : (estimate - m_vError[_model_predict]);
    // `end` is exclusive.
    const size_t end = (estimate + m_vError[_model_predict] + 2 > m_nMaxPos)
                           ? m_nMaxPos
                           : (estimate + m_vError[_model_predict] + 2);
    return SearchBound{begin, end};
  }

  //计算该模型的叶子阶层的的损失。
  void CalError(const std::vector<KeyType>& keys,
                const std::vector<double>& values) {
    //    let mut last_layer_max_l1s = vec![(0, 0) ; num_leaf_models as usize];
    //    for (x, y) in md_container.iter_model_input() {
    //        let leaf_idx = top_model.predict_to_int(&x);
    //        let target = u64::min(num_leaf_models - 1, leaf_idx) as usize;
    //
    //        let pred = leaf_models[target].predict_to_int(&x);
    //        let err = error_between(pred, y as u64, md_container.len() as
    //        u64);
    //
    //        let cur_val = last_layer_max_l1s[target];
    //        last_layer_max_l1s[target] = (cur_val.0 + 1, u64::max(err,
    //        cur_val.1));
    //    }
    uint64_t max_pos = values[values.size() - 1];
    m_vError.resize(m_vSecondLayer.size());
    m_vCounts.resize(m_vSecondLayer.size());
    for (int i = 0; i < keys.size(); ++i) {
      uint32_t _model = m_pFirstLayer->Predict(keys[i]);
      uint32_t _model_predict =
          _model > m_vSecondLayer.size() - 1
              ? m_vSecondLayer.size() - 1
              : _model;  // std::min(_model,  m_vSecondLayer.size()-1);
      uint64_t _pos = m_vSecondLayer[_model_predict]->Predict(keys[i]);
      uint64_t _pos_predict = std::min(_pos, max_pos - 1);
      uint32_t _cur_err =(uint32_t)(_pos_predict > values[i] ? _pos_predict - values[i]
                                                              : values[i] - _pos_predict);
      if(_cur_err > 10){
//        std::cout<<"big error="<<_cur_err<<",i="<<i<<",key="<<keys[i]<<",model="<<_model_predict<<",pos="<<_pos_predict<<",real="<<values[i]<<std::endl;
      }
      m_vCounts[_model_predict]++;
      m_vError[_model_predict] = std::max(
          (uint32_t)m_vError[_model_predict],
          _cur_err);
    }
    std::vector<uint32_t>::iterator min_e_pos = std::min_element(m_vError.begin(),m_vError.end());
    std::vector<uint32_t>::iterator max_e_pos = std::max_element(m_vError.begin(),m_vError.end());

    std::cout<<m_pFirstLayer->Name()<<"-"<<m_vSecondLayer[0]->Name()<<"-"<<m_vSecondLayer.size()<<","
              <<*max_e_pos
              <<","<<*min_e_pos
              <<std::endl;
//    for (int i = 0; i < m_vError.size(); ++i) {
//      if (i % 10 == 0) {
//        std::cout << "" << std::endl;
//      }
//
//      std::cout << m_vError[i] << ",";
//    }

  }

  static RMIModels* New(const std::string& first, const std::string& second,
                        const std::vector<KeyType>& keys,
                        const std::vector<double>& values, uint32_t submodels,
                        const std::vector<double>& first_layer_data) {
    assert(keys.size() == values.size());
    Model* firstmod = ModelCreator::New(first, keys, first_layer_data);
    assert(firstmod);
    std::vector<Model*> sub_models(submodels);
    for (int i = 0; i < sub_models.size(); ++i) {
      sub_models[i] = NULL;
    }

    uint32_t last_model = 0;
    std::vector<KeyType> train_keys;
    std::vector<double> train_values;

    for (int i = 0; i < values.size(); ++i) {
      uint32_t _predict = firstmod->Predict(keys[i]);
      uint32_t predict = std::min(_predict, submodels - 1);
      if (predict != last_model) {
        //更新了，把数据更新到last模型。
//        std::cout<<"train model:"<<predict<<",size="<<train_values.size()<<std::endl;
        Model* tmp_model = ModelCreator::New(second, train_keys, train_values);
        //清空数据
        train_values.clear();
        train_keys.clear();
        sub_models[last_model] = tmp_model;
        for (int j = last_model + 1; j < predict; ++j) {
          sub_models[j] = ModelCreator::New(second, train_keys, train_values);
        }
      }
      last_model = predict;
      train_values.push_back(values[i]);
      train_keys.push_back(keys[i]);
    }
    //数据到头了。
    assert(train_values.size() > 0);
    Model* tm_last_model =
        ModelCreator::New<KeyType>(second, train_keys, train_values);
    train_values.clear();
    train_keys.clear();
    sub_models[last_model] = tm_last_model;
    for (int j = last_model + 1; j < submodels; ++j) {
      sub_models[j] = ModelCreator::New(second, train_keys, train_values);
    }
    RMIModels<KeyType>* retmodel =
        new RMIModels<KeyType>(firstmod, sub_models, values[values.size() - 1]);
    retmodel->CalError(keys, values);
    return retmodel;
  }
  ~RMIModels(){
    if(m_pFirstLayer){
      delete m_pFirstLayer;
      m_pFirstLayer =NULL;
    }
    for (int i = 0; i < m_vSecondLayer.size(); ++i) {
      if(m_vSecondLayer[i]){
        delete m_vSecondLayer[i];
        m_vSecondLayer[i]=NULL;
      }
    }
    m_nMaxPos = 0;
  }

 private:
  RMIModels(Model* top, const std::vector<Model*>& layer, uint64_t max_pos) {
    m_pFirstLayer = top;
    m_nMaxPos = max_pos;
    m_vSecondLayer = std::move(layer);
  }

  Model* m_pFirstLayer;                //第一层的模型
  std::vector<Model*> m_vSecondLayer;  //第二层的模型
  std::vector<uint32_t> m_vError;      //叶子层的差错范围
  std::vector<uint32_t > m_vCounts;   //叶子层的数据数量
  uint64_t m_nMaxPos;                  //最大的位置
};

}  // namespace rmi

#endif  // TS_RMIMODELS_H
