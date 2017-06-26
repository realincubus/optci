


#include "results_analyzer.hpp"
#include <iostream>
#include <functional>
#include <regex>
#include <fstream>

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

    do_axis_analysis( analysis_result );
    
    ret.emplace_back( analysis_result );
  }

  return ret;
}

using data2d = std::vector<std::vector<double>> ;
using data3d = std::vector<data2d>;

void make_2d_matrix_plot( std::ostream& out, data2d& data ){
  using namespace std;
  static int map_ctr = 0;
  std::string map_name = "$map_"s + to_string(map_ctr++);
  out << map_name << "<< EOD" << std::endl;
  for( auto& plot_data1d : data ){
    for( auto& speedup : plot_data1d ){
      out << speedup << " ";
    }
    out << std::endl;
  }
  out << "EOD" << std::endl;
  out << "set view map" << endl;
  out << "splot '" << map_name << "' matrix with image" << std::endl;
}

void make_gnuplot_multiplot( data3d& data ){

  std::ofstream out("outfile.plt");

  out << "set terminal png" << std::endl;
  out << "set palette defined ( 0 \"red\", 1 \"blue\" )" << std::endl;
  out << "set multiplot layout " << data.size() << ",1 margins 0.1,0.98,0.1,0.98 spacing 0.08,0.08" << std::endl;

  for( auto& plot_data2d : data ){
    // TODO insert the multiplot origin and size calculation here !! not needed right now
    make_2d_matrix_plot( out, plot_data2d );
  }
  out << "unset multiplot" << std::endl;
}

// TODO everything needs to be rewritten to allow hypercubes
// TODO axis names need to be carried on to gnuplot
void generate_gnuplot_matrix_plot( AnalysisResult& result, std::string filter ){

  data3d plot_data3d;

  int max_size[3] = {
    std::numeric_limits<int>::min(),
    std::numeric_limits<int>::min(),
    std::numeric_limits<int>::min()
  };

  // calculate the minimum bounding hypercube dimensions
  for( auto& axis_result : result ){
    std::regex filter_expr(filter, std::regex::grep);
    if ( std::regex_match( axis_result.configuration.name, filter_expr ) ) {
      int ctr = 0;

      for( auto& element : axis_result.configuration.manifestation ){
        int addr = stoi(element.second);
        if ( addr > max_size[ctr] ) {
          max_size[ctr] = addr;
        }
        ctr++;
      }
    }
  }


  plot_data3d.resize( max_size[0] );
  for( auto& plot_data2d : plot_data3d ){
    plot_data2d.resize( max_size[1] ); 
    for( auto& plot_data1d : plot_data2d ){
      plot_data1d.resize(max_size[2]);
    }
  }
  
  for( auto& axis_result : result ){
    std::regex filter_expr(filter, std::regex::grep);
    if ( std::regex_match( axis_result.configuration.name, filter_expr ) ) {
      // all other axis with this name have to have the same configuration
      // what is the dimensionality of this axis
      int dimensionality = axis_result.configuration.manifestation.size();

      // TODO generalize
      double BASE = std::stod(axis_result.sorted_configs[0].second);
      double CHANGED = std::stod(axis_result.sorted_configs[1].second);
      double speedup = BASE/CHANGED;

      int ctr = 0;
      int addr[3];

      for( auto& element : axis_result.configuration.manifestation ){
        addr[ctr] = stoi(element.second)-1;        
        ctr++;
      }

      plot_data3d[addr[0]][addr[1]][addr[2]] = speedup;      

      std::cout << "dimensionality " << dimensionality << std::endl;
      std::cout << "coordinates " << " follows " << std::endl;
      std::cout << "speedup " << speedup << std::endl;
    }
  }
   
  make_gnuplot_multiplot(plot_data3d);

}

struct Token {
  std::string token;
};

Token make_token( std::string& token ) {
  std::cout << "parsed token: " << token << std::endl;
  Token t;
  t.token = token;
  token = "";
  return t;
}

auto parse_var( std::string str, std::string& token) {
  int ctr = 0;
  for( auto& c : str ){
    ctr++;
    // numeric
    if ( std::isalnum( c ) ) {
      token += c;
      continue;
    }
    break;
  }
  return ctr;
}

int parse_op( std::string str, std::string& token) {
  int ctr = 0;
  for( auto& c : str ){
    ctr++;
    if ( std::isspace( c ) || std::isalnum(c) ) {
      break;
    }
    token += c;
  }
  return ctr;

}

