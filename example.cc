#include <algorithm>
#include <iostream>
#include <vector>

#include "include/rmi/models/balanced_radix.h"
#include "include/rmi/models/cubic_spline.h"
#include "include/rmi/models/linear.h"
#include "include/rmi/models/linear_spline.h"
#include "include/rmi/models/log_normal.h"
#include "include/rmi/models/model.h"
#include "include/rmi/models/normal.h"
#include "include/rmi/models/radix.h"
#include "include/rmi/models/radix_table.h"
#include "include/rmi/models/robust_linear.h"
#include "include/rmi/rmi_models.h"
#include "include/ts/builder.h"
#include "include/ts/common.h"
#include "include/rmi/builder.h"
#include "include/rmi/radix_spline.h"
#include "include/rmi/rmi_spline.h"

using namespace std;

void TrieSplineExample() {
  // Create random keys.
  std::vector<uint64_t> keys(1e6);
  generate(keys.begin(), keys.end(), rand);
  keys.push_back(424242);
  std::sort(keys.begin(), keys.end());

  // Build TS
  uint64_t min = keys.front();
  uint64_t max = keys.back();
  ts::Builder<uint64_t> tsb(min, max, /*spline_max_error=*/32);

  for (const auto& key : keys) tsb.AddKey(key);
  auto ts = tsb.Finalize();

  // Search using TS
  ts::SearchBound bound = ts.GetSearchBound(424242);
  std::cout << "The search key is in the range: [" << bound.begin << ", "
            << bound.end << ")" << std::endl;
  auto start = std::begin(keys) + bound.begin,
       last = std::begin(keys) + bound.end;
  auto pos = std::lower_bound(start, last, 424242) - begin(keys);
  assert(keys[pos] == 424242);
  std::cout << "The key is at position: " << pos << std::endl;
}

//void  TestLinearLinear(){
//  std::vector<uint64_t> keys(65000);
//  std::vector<double> values(65000);
//
//  for (int i = 0; i < values.size(); ++i) {
//    keys[i] = i;
//    values[i] = i;
//  }
//
//  rmi::RMIModels<uint64_t>* model =
//      rmi::RMIModels<uint64_t>::New("linear", "linear", keys, values, 1024);
//}
//
template <typename T>
static vector<T> load_data(const string& filename, bool print = true) {
  vector<T> data;
  ifstream in(filename, ios::binary);
  if (!in.is_open()) {
    cerr << "unable to open " << filename << endl;
    exit(EXIT_FAILURE);
  }
  // Read size.
  uint64_t size;
  in.read(reinterpret_cast<char*>(&size), sizeof(uint64_t));
  data.resize(size);
  // Read values.
  in.read(reinterpret_cast<char*>(data.data()), size * sizeof(T));

  return data;
}

template <class KeyType>
struct Lookup {
  KeyType key;
  uint64_t value;
};




void    TestOne(){
  std::vector<uint64_t> keys(1e6);
  generate(keys.begin(), keys.end(), rand);
  keys.push_back(424242);
  std::sort(keys.begin(), keys.end());


  std::vector<double>  values(keys.size());
  for (int i = 0; i < keys.size(); ++i) {
    values[i]=i;
  }

  std::vector<double>     first_layer_data(values);
  double  datasize = (double)first_layer_data.size();
  for (int i = 0; i < datasize; ++i) {
    first_layer_data[i] = (double)values[i]*(64)/(double )datasize;
  }



  rmi::RMIModels<uint64_t>  *model = rmi::RMIModels<uint64_t >::New("linear","log_linear",keys,values,64,first_layer_data);
  //  for (int i = 0; i < lookups.size(); ++i) {
  rmi::SearchBound bound = model->GetSearchBound(424242);
  std::cout << "The search key is in the range: [" << bound.begin << ", "
            << bound.end << ")" << std::endl;
  auto start = std::begin(keys) + bound.begin,
       last = std::begin(keys) + bound.end;
  auto pos = std::lower_bound(start, last, 424242) - begin(keys);
  assert(keys[pos] == 424242);
  std::cout << "The key is at position: " << pos << std::endl;


  //  }
}

void    TestRadixSpline(){
  std::vector<uint64_t> keys(200000000);
  generate(keys.begin(), keys.end(), rand);
//  keys.push_back(424242);
  std::sort(keys.begin(), keys.end());


  std::vector<double>  values(keys.size());
  for (int i = 0; i < keys.size(); ++i) {
    values[i]=i;
  }

  rmi::RadixSpline<uint64_t >  *model= new rmi::RadixSpline<uint64_t >(keys,values, 50);

}

