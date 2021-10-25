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
struct   KeyPos{
  bool    valid;
  KeyType   key;
  double    pos;
  KeyPos(){
    valid=false;
  }
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
      os<<i<<"_"<<m_vCounts[i]<<"_"<<m_vError[i]<<"_"<<m_vMinMax[i].begin<<"_"<<m_vMinMax[i].end<<std::endl;
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
    uint64_t max_pos = values[values.size() - 1];
    m_vError.resize(m_vSecondLayer.size());
    m_vCounts.resize(m_vSecondLayer.size());
    m_vMinMax.resize(m_vSecondLayer.size());
    m_vMinMaxPos.resize(m_vSecondLayer.size());
    m_vNext.resize(m_vSecondLayer.size());
    m_vPrev.resize(m_vSecondLayer.size());

    std::vector<  KeyPos<KeyType> >  tmpFirst;
    std::vector<  KeyPos<KeyType> >  tmpLast;
    tmpFirst.resize(m_vSecondLayer.size());
    tmpLast.resize(m_vSecondLayer.size());


    for (int i = 0; i < m_vMinMax.size(); ++i) {
      m_vMinMax[i].begin=(size_t)(-1);
      m_vMinMax[i].end = 0;
      tmpFirst[i].valid = false;
      tmpLast[i].valid=false;
    }
    uint32_t   pre_model =0;
    KeyPos<KeyType>   last_key;

    for (int i = 0; i < keys.size(); ++i) {
      if(i==7638920){
        std::cout<<"here"<<std::endl;
      }
      uint32_t _model = m_pFirstLayer->Predict(keys[i]);
      uint32_t _model_predict =
          _model > m_vSecondLayer.size() - 1
              ? m_vSecondLayer.size() - 1
              : _model;  // std::min(_model,  m_vSecondLayer.size()-1);

      if(tmpFirst[_model_predict].valid == false){
        tmpFirst[_model_predict].valid=true;
        tmpFirst[_model_predict].key = keys[i];
        tmpFirst[_model_predict].pos = values[i];
      }
      {  //直接更新tmpLast
        tmpLast[_model_predict].valid=true;
        tmpLast[_model_predict].key = keys[i];
        tmpLast[_model_predict].pos = values[i];
      }

      if(m_vMinMax[_model_predict].begin > keys[i]){
        m_vMinMax[_model_predict].begin = keys[i];
      }

      if(m_vMinMax[_model_predict].end<keys[i]){
        m_vMinMax[_model_predict].end= keys[i];
      }

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
      last_key.pos = values[i];
      last_key.key = keys[i];

    }
    //此处还需要校正两个模型之间的点，如果把后面的模型
    //TODO  这里应该是最大的 和 最小的值的位置。
    m_vPrev[0].pos = 0;
    m_vPrev[0].key = keys[0];
    m_vNext[m_vSecondLayer.size()-1].key = keys[keys.size()-1];
    m_vNext[m_vSecondLayer.size()-1].pos = values[keys.size()-1];
    for(int  i=1; i< m_vSecondLayer.size()-1; i++){
        int   prev = getPrev( i, tmpFirst);
        int   next = getNext(i,  tmpLast);
        m_vNext[i] = tmpLast[next];
        m_vPrev[i] = tmpFirst[prev];

    }
    for( int i=0;  i<m_vSecondLayer.size(); i++){
      uint64_t next_pre_pos = m_vSecondLayer[i]->Predict(m_vNext[i].key);
      uint64_t _pos_next = std::min(next_pre_pos, max_pos - 1);
      uint64_t prev_pre_pos = m_vSecondLayer[i]->Predict(m_vPrev[i].key);
      uint64_t _pos_prev = std::min(prev_pre_pos, max_pos - 1);
      int   next_err = _pos_next> m_vNext[i].pos?_pos_next-m_vNext[i].pos:m_vNext[i].pos-_pos_next;
      int   prev_err = _pos_prev>m_vPrev[i].pos?_pos_prev-m_vPrev[i].pos:m_vPrev[i].pos-_pos_prev;
      int  final_max_err =  std::max(prev_err, next_err);
      if( final_max_err > m_vError[i] &&   final_max_err-m_vError[i]>100 ){
        std::cout<<"big correaltion "<<i<<","<<next_err<<","<<prev_err<<","<<m_vError[i]<<","<<final_max_err - m_vError[i]<<std::endl;
      }
      m_vError[i] = std::max((uint32_t )final_max_err, m_vError[i]);
    }

    std::vector<uint32_t>::iterator min_e_pos = std::min_element(m_vError.begin(),m_vError.end());
    std::vector<uint32_t>::iterator max_e_pos = std::max_element(m_vError.begin(),m_vError.end());

    std::cout<<m_pFirstLayer->Name()<<"-"<<m_vSecondLayer[0]->Name()<<"-"<<m_vSecondLayer.size()<<","
              <<*max_e_pos
              <<","<<*min_e_pos
              <<std::endl;

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

  int   getNext(uint32_t  start_pos, const  std::vector<  KeyPos<KeyType> >  &next_array){
    for (int i = start_pos+1; i <next_array.size() ; ++i) {
      if(next_array[i].valid){
        return   i;
      }
    }
    return next_array.size()-1;
  }

  int   getPrev(uint32_t  start_pos, const  std::vector<  KeyPos<KeyType> >  &prev_array){
    for(   int i=start_pos-1;  i>=0;  i--){
      if(prev_array[i].valid){
        return   i;
      }
    }
    return 0;
  }

  Model* m_pFirstLayer;                //第一层的模型
  std::vector<Model*> m_vSecondLayer;  //第二层的模型
  std::vector<uint32_t> m_vError;      //叶子层的差错范围
  std::vector<uint32_t > m_vCounts;   //叶子层的数据数量
  std::vector<SearchBound>  m_vMinMax;  //最小的和最大的key
  std::vector<SearchBound>  m_vMinMaxPos;  //最小的和最大的key对应的位置
  uint64_t m_nMaxPos;                  //最大的位置
  std::vector< KeyPos<KeyType>  >     m_vNext;   //下一个
  std::vector< KeyPos<KeyType>  >     m_vPrev;   //下一个
};

}  // namespace rmi

#endif  // TS_RMIMODELS_H
