//
// Created by xxp on 2021/10/18.
//

#ifndef TS_RMIMODELS_H
#define TS_RMIMODELS_H

#include "models/model.h"
#include "model_creator.h"
#include <algorithm>

namespace   rmi{

template <class KeyType>
class   RMIModels{
 public:

  //计算该模型的叶子阶层的的损失。
  void  CalError(const std::vector<KeyType> &keys,
                const std::vector<double> &values){
    //    let mut last_layer_max_l1s = vec![(0, 0) ; num_leaf_models as usize];
    //    for (x, y) in md_container.iter_model_input() {
    //        let leaf_idx = top_model.predict_to_int(&x);
    //        let target = u64::min(num_leaf_models - 1, leaf_idx) as usize;
    //
    //        let pred = leaf_models[target].predict_to_int(&x);
    //        let err = error_between(pred, y as u64, md_container.len() as u64);
    //
    //        let cur_val = last_layer_max_l1s[target];
    //        last_layer_max_l1s[target] = (cur_val.0 + 1, u64::max(err, cur_val.1));
    //    }
    uint64_t   max_pos = values[ values.size()-1];
    m_vError.resize(m_vSecondLayer.size());
    for (int i = 0; i < keys.size(); ++i) {
      uint32_t   _model = m_pFirstLayer->Predict(  keys[i]);
      uint32_t   _model_predict =_model>m_vSecondLayer.size()-1?m_vSecondLayer.size()-1:_model;// std::min(_model,  m_vSecondLayer.size()-1);
      uint64_t  _pos = m_vSecondLayer[_model_predict]->Predict(keys[i]);
      uint64_t  _pos_predict = std::min(_pos,max_pos-1);
      m_vError[_model_predict]= std::max( (uint32_t )m_vError[_model_predict], (uint32_t )(_pos_predict>values[i]?_pos_predict-values[i]:values[i]-_pos_predict ) );
    }
    for (int i = 0; i < m_vError.size(); ++i) {
      if( i%10==0){
        std::cout<<""<<std::endl;
      }

      std::cout<<m_vError[i]<<",";
    }
  }

  static  RMIModels * New(
      const std::string &first, const std::string &second,
      const std::vector<KeyType> &keys,
                       const std::vector<double> &values,
                       uint32_t  submodels){
    assert(keys.size() == values.size());
      std::vector<double>     first_layer_data(values);
      double  datasize = (double)first_layer_data.size();
      for (int i = 0; i < datasize; ++i) {
          first_layer_data[i] = (double)first_layer_data[i]*(double)submodels/(double )datasize;
      }
      Model  * firstmod = ModelCreator::New(first, keys, first_layer_data);
      assert(firstmod);
      std::vector<Model*>  sub_models( submodels);
      for (int i = 0; i < sub_models.size(); ++i) {
        sub_models[i]=NULL;
      }

      uint32_t   last_model = 0;
      std::vector<KeyType>   train_keys;
      std::vector<double>     train_values;

      for (int i = 0; i < values.size(); ++i) {
        uint32_t  _predict = firstmod->Predict( keys[i]);
        uint32_t   predict = std::min(_predict,  submodels-1 );
        if(predict !=  last_model){
          //更新了，把数据更新到last模型。
          Model * tmp_model  = ModelCreator::New(second, train_keys, train_values);
          //清空数据
          train_values.clear();
          train_keys.clear();
          sub_models[last_model] = tmp_model;
          for (int j = last_model+1; j <predict ; ++j) {
            sub_models[j] = ModelCreator::New(second, train_keys, train_values);
          }
        }
        last_model = predict;
        train_values.push_back(values[i]);
        train_keys.push_back(keys[i]);
      }
      //数据到头了。
      assert( train_values.size()>0);
      Model * tm_last_model  = ModelCreator::New<KeyType>(second, train_keys, train_values);
      train_values.clear();
      train_keys.clear();
      sub_models[last_model] = tm_last_model;
      for (int j = last_model+1; j <submodels ; ++j) {
        sub_models[j] = ModelCreator::New(second, train_keys, train_values);
      }
      RMIModels<KeyType>  *retmodel = new  RMIModels<KeyType>(  firstmod,  sub_models);
      retmodel->CalError(keys,values);
      return retmodel;
  }

 private:
    RMIModels(Model  *top, const std::vector<Model*> & layer){
      m_pFirstLayer = top;
      m_vSecondLayer =std::move(layer);
    }

  Model  *m_pFirstLayer;  //第一层的模型
  std::vector<Model*>   m_vSecondLayer; //第二层的模型
  std::vector<uint32_t >  m_vError;   //叶子层的差错范围


  //    pub model_avg_error: f64,
  //    pub model_avg_l2_error: f64,
  //    pub model_avg_log2_error: f64,
  //    pub model_max_error: u64,
  //    pub model_max_error_idx: usize,
  //    pub model_max_log2_error: f64,
  //    pub last_layer_max_l1s: Vec<u64>,
  //    pub rmi: Vec<Vec<Box<dyn Model>>>,
  //    pub models: String,
  //    pub branching_factor: u64,
};


}

#endif  // TS_RMIMODELS_H
