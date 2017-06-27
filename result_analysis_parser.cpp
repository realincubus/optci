

#include "result_analysis_parser.hpp"

#include <set>
#include <functional>

Token make_token( std::string& token ) {
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

ASTNode* parse_compare_expr(std::string compare_str ){
  std::vector<Token> tokens;
  std::set<char> operators = {'/', '=' };
  
  for( int i = 0; i < compare_str.size(); i++ ){

    char c = compare_str[i];

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
  ASTNode* root = new ASTNode(ASTNode::Root, root_token);
  build_ast( tokens, root ) ;
  return root;
}

using ASTNodeFunction = std::function<void(ASTNode*)>;

void for_each_node( ASTNode* n, ASTNodeFunction  f)
{
  f(n);
  for( auto& child : n->children ){
    for_each_node( child, f );
  }
}

void for_each_node_of_type( ASTNode* root, ASTNode::Type t, ASTNodeFunction f)
{
  for_each_node( root, [&](ASTNode* n){ 
    if ( n->type == t ) {
      f(n);
    }
  });
}

std::vector<std::string> get_selects_as_filter( ASTNode* root ){
  std::set<std::string> select_vars;
  for_each_node_of_type( root, ASTNode::Select, [&](ASTNode* n){ 
    auto axis_name = n->children.front()->t.token ;
    select_vars.insert(axis_name); 
  });

  if ( select_vars.size() > 1 ) {
    std::cerr << "you have selected more than 2 axis for comparison right now we dont support more than 1" << std::endl;
    std::cerr << "selcted axis:" << std::endl;
    for( auto& select_var : select_vars ){
      std::cerr << select_var << std::endl;
    }
    exit(EXIT_FAILURE);
  }

  std::vector<std::string> ret;
  std::copy( begin(select_vars),
      end(select_vars),
      std::back_inserter(ret)
  );

  return ret;
}








