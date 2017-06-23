
#pragma once
#include <yaml-cpp/yaml.h>
#include <string>

void analyze_configuration_matrix( YAML::Node root, std::string target_axis = "TIME", std::string filter = "" );
