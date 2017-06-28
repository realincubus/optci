


#include "results_analyzer.hpp"
#include "result_analysis_parser.hpp"
#include "function_analysis.hpp"
#include "result_analysis_plot.hpp"

#include <iostream>
#include <functional>
#include <regex>
#include <fstream>

using namespace std;
// 
//
// LOOP FUNCTIONS
//
//


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

//
//
// get functions
//
//

auto get_axis_value( NodeRef axis ) {
  auto [ name, value ] = get_name_and_value( axis );
  return value; 
}


using XYPair = std::pair<std::string,std::string>;
using ConfigMap = std::map<AxisConfiguration,std::set<XYPair>>; 

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

      auto axis_configuration = make_axis_configuration( config_a, axis );
      possible_configs[axis_configuration].emplace( axis_a_value, axis_a_time_value ); 
      //}
    }

  });
  //std::cout << std::endl;
}


void do_axis_analysis ( AxisAnalysisResult& analysis_result ){
  
  std::sort( begin(analysis_result.sorted_configs), end(analysis_result.sorted_configs), [](auto& a, auto& b){ 
      return std::get<0>(a) < std::get<0>(b) ; 
  });


  bool is_numeric = true;
  // check that the first dimension is numeric 
  // TODO check if double works to
  for( auto&& config : analysis_result.sorted_configs ){
    try {
      stoi(config.first);
    }catch (...) {
      is_numeric = false;
      break;
    }
  }
   
  // TODO check that second dimension is numeric

  if ( !is_numeric ) return;

  std::vector<std::pair<int,double>> xy_value_row;
  for( auto& config : analysis_result.sorted_configs ){
    auto& [a,b] = config;
    xy_value_row.emplace_back( stoi(a),stod(b) ); 
  }


  double relative_tolerance = 0.05;

  analysis_result.no_change = one_d_function_analysis::no_change( xy_value_row, relative_tolerance );

  if ( !analysis_result.no_change ) {
    
    analysis_result.best = one_d_function_analysis::minimum_combination( xy_value_row );

    // convert to simple row of values
    std::vector<double> y_value_row;
    for( auto&& config : analysis_result.sorted_configs ){
      auto& [a,b] = config;
      y_value_row.push_back( stod(b) ); 
    }
    
    analysis_result.monotonic_decrease = one_d_function_analysis::is_monotonically_decreasing( y_value_row );
  }


}


void AxisAnalysisResult::report ( std::ostream& out ){

  std::cout << "for axis: " << configuration << std::endl;
  for( auto& config : sorted_configs ){
    auto& [a,b] = config;
    std::cout << a << " " << b << std::endl;    
  }
  std::cout << "---" << std::endl;

  if ( no_change ) {
    // TODO refactor out 
    double relative_tolerance = 0.05;
    std::cout << "the changes that are caused by axis " << configuration << " do not result in a TIME change tolerance = " << relative_tolerance << std::endl;
  }else{

    if ( best ) {
      std::cout << "best combination is " << best->first << " " << best->second << std::endl;
    }

    if ( monotonic_decrease ) {
      std::cout << "if " << configuration << " is increased TIME also falls monotonically" << std::endl;
    }
  }

  std::cout << std::endl;

}

std::vector<AxisAnalysisResult> analyze_configuration_matrix( YAML::Node root, std::string target_axis, std::string filter ){

  std::vector<AxisAnalysisResult> ret;

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

    AxisAnalysisResult analysis_result;
    analysis_result.configuration = axis_configuration;

    // apply filter to not analyze things that are not interesting
    if ( filter != "" ){
      // TODO make customizable regex type
      std::regex filter_expr(filter, std::regex::grep);
      if ( !std::regex_match( analysis_result.configuration.name, filter_expr ) ) {
        continue;
      }
    }

    for( auto&& config : axis_set.second ){
      analysis_result.sorted_configs.push_back( config );
    }

    // TODO just do if requested 
    do_axis_analysis( analysis_result );
    
    ret.emplace_back( analysis_result );
  }

  return ret;
}


int get_dimensionality( AnalysisResult& result ){
  if ( !result.empty() ) {
    auto& first = result.front();
    return first.configuration.manifestation.size();
  }else{
    return -1;
  }
}


void for_each_node_bottom_up( ASTNode* n, std::function<void(ASTNode*)> f ){
  for( auto& child : n->children ){
    for_each_node_bottom_up( child, f );
  }
  f(n); 
}

