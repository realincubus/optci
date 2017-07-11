
#pragma once
#include <yaml-cpp/yaml.h>
#include <string>
#include <iostream>

#include "Trilean.hpp"


struct AxisConfiguration {
  std::string name;
  std::vector<std::pair<std::string,std::string>> manifestation;

  std::string to_string() const{
    using namespace std;
    std::string ret;
    ret = name + "[";
    bool first = true;
    for( auto& [ key, value ] : manifestation ){
      if ( first ) {
        first = false;
      }else{
        ret += ", ";
      }
      ret += key + "="s + value;
    }
    ret += "]";
    return ret;
  }
  
};

inline auto operator < ( const AxisConfiguration& lhs, const AxisConfiguration& rhs){
  return lhs.to_string() < rhs.to_string();
}

inline auto& operator << ( std::ostream& rhs, AxisConfiguration lhs){
  rhs << lhs.to_string();
  return rhs;
}

// the result for each axis
struct AxisAnalysisResult {
  using XYPair = std::pair<std::string,std::string>;
  AxisConfiguration configuration;
  std::vector<XYPair> sorted_configs; // to be sorted 
  // TODO make something better to autoconvert from bool

  Trilean no_change;
  Trilean monotonic_decrease;
  Trilean monotonic_increase;
  Trilean has_quadratic_shape;
  Trilean has_strided_monotonic_decrease;
  Trilean has_strided_monotonic_increase;

  std::pair<int,double>* best = nullptr;

  void report ( std::ostream& out );

};

template <typename T>
struct MultiDArray {
  int dimensions;
  typedef std::vector<int> extent_t;
  typedef std::vector<int> address_t;

  void extents( std::vector<int>& extents, int i = 0 ){

    dimension = extents.size() - i;
    if ( i >= extents.size() ) return;

    sub_dimension = new std::vector<MultiDArray<T>>();
    sub_dimension->resize(extents[i]);
    union_is_value = false;
    for( auto& dimension : *sub_dimension ){
      dimension.extents( extents, i + 1); 
    }
  }
  
  T get_value( address_t addr, int i = 0 ){
    if ( union_is_value ){
      return value;
    }else{
      return (*sub_dimension)[addr[i]].get_value(addr,i+1);
    }
  }

  void set_value( address_t addr, T _value, int i = 0 ){
    if ( union_is_value ){
      value = _value;
    }else{
      (*sub_dimension)[addr[i]].set_value(addr,_value,i+1);
    }
  }

  int size(){
    if ( sub_dimension ) {
      return sub_dimension->size();
    }else{
      return 1;
    }
  }

  auto begin(){
    return sub_dimension->begin();
  }

  auto end(){
    return sub_dimension->end();
  }

  T max (){
    T max_val = std::numeric_limits<T>::lowest();
    if ( sub_dimension ) {
      for( auto& sub_dimension : *sub_dimension ){
        auto max_subd = sub_dimension.max();
        if ( max_subd > max_val ) {
          max_val = max_subd;
        }
      }
    }else{
      return value;
    }
    return max_val;
  }

  int dimension = -1;
  bool union_is_value = true;
  // TODO put this in a variant with T
  std::vector<MultiDArray>* sub_dimension = nullptr;
  T value;
};

using AnalysisResult = std::vector<AxisAnalysisResult>;
AnalysisResult analyze_configuration_matrix( YAML::Node root, std::string target_axis = "TIME", std::string filter = "" );

using UnaryFunction = std::function<void( const YAML::Node&)>;
using NodeRef = const YAML::Node&;
using NodePtr = const YAML::Node*;

struct Options {
  std::string input_file = "";
  std::string target_axis = "TIME";
  std::string compare = "";
  std::string filter = "";
  std::string plot_file = "";
  // TODO remove after new syntax was introduced for c flag
  std::string greater = "";
  bool accumulate = false;
};

int compare_data( NodeRef root, const Options& options );
void generate_gnuplot_matrix_plot(AnalysisResult& result, std::string filter);
