#pragma once

#include <string>
#include <vector>
#include <iostream>

struct Token {
  std::string token;
};

// TODO change to use unique_ptr
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
    //std::cout << "new node " << t.token << std::endl; 
  }
  Type type;
  Token t;
  double result;
  std::vector<ASTNode*> children;
};

void build_ast(std::vector<Token>& tokens, ASTNode* root);

ASTNode* parse_compare_expr(std::string compare_str );
std::vector<std::string> get_selects_as_filter( ASTNode* root );

