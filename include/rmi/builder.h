//
// Created by xxp on 2021/10/18.
//

#ifndef TS_BUILDER_H
#define TS_BUILDER_H
#include <thread>

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
struct Lookup {
  KeyType key;
  uint64_t value;
};

static   std::map<std::string,uint64_t >    test_elapse;

template <class KeyType>
class   Builder{
 public:
  const static   std::vector<std::string>  top_layers;//{"linear", "robust_linear", "linear_spline","cubic","loglinear","normal","lognormal","radix", "radix18", "radix22","radix26","radix28" };
  const static   std::vector<std::string>  leaf_layers;//{"linear", "robust_linear","linear_spline","cubic","loglinear","normal","lognormal" };
  const static   std::vector<uint32_t >   submodels;//{2<<5,2<<6,2<<7,2<<8,2<<9,2<<10,2<<11,2<<12,2<<13,2<<14,2<<15,2<<16,2<<17,2<<18,2<<19,2<<20,2<<21,2<<22,2<<23,2<<24,2<<25};

  static   void  Run(int k,const std::vector<KeyType> &keys, const std::vector<double > &values,
                                 const std::vector<Lookup<KeyType> >  &tests){
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
        uint64_t  test_tim=TestModel(tests,model);
        std::ostringstream   test_name;
        test_name<<top_layers[i]<<"-"<<leaf_layers[j]<<"-"<<submodels[k];
        test_elapse.insert(std::map<std::string, uint64_t >::value_type(test_name.str(), test_tim));
        delete model;
        std::cout<<"run result:"<< test_name.str()<<","<<test_tim <<std::endl;
        //          model->DumpLayerErr();
      }
    }

  }

  static   void  RunOneLayer(const std::vector<KeyType> &keys, const std::vector<double > &values,
                  const std::vector<Lookup<KeyType> >  &tests){
      for (int j = 0; j < leaf_layers.size(); ++j) {
        RMISpline<KeyType>* model = RMISpline<KeyType>::New(
            leaf_layers[j], "",0, keys, values,10);
        assert(model);
        //计算时间
        uint64_t  test_tim=TestModel(tests,model);
        std::ostringstream   test_name;
        test_name<<leaf_layers[j];
        test_elapse.insert(std::map<std::string, uint64_t >::value_type(test_name.str(), test_tim));
        delete model;
        std::cout<<"run result:"<< test_name.str()<<","<<test_tim <<std::endl;
        //          model->DumpLayerErr();
      }

  }


  static  RMIModels<KeyType>  *Build(const std::vector<KeyType> &keys, const std::vector<double > &values,
                                   const std::vector<Lookup<KeyType> >  &tests){
    std::map<std::string, uint64_t >  test_elapse;
    //根据数据选择最适宜的模型进行计算。
    std::vector<std::thread >    threads;

    for (int i = 0; i < leaf_layers.size(); ++i) {
      threads.push_back(std::thread(RunOneLayer,std::ref(keys),std::ref(values),std::ref(tests)));
    }
    for (int k = 0; k < submodels.size(); ++k) {
      //在这里方便对value进行一次缩放多次使用吧
      threads.push_back(std::thread(  Run,k,std::ref(keys),std::ref(values),std::ref(tests)));
    }
    for (int i = 0; i < threads.size(); ++i) {
      threads[i].join();
    }
    typedef std::pair<std::string, int> MyPairType;
    struct CompareSecond
    {
      bool operator()(const MyPairType& left, const MyPairType& right) const
      {
        return left.second < right.second;
      }
    };

    std::map<std::string, uint64_t >::iterator ret=std::min_element(  test_elapse.begin(),test_elapse.end(),CompareSecond());
    std::cout<<"rmi final result:"<<ret->first<<",elapse="<<ret->second<<std::endl;
  }

  static uint64_t TestModel(const std::vector< Lookup<KeyType> > &tests , RMISpline<KeyType> *model){
    auto  begin = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < tests.size(); ++i) {
      rmi::SearchBound bond = model->GetSearchBound(tests[i].key);
      assert(tests[i].value >=bond.begin  && tests[i].value<= bond.end );
    }
    auto  end = std::chrono::high_resolution_clock::now();
    uint64_t build_ns =
        std::chrono::duration_cast<std::chrono::nanoseconds>(end - begin)
            .count();
//    std::cout<<"outputresult:"<<model->Name()<<","<<build_ns<<std::endl;
    return  build_ns;
  }

 private:
};

template <class KeyType>
const    std::vector<std::string>  Builder<KeyType>::top_layers({"linear", "robust_linear", "linear_spline","cubic","log_linear","normal","log_normal","radix", "radix18", "radix22","radix26","radix28" });
template <class KeyType>
const    std::vector<std::string>  Builder<KeyType>::leaf_layers({"linear", "robust_linear","linear_spline","cubic","log_linear","normal","log_normal" });
template <class KeyType>
const    std::vector<uint32_t >   Builder<KeyType>::submodels({2<<5,2<<6,2<<7,2<<8,2<<9,2<<10});
//template <class KeyType>
//std::map<std::string,uint64_t >    Builder<KeyType>::test_elapse;
//const    std::vector<uint32_t >   Builder<KeyType>::submodels({2<<5,2<<6,2<<7,2<<8,2<<9,2<<10,2<<11,2<<12,2<<13,2<<14,2<<15,2<<16,2<<17,2<<18,2<<19,2<<20,2<<21,2<<22,2<<23,2<<24,2<<25});



}

#endif  // TS_BUILDER_H
