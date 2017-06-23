


#include "results_analyzer.hpp"
#include <iostream>
#include <functional>
#include <regex>

using UnaryFunction = std::function<void( const YAML::Node&)>;
using NodeRef = const YAML::Node&;
using NodePtr = const YAML::Node*;

void for_each_configuration( YAML::Node root, UnaryFunction f ) {
   for (int i = 0; i < root.size() ; ++i){
    const YAML::Node& node = root[i];
    if ( node["configuration"] ) { 
      f( node["configuration"] );
    }
   }
}


void for_each_axis( YAML::Node root, UnaryFunction f)
{
   for (int i = 0; i < root.size() ; ++i){
    const YAML::Node& node = root[i];
    f(node);
   }
}

void for_each_axis_but( YAML::Node root, const YAML::Node& exclude, UnaryFunction f)
{
  for_each_axis( root, [&](NodeRef axis){ 
    if ( ! axis.is( exclude ) ) {
      f( axis );
    }
  });
}

auto get_name_and_value( NodeRef axis ) {
  std::string axis_name;
  std::string axis_value;

  // TODO there should be just one but perhaps check this 
  for ( auto i = axis.begin(); i != axis.end(); i++){
    axis_name = i->first.as<std::string>();
    axis_value = i->second.as<std::string>();
  }

  return std::make_tuple( axis_name, axis_value );
}

bool is_axis_value_different_in( NodeRef config_b, std::string axis_name, std::string axis_value, std::string minimize_axis = "TIME" )
{
  bool is_difference = false;

  for_each_axis( config_b, [&](NodeRef config_b_axis){ 
    auto [ axis_b_name, axis_b_value ] = get_name_and_value( config_b_axis );

    if ( axis_name == axis_b_name && axis_name != minimize_axis ) {
      if ( axis_value != axis_b_value ) {
        is_difference = true;
      }
    }
  });

  return is_difference;
}

void for_each_axis_named( NodeRef config, std::string name, std::function<void(NodeRef)> f ) {

  for_each_axis( config, [&](const YAML::Node& axis){ 
      // compare with config_b
      auto [ axis_name, axis_value ] = get_name_and_value( axis );
      if ( axis_name == name ) {
        f( axis );
      } 
  });
}

auto get_axis_value( NodeRef axis ) {
  auto [ name, value ] = get_name_and_value( axis );
  return value; 
}

using XYPair = std::pair<std::string,std::string>;

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

using ConfigMap = std::map<AxisConfiguration,std::set<XYPair>>; 

std::string make_config_name( NodeRef config, NodeRef base_axis, std::string minimize_axis = "TIME" ) {
  using namespace std;
  auto [ name, value ] = get_name_and_value(base_axis);
  std::string ret = name + "[";
  bool first = true;
  for_each_axis_but( config, base_axis, [&](NodeRef axis){ 
    auto [ name, value ] = get_name_and_value( axis );
    if ( name == minimize_axis ) return; 
    if ( first ) {
      first = false;
    }else{
      ret += ", ";
    }
    ret += name + "="s + value;

  });

  ret += "]";
  return ret;
}

AxisConfiguration make_axis_configuration( NodeRef config, NodeRef base_axis, std::string minimize_axis = "TIME" ) {
  using namespace std;
  auto [ name, value ] = get_name_and_value(base_axis);

  AxisConfiguration ret;
  ret.name = name;
  
  for_each_axis_but( config, base_axis, [&](NodeRef axis){ 
    auto [ name, value ] = get_name_and_value( axis );
    if ( name == minimize_axis ) return; 
    ret.manifestation.emplace_back( name, value);
  });

  return ret;
}