MultiDArray<double> generate_multidimensional_dense_array( AnalysisResult& result, ASTNode* ast) {

  // get dimensionality
  int dimensionality = get_dimensionality( result ); 

  // generate plot_data_array
  MultiDArray<double> mda;

  // get the extents from the results
  std::vector<int> extents;

  extents.resize(dimensionality);
  std::fill( begin(extents), end(extents), std::numeric_limits<int>::min() );

  for( auto& axis_result : result ){
    int ctr = 0;

    for( auto& element : axis_result.configuration.manifestation ){
      int addr = stoi(element.second);
      if ( addr > extents[ctr] ) {
        extents[ctr] = addr;
      }
      ctr++;
    }
  }

  mda.extents( extents );

  // apply the AST to the results and store the result in the MultiDMatrix
  
  for( auto& axis_result : result ){
    for_each_node_bottom_up( ast, [&](ASTNode* n){ 
      if (n->type == ASTNode::Select ){
        //std::cout << "select node" << std::endl;
        auto* rhs = n->children[1];
        auto get_value = [&](auto side){ 
          for( auto& config : axis_result.sorted_configs ){
            if ( config.first == side->t.token ) {
              return config.second;
            }
          }
          return std::string("not found");
        };

        auto rhs_value = get_value( rhs );
        //std::cout << "rhs value for " << rhs->t.token << " " << rhs_value << std::endl;
        n->result = stod( rhs_value );
      }
      if (n->type == ASTNode::Divide ){
        //std::cout << "divide node" << std::endl;
        auto* lhs = n->children[0];
        auto* rhs = n->children[1];

        auto res = lhs->result / rhs->result;
        //std::cout << "setting result " << res << std::endl;
        n->result = res;
      }
      if ( n->type == ASTNode::Root ) {
        //std::cout << "root node" << std::endl;
        // get the position 
        int ctr = 0;
        MultiDArray<double>::address_t addr;
        addr.resize(dimensionality);

        for( auto& element : axis_result.configuration.manifestation ){
          addr[ctr] = stoi(element.second)-1;        
          ctr++;
        }

        auto* child = n->children[0];
        mda.set_value( addr, child->result );

      }
    });
  }
  return mda;
}



auto handle_results = [](auto& results){ 
  for( auto&& result : results ){
    result.report( std::cout );
  }    
};

int compare_data( NodeRef root, const Options& options ){
  auto ast = parse_compare_expr( options.compare );
  auto selects = get_selects_as_filter( ast );
  
  if (!selects.empty()) {
    // TODO for the time beeing 
    auto select = selects.front();
    auto filter = ".*"s + select + ".*";

    auto results = analyze_configuration_matrix( root, options.target_axis, filter );

    handle_results(results);

    auto mda = generate_multidimensional_dense_array( results, ast );
    if ( options.plot_file != "" ) {
      generate_gnuplot_matrix_plot( mda );
    } 

    if ( options.greater != "" ) {
      auto max = mda.max();
      auto greater_val = stod( options.greater );
      if ( max > greater_val ) {
        std::cout << "the matix contains a value that is greater then " << greater_val << " -> " << max << std::endl;
        return EXIT_SUCCESS;
      }else{
        std::cout << "the matix does not contain a value that is greater then " << greater_val << " maximum is " << max << std::endl;
        return EXIT_FAILURE;
      }
    }

  }

  return 0;
}

#if BUILDING_EXE 
int main(int argc, char** argv){

  // command line argument parsing

  Options options;
  
  // parse all switches
  while ( int opt = getopt(argc, argv, "i:t:f:c:p:g:") ){
    if ( opt == -1 ) break;
    switch(opt) {
      case 'i':{
          options.input_file = optarg;
          break;
      }
      case 't':{
          options.target_axis = optarg;
          break;
      }
      case 'f':{
          options.filter = optarg;
          break;
      }
      case 'c':{
          options.compare = optarg;
          break;
      }
      case 'p' :{
          options.plot_file = optarg;
          break;
      }
      case 'g' :{
          options.greater = optarg;
          break;
      }
      default:{
          fprintf(stderr, "Usage: %s \n",argv[0]);
          exit(EXIT_FAILURE);
      }
    }
  }
  
  YAML::Node root = YAML::LoadFile(options.input_file);


  if ( options.compare != "" ) {
    compare_data( root, options );
  }

  if ( options.filter != "" ) {
    auto results = analyze_configuration_matrix( root, options.target_axis, options.filter );

    handle_results( results );
    return 0;

  }

  return 0;
}
#endif







