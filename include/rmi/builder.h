//
// Created by xxp on 2021/10/18.
//

#ifndef TS_BUILDER_H
#define TS_BUILDER_H

#include "models/cubic_spline.h"
#include "models/linear.h"
#include "models/linear_spline.h"
#include "models/log_normal.h"
#include "models/model.h"
#include "models/normal.h"
#include "models/radix.h"
#include "models/radix_table.h"
#include "models/robust_linear.h"
#include "rmi_models.h"
#include "rmi_spline.h"
#include <chrono>

namespace  rmi{

template <class KeyType>
class   Builder{
 public:
  const static   std::vector<std::string>  top_layers;//{"linear", "robust_linear", "linear_spline","cubic","loglinear","normal","lognormal","radix", "radix18", "radix22","radix26","radix28" };
  const static   std::vector<std::string>  leaf_layers;//{"linear", "robust_linear","linear_spline","cubic","loglinear","normal","lognormal" };
  const static   std::vector<uint32_t >   submodels;//{2<<5,2<<6,2<<7,2<<8,2<<9,2<<10,2<<11,2<<12,2<<13,2<<14,2<<15,2<<16,2<<17,2<<18,2<<19,2<<20,2<<21,2<<22,2<<23,2<<24,2<<25};

  static  RMIModels<KeyType>  *Build(const std::vector<KeyType> &keys, const std::vector<double > &values){
    //根据数据选择最适宜的模型进行计算。
    for (int k = 0; k < submodels.size(); ++k) {
      //在这里方便对value进行一次缩放多次使用吧
      std::vector<double>     first_layer_data(submodels[k]);
      uint32_t   permode_count = values.size()/submodels[k];
//      double  datasize = (double)first_layer_data.size();
      for (int i = 0; i < submodels[k]; ++i) {
        first_layer_data[i] = values[permode_count*i];//(double)values[i]*((double)submodels.size())/(double )datasize;
      }
      for (int i = 0; i < top_layers.size(); ++i) {
        for (int j = 0; j < leaf_layers.size(); ++j) {
          RMISpline<KeyType>* model = RMISpline<KeyType>::New(
              top_layers[i], leaf_layers[j],submodels[k], keys, values,10);
          assert(model);
          //计算时间
          TestModel(keys,values,model);
          delete model;
//          model->DumpLayerErr();
        }
      }
    }
  }

  static void TestModel(const std::vector<KeyType> &keys, const std::vector<double > &values , RMISpline<KeyType> *model){
    auto  begin = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < keys.size(); ++i) {
      if( i== 9999826){
        std::cout<<"here"<<std::endl;
      }
      rmi::SearchBound bond = model->GetSearchBound(keys[i]);
      assert(i >=bond.begin  && i<= bond.end );
    }
    auto  end = std::chrono::high_resolution_clock::now();
    uint64_t build_ns =
        std::chrono::duration_cast<std::chrono::nanoseconds>(end - begin)
            .count();
    std::cout<<"outputresult:"<<model->Name()<<","<<build_ns<<std::endl;
  }

 private:
};

template <class KeyType>
const    std::vector<std::string>  Builder<KeyType>::top_layers({"linear", "robust_linear", "linear_spline","cubic","log_linear","normal","log_normal","radix", "radix18", "radix22","radix26","radix28" });
template <class KeyType>
const    std::vector<std::string>  Builder<KeyType>::leaf_layers({"linear", "robust_linear","linear_spline","cubic","log_linear","normal","log_normal" });
template <class KeyType>
const    std::vector<uint32_t >   Builder<KeyType>::submodels({2<<5,2<<6,2<<7,2<<8,2<<9,2<<10});

//const    std::vector<uint32_t >   Builder<KeyType>::submodels({2<<5,2<<6,2<<7,2<<8,2<<9,2<<10,2<<11,2<<12,2<<13,2<<14,2<<15,2<<16,2<<17,2<<18,2<<19,2<<20,2<<21,2<<22,2<<23,2<<24,2<<25});



}

#endif  // TS_BUILDER_H
