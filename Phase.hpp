#pragma once

#include <string>
#include <fstream>
#include <experimental/filesystem>
#include <functional>
#include <set>

#include "ConfigurationMatrix.hpp"


// TODO remove global var
extern std::experimental::filesystem::path g_base_dir;
extern std::string g_files_folder;
extern std::string g_config_file;
extern int g_phase_id;
extern int g_max_phases;

struct Phase {
  Phase( std::string phase_name ); 

  void print(int indent = 0); 
  void execute (std::vector<std::string>& paths); 
  std::vector<Phase> sub_phases;

  std::string get_name(); 
  void set_run_configuration( ConfigurationMatrix& m); 

  void for_each_phase( std::string p_name, std::function<void(Phase&)> f);
  void pass_down_matrix();
  void merge_matrix( ConfigurationMatrix& parent_matrix );
  void build_configuration();
  void config_merge( Configuration& config );

  bool has_matrix(){
    return has_matrix_var;
  }

  int count_runnable_phases();

private:
  void parse_artifacts(std::vector<std::string>& paths, std::string folder);
  void build_source_file_from_path(std::vector<std::string>& paths);

  void run_in_pipe( std::string command, std::ostream& to ) ;

  void run_command( std::vector<std::string>& paths, std::string command ) ;
  void execute_recursive (std::vector<std::string>& paths);

  bool has_matrix_var = false;
  std::string name;
  ConfigurationMatrix matrix;
  Configuration config;
  std::chrono::microseconds duration;
};


inline void parse_folder( std::vector<Phase>& phases, std::string folder ) {
  using namespace std::experimental::filesystem;
  using namespace std;
  std::cout << "folder " << folder << std::endl;

  std::vector<std::string> paths;

  for( auto& p : directory_iterator(folder) ){
    paths.push_back( p.path() );
  }
 
  std::sort( paths.begin(), paths.end() ); 

  for(auto& p : paths) {
    std::cout << "p " << p << std::endl;
    if ( is_regular_file( p ) ) {
      Phase phase( p );
      phases.push_back( phase );
    }
    if ( is_directory( p ) ) {
      Phase phase( p );
      parse_folder( phase.sub_phases, p );
      phases.push_back( phase );
    }
  }
}



