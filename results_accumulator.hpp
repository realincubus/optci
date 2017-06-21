
#pragma once
#include <yaml-cpp/yaml.h>
#include <iostream>

YAML::Node build_yaml_root();
YAML::Node add_yaml_configuration_from_stream( YAML::Node root, std::istream& stream);
void write_yaml_tree( YAML::Node root, std::ostream& out ) ;
