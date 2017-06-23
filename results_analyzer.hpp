
#pragma once
#include <yaml-cpp/yaml.h>
#include <string>

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

auto operator < ( const AxisConfiguration& lhs, const AxisConfiguration& rhs){
  return lhs.to_string() < rhs.to_string();
}

auto& operator << ( std::ostream& rhs, AxisConfiguration lhs){
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

using AnalysisResult = std::vector<AxisAnalysisResult>;
AnalysisResult analyze_configuration_matrix( YAML::Node root, std::string target_axis = "TIME", std::string filter = "" );

void generate_gnuplot_matrix_plot(AnalysisResult& result, std::string filter);