void    TestBuilder(){
  std::vector<uint64_t> keys(1e6);
  generate(keys.begin(), keys.end(), rand);
  keys.push_back(424242);
  std::sort(keys.begin(), keys.end());


  std::vector<double>  values(keys.size());
  for (int i = 0; i < keys.size(); ++i) {
    values[i]=i;
  }

//  rmi::RMIModels<uint64_t>  *model = rmi::Builder<uint64_t>::Build(keys,values);
////  for (int i = 0; i < lookups.size(); ++i) {
//    rmi::SearchBound bound = model->GetSearchBound(424242);
//    std::cout << "The search key is in the range: [" << bound.begin << ", "
//              << bound.end << ")" << std::endl;
//    auto start = std::begin(keys) + bound.begin,
//         last = std::begin(keys) + bound.end;
//    auto pos = std::lower_bound(start, last, 424242) - begin(keys);
//    assert(keys[pos] == 424242);
//    std::cout << "The key is at position: " << pos << std::endl;


//  }
}


template <class KeyType>
void    TestBuilder(const string& data_file, const string lookup_file,
                 const std::string &first, const std::string &second, uint32_t submods){
  std::vector<KeyType> keys = load_data<KeyType>(data_file);
  std::vector<double>  values(keys.size());
  vector<rmi::Lookup<KeyType>> lookups =load_data<rmi::Lookup<KeyType>>(lookup_file);
  for (int i = 0; i < keys.size(); ++i) {
    values[i]=i;
  }
  std::vector<double>     first_layer_data(values);
  double  datasize = (double)first_layer_data.size();
  for (int i = 0; i < datasize; ++i) {
    first_layer_data[i] = (double)values[i]*(submods)/(double )datasize;
  }

  rmi::RMIModels<KeyType>  *model = rmi::RMIModels<KeyType >::New(first,second,keys,values,submods,first_layer_data);

//  rmi::RMIModels<KeyType>  *model = rmi::Builder<KeyType>::Build(keys,values);
  for (int i = 0; i < lookups.size(); ++i) {
    rmi::SearchBound bound = model->GetSearchBound(lookups[i].key);
    if(bound.begin> lookups[i].value ||
        bound.end  < lookups[i].value) {
      std::cout<<"error"<<std::endl;
    }
  }

}

void  TestRMISpline(){
  std::vector<uint64_t> keys(10000000);
  generate(keys.begin(), keys.end(), rand);
//  keys.push_back(424242);
  std::sort(keys.begin(), keys.end());
  std::vector<double>  values(keys.size());
  for (int i = 0; i < keys.size(); ++i) {
    values[i]=i;
  }

//  rmi::Builder<uint64_t >::Build(keys,values);
//  rmi::RMISpline<uint64_t >  *model = rmi::RMISpline<uint64_t >::New("linear","normal", 64,keys,values,10);
////  model->GetSearchBound(1000);
//  for (int i = 0; i < keys.size(); ++i) {
//    if(i==9844513){
//      std::cout<<"here"<<std::endl;
//    }
//    rmi::SearchBound bond = model->GetSearchBound(keys[i]);
//    assert(i >=bond.begin  && i<= bond.end );
//  }
  std::cout<<"success!"<<std::endl;
}

int main(int argc, char** argv) {
  //  TrieSplineExample();
  //  rmi::BalancedRadix::Test();
//  TestRadixSpline();
//  TestRMISpline();
  if (argc != 6) {
    std::cout << "usage: " << argv[0] << " <data_file> <lookup_file> <first_layer> <second_layer> <submods>" << endl;
    exit(-1);
  }

  const string data_file = argv[1];
  const string lookup_file = argv[2];
  const std::string   first_layer = argv[3];
  const std::string     second_layer = argv[4];
  uint32_t  submods =  atoi(argv[5]);
  if (data_file.find("32") != string::npos) {
    TestBuilder<uint32_t >(data_file,lookup_file,  first_layer, second_layer,  submods);
  } else {
    TestBuilder<uint64_t >(data_file,lookup_file,  first_layer, second_layer,  submods);
  }

//  TestBuilder();

//  ??????
//  std::vector<uint64_t> keys(65000);
//  std::vector<double> values(65000);
//
//  for (int i = 0; i < values.size(); ++i) {
//    keys[i] = i;
//    values[i] = i;
//  }


  return 0;
}