void compare_configs( NodeRef config_a, NodeRef config_b, ConfigMap& possible_configs, std::string minimize_axis = "TIME" ) {
  //std::cout << "comparing configs "  << std::endl; 
  for_each_axis( config_a, [&](NodeRef axis){ 

    auto [ axis_name, axis_a_value ] = get_name_and_value( axis );

    if ( axis_name == minimize_axis ) return;

    //std::cout << "axis " << axis_name << std::endl;
    bool is_difference = false;

    // TODO loop over all other axis in config_a and compare them with config_b -> they have to have identical values
    for_each_axis_but( config_a, axis, [&](const YAML::Node& other_axis){ 
      // compare with config_b
      auto axis_a_name_and_value = get_name_and_value( other_axis );
      auto axis_a_name = std::get<0>(axis_a_name_and_value);
      auto axis_a_value = std::get<1>(axis_a_name_and_value);
      if (is_axis_value_different_in( config_b, axis_a_name, axis_a_value ) ){
        //std::cout << "  differs " << axis_a_name << " in other conf " << std::endl;
        is_difference = true;
      }
      
    });

    if ( is_difference == false ) {
      //std::cout << "  no difference collecting to " << axis_name << std::endl;
      //std::cout << config_a << std::endl;
      //std::cout << std::endl;
      //std::cout << config_b << std::endl;
      // check for change in this axis
      //if ( is_axis_value_different_in( config_b, axis_name, axis_a_value ) ) { 
      //std::cout << "there is no change but in this axis (" << axis_name << ")" << std::endl;

      std::string axis_a_time_value;
      std::string axis_b_time_value;
      for_each_axis_named( config_a, "TIME", [&](NodeRef axis){ 
        axis_a_time_value = get_axis_value( axis ); 
      });

      for_each_axis_named( config_b, "TIME", [&](NodeRef axis){ 
        axis_b_time_value = get_axis_value( axis ); 
      });

      std::string axis_b_value;
      for_each_axis_named( config_b, axis_name, [&](NodeRef axis){ 
        axis_b_value = get_axis_value( axis ); 
      });

      auto name = make_config_name( config_a, axis );
      auto axis_configuration = make_axis_configuration( config_a, axis );
      possible_configs[axis_configuration].emplace( axis_a_value, axis_a_time_value ); 
      //}
    }

  });
  //std::cout << std::endl;
}

namespace one_d_function_analysis{

  template < typename U, typename P >
  bool adjacent_compare( std::vector<U>& v, P p ){

    U last;

    bool first = true;
    for( auto& y : v ){
      if (first){
        last = y;
        first = false;
      }else{
        if ( p( y, last ) ) {
          return false;
        } 
      }
    }
    return true;
  }

  // very strict comparisons 
  template < typename U >
  bool is_monotonically_decreasing( std::vector<U>& v ){
    return adjacent_compare( v, std::greater_equal<U>() );
  }

  template < typename U >
  bool is_monotonically_increasing( std::vector<U>& v ){
    return adjacent_compare( v, std::less_equal<U>() );
  }

template < typename U, typename V >
  auto maximum_combination( std::vector<std::pair<U,V>>& v ){
    std::pair<U,V>* ret = nullptr;
    V max = std::numeric_limits<V>::lowest();
    for( auto& pair : v ){
      auto [x,y] = pair;
      if ( y > max ) {
        ret = &pair;
        max = y;
      }
    }
    return ret;
  }

  template < typename U, typename V >
  auto minimum_combination( std::vector<std::pair<U,V>>& v ){
    std::pair<U,V>* ret = nullptr;
    V min = std::numeric_limits<V>::max();
    for( auto& pair : v ){
      auto [x,y] = pair;
      if ( y < min ) {
        ret = &pair;
        min = y;
      }
    }
    return ret;
  }

  template < typename U, typename V >
  auto no_change( std::vector<std::pair<U,V>>& v, double relative_tolerance ){
    auto min = minimum_combination( v );
    auto max = maximum_combination( v );
    
    if ( min && max ) {
      auto minv = min->second;
      auto maxv = max->second;
      auto distance = maxv-minv;
      //std::cout << "distance " << distance << " absolute tolerance " << relative_tolerance * maxv << std::endl;
      if ( distance < (relative_tolerance * maxv) ) {
        return true;
      }
    }

    return false;
  }


}
  
