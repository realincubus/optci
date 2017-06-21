
#include "results_accumulator.hpp"

#include <sstream>
#include <fstream>
#include <tuple>

using Node = YAML::Node;

#if 0

  configuration["configuration"].push_back(make_node("compiler","g++"));
  configuration["configuration"].push_back(make_node("sockets", "1"));
  configuration["configuration"].push_back(make_node("cores_per_socket", "2"));
  configuration["configuration"].push_back(make_node("threads_per_cores", "1"));
  configuration["configuration"].push_back(make_node("flags", "-03"));
  configuration["configuration"].push_back(make_node("mpi", "openmpi-1.10"));
  configuration["configuration"].push_back(make_node("time", "1.0"));

#endif

Node build_yaml_root() {
  Node root;
  return root;
}

Node add_yaml_configuration_from_stream(Node root, std::istream& stream){

  Node config;

  // helper function 
  auto make_node = [](auto key, auto value){ 
    Node node;
    node[key] = value;
    return node;
  };  

  std::string line;
  while( getline( stream, line ) ){
    std::stringstream line_stream(line);
    std::string key;
    std::getline( line_stream, key, '=' );
    std::string value;
    std::getline( line_stream, value );
    config["configuration"].push_back(make_node(key,value));
  }

  root.push_back(config);

  return root;
}

void write_yaml_tree( Node root, std::ostream& out ) {
  YAML::Emitter emitter;
  emitter << root;
  out << emitter.c_str() << std::endl;
}

// TODO lets write a program that takes some variables and converts them 
// into a yaml file 
#if BUILDING_LIB
int ra_main(int argc, char** argv){
#else
int main(int argc, char** argv){
#endif

  std::istream* in_file = &std::cin;

  if ( argc > 1 ) {
    if ( std::string(argv[1]) != "-" ){
      in_file = new std::ifstream(argv[1]);
    }
  }

  auto root = build_yaml_root();
  root = add_yaml_configuration_from_stream(root, *in_file );
  write_yaml_tree( root, std::cout );

  return 0;
}
