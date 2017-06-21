


#include "results_analyzer.hpp"
#include <iostream>

using UnaryFunction = std::function<void( const YAML::Node&)>;
using NodeRef = const YAML::Node&;

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

void compare_configs( NodeRef config_a, NodeRef config_b, std::string minimize_axis = "TIME" ) {
  std::cout << "comparing configs "  << std::endl; 
  for_each_axis( config_a, [&](NodeRef axis){ 

    bool is_difference = false;

    // TODO loop over all other axis in config_a and compare them with config_b -> they have to have identical values
    for_each_axis_but( config_a, axis, [&](const YAML::Node& other_axis){ 
      // compare with config_b
      auto axis_a_name_and_value = get_name_and_value( other_axis );
      auto axis_a_name = std::get<0>(axis_a_name_and_value);
      auto axis_a_value = std::get<1>(axis_a_name_and_value);
      is_difference = is_axis_value_different_in( config_b, axis_a_name, axis_a_value );
      
    });

    if ( is_difference == false ) {
      auto [ axis_name, axis_value ] = get_name_and_value( axis );
      // TODO check for change in this axis
      if ( is_axis_value_different_in( config_b, axis_name, axis_value ) ) { 
        std::cout << "there is no change but in this axis (" << axis_name << ")" << std::endl;
        // TODO now implement the minimize axis search 
        // perhaps simply collect all configs that enter this point ? and sort them by minimal time ? 
      }
    }

  });
}
  
void analyze_configuration_matrix( YAML::Node root ){


  // TODO compare configurations with each other 
  //      search along the axis

  for_each_configuration( root, [&]( const YAML::Node& config_a){
    for_each_configuration( root, [&]( const YAML::Node& config_b){
      if ( config_a.is( config_b ) ) return;
      compare_configs( config_a, config_b );
      
    });
  });
}