// the result for each axis
struct AnalysisResult {
  
};

void analyze_configuration_matrix( YAML::Node root, std::string target_axis, std::string filter ){

  // TODO compare configurations with each other 
  //      search along the axis
  ConfigMap possible_configs;

  for_each_configuration( root, [&]( const YAML::Node& config_a){
    for_each_configuration( root, [&]( const YAML::Node& config_b){
      if ( config_a.is( config_b ) ) return;
      compare_configs( config_a, config_b, possible_configs );
    });
  });


  for( auto&& axis_set : possible_configs ){


    auto axis_configuration = axis_set.first;
    auto axis_name = axis_configuration.name;

    // apply filter to not analyze things that are not interesting
    if ( filter != "" ){
      // TODO make customizable regex type
      std::regex filter_expr(filter, std::regex::grep);
      // TODO remove after axis name rewrite 
      if ( !std::regex_match( axis_name, filter_expr ) ) {
        continue;
      }
    }
    
    std::vector<XYPair> sorted_configs;
    for( auto&& config : axis_set.second ){
      sorted_configs.push_back( config );
    }
    
    std::sort( begin(sorted_configs), end(sorted_configs), [](auto& a, auto& b){ return std::get<0>(a) < std::get<0>(b) ; } );

    std::cout << "for axis: " << axis_configuration << std::endl;
    for( auto& config : sorted_configs ){
      auto& [a,b] = config;
      std::cout << a << " " << b << std::endl;    
    }
    std::cout << "---" << std::endl;

    bool is_numeric = true;
    // TODO check that the first dimension is numeric 
    for( auto&& config : sorted_configs ){
      try {
        stoi(config.first);
      }catch (...) {
        is_numeric = false;
        break;
      }
    }
     
    // TODO check that second dimension is numeric

    if ( !is_numeric ) continue;

    std::vector<std::pair<int,double>> xy_value_row;
    for( auto& config : sorted_configs ){
      auto& [a,b] = config;
      xy_value_row.emplace_back( stoi(a),stod(b) ); 
    }


    double relative_tolerance = 0.05;
    if ( one_d_function_analysis::no_change( xy_value_row, relative_tolerance ) ) {
      std::cout << "the changes that are caused by axis " << axis_name << " do not result in a TIME change tolerance = " << relative_tolerance << std::endl;
    }else{
      
      if ( auto best = one_d_function_analysis::minimum_combination( xy_value_row ) ) {
        std::cout << "best combination is " << best->first << " " << best->second << std::endl;
      }

      // convert to simple row of values
      std::vector<double> y_value_row;
      for( auto&& config : sorted_configs ){
        auto& [a,b] = config;
        y_value_row.push_back( stod(b) ); 
      }
      
      if ( one_d_function_analysis::is_monotonically_decreasing( y_value_row ) ) {
        // TODO replace CORES with the variable from a after shrinking to another data structure
        std::cout << "if " << axis_name << " are increased TIME also falls monotonically" << std::endl;
      }
    }

    std::cout << std::endl;

  }

}


#if BUILDING_EXE 
int main(int argc, char** argv){

  // command line argument parsing
  
  // data with defaults
  std::string input_file = "";
  std::string target_axis = "TIME";
  std::string filter = "";
  
  // parse all switches
  while ( int opt = getopt(argc, argv, "i:t:f:") ){
    if ( opt == -1 ) break;
    switch(opt) {
    case 'i':{
        input_file = optarg;
        break;
    }
    case 't':{
        target_axis = optarg;
        break;
    }
    case 'f':{
        filter = optarg;
        break;
    }
    default:{
        fprintf(stderr, "Usage: %s \n",argv[0]);
        exit(EXIT_FAILURE);
    }
      }
  }
  
  YAML::Node root = YAML::LoadFile(input_file);

  analyze_configuration_matrix( root, target_axis, filter );

  return 0;
}
#endif