struct ASTNode {
  enum Type {
    Divide,
    Select,
    Variable,
    Root
  };
  ASTNode( Type _type, Token& _token) :
    type(_type),
    t(_token)
  {
    std::cout << "new node " << t.token << std::endl; 
  }
  Type type;
  Token t;
  std::vector<ASTNode*> children;
};
void build_ast(std::vector<Token>& tokens, ASTNode* root);

bool handle_divide( std::vector<Token>& tokens, ASTNode* node ) {
   for( int i = 0 ; i < tokens.size(); i++ ){
    auto& token = tokens[i];
    if ( token.token == "/" ) {
      ASTNode* divide_node = new ASTNode(ASTNode::Divide,token);
      node->children.push_back( divide_node );
      // vector for left
      std::vector<Token> token_left;
      std::copy( begin(tokens),
          begin(tokens)+i,
          std::back_inserter(token_left)
      );
      build_ast( token_left, divide_node ); 

      // vector for right
      std::vector<Token> token_right;
      std::copy( begin(tokens)+i+1,
          end(tokens),
          std::back_inserter(token_right)
      );
      build_ast( token_right, divide_node ); 
      return true;
    }
  } 
  return false;
}

bool handle_select( std::vector<Token>& tokens, ASTNode* node ) {
  for( int i = 0 ; i < tokens.size(); i++ ){
    auto& token = tokens[i];
    if ( token.token == "=" ) {
      ASTNode* select_node = new ASTNode(ASTNode::Select,token);
      node->children.push_back( select_node );
      // vector for left
      std::vector<Token> token_left;
      std::copy( begin(tokens),
          begin(tokens)+i,
          std::back_inserter(token_left)
      );
      build_ast( token_left, select_node ); 

      // vector for right
      std::vector<Token> token_right;
      std::copy( begin(tokens)+i+1,
          end(tokens),
          std::back_inserter(token_right)
      );
      build_ast( token_right, select_node ); 
      return true;
    }
  }
  return false;
}

void build_ast(std::vector<Token>& tokens, ASTNode* root) {
#if 0
  std::cout << "tokens" << std::endl;
  for( auto&& t : tokens ){
    std::cout << t.token << " ";
  }
  std::cout << std::endl;
#endif
  if ( !handle_divide( tokens, root ) ){
    if ( !handle_select( tokens, root ) ) {
      root->children.push_back( new ASTNode( ASTNode::Variable, tokens.front() ) );
    }
  }  

}

void print_ast( ASTNode* root, int indent = 0 ) {
  for (int i = 0; i < indent; ++i){
    std::cout << " ";
  }
  std::cout << root->t.token << std::endl;
  for( auto& element : root->children ){
    print_ast( element, indent + 2);
  }
}

auto parse_compare_expr(std::string compare_str ){
  std::vector<Token> tokens;
  std::set<char> operators = {'/', '=' };
  
  for( int i = 0; i < compare_str.size(); i++ ){

    char c = compare_str[i];
    std::cout << "c " << c << std::endl;

    if ( std::isalnum( c ) ){
      std::string token_var;
      int skip = parse_var( compare_str.substr( i ), token_var ); 
      i += skip - 1;
      tokens.emplace_back( make_token( token_var ) );
      continue;
    }  

    if ( operators.find( c ) != operators.end() ) {
      std::string token_op;
      int skip = parse_op( compare_str.substr( i ), token_op );
      i += skip - 1;
      tokens.emplace_back( make_token( token_op ) );
      continue;
    }
  }

  Token root_token;
  root_token.token = "root";
  ASTNode root(ASTNode::Root, root_token);
  build_ast( tokens, &root ) ;
  print_ast( &root );
}

auto compare_results( std::vector<AxisAnalysisResult>& results, std::string compare_str ) {
  
  return 0;
}

#if BUILDING_EXE 
int main(int argc, char** argv){

  // command line argument parsing
  
  // data with defaults
  std::string input_file = "";
  std::string target_axis = "TIME";
  std::string compare = "";
  std::string filter = "";
  
  // parse all switches
  while ( int opt = getopt(argc, argv, "i:t:f:c:") ){
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
    case 'c':{
        compare = optarg;
        break;
    }
    default:{
        fprintf(stderr, "Usage: %s \n",argv[0]);
        exit(EXIT_FAILURE);
    }
      }
  }
  
  YAML::Node root = YAML::LoadFile(input_file);

  auto results = analyze_configuration_matrix( root, target_axis, filter );

  for( auto&& result : results ){
    result.report( std::cout );
  }

  if ( compare != "" ) {
    parse_compare_expr( compare );
    compare_results( results, compare );
  }

  if ( filter != "" ) {
    generate_gnuplot_matrix_plot( results, filter );
  }


  return 0;
}
#endif
