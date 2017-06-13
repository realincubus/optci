

#include "ConfigurationParser.hpp"
#include <gtest/gtest.h>
#include <experimental/filesystem>
#include <yaml-cpp/yaml.h>

namespace fs = std::experimental::filesystem;

TEST( ADD_CONFIG_TO_PHASES, Positive ) {

  // TODO remove this hack;
  fs::path p = "../../bin_optci";

  auto install_dir = canonical(p).remove_filename();
  std::cout << "install_dir " << install_dir << std::endl;
  g_base_dir = install_dir;

  fs::path hook_folder = "config";
  fs::path phases_folder = hook_folder / "phases";
  fs::path files_folder = hook_folder / "files";

  g_files_folder = canonical(files_folder);
  g_config_file = canonical(hook_folder / "config.yaml" );

  Phase root("config/phases");

  parse_folder( root.sub_phases, phases_folder );

  // this needs to go into the main program
  if (!fs::exists(g_config_file) )  {
    std::cout << "no config file provided not adding extra info to phases" << std::endl;
    return;
  } 

  YAML::Node config = YAML::LoadFile(g_config_file);


  std::vector<std::string> phases;
  if (config["scaling_phases"]) {
    const YAML::Node& sp = config["scaling_phases"];
    phases = sp.as<std::vector<std::string>>();
  }


  ConfigurationMatrix matrix;
  if (config["matrix"]){
    const YAML::Node& m = config["matrix"];
    for (int i = 0 ; i < m.size() ; i++){
      const YAML::Node& n = m[i];
      std::cout << "n" << n["name"].as<std::string>() << std::endl; 
      auto axis_name = n["name"].as<std::string>();
      Axis a(axis_name);

      auto v = n["values"].as<std::vector<std::string>>();
      for( auto& e : v ){
        std::cout << e << std::endl; 
        a.add_value(e);
      }
      matrix.add_axis(a);
    }
  } 

  for( auto& phase : phases ){
    root.for_each_phase( phase, [&](Phase& p){ 
        std::cout << "setting run configuration" << std::endl;
      p.set_run_configuration(matrix);
    });
  }


  std::vector<std::string> paths;
  root.execute(paths);
  
  //add_config_information( config, root );


}


