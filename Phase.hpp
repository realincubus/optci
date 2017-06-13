#pragma once

#include <string>
#include <fstream>
#include <experimental/filesystem>

#include "ConfigurationMatrix.hpp"

// TODO remove global var
std::experimental::filesystem::path g_base_dir;
std::string g_files_folder;

struct Phase {
  Phase( std::string phase_name ) :
    name(phase_name ) 
  {
    
  }

  void print(int indent = 0) {
    
    for (int i = 0; i < indent; ++i){
      std::cout << " ";
    }
    std::cout << "phase " << name  << std::endl;
    for( auto& sub_phase : sub_phases ){
      sub_phase.print(indent+2); 
    }
    
  }

  void execute (std::vector<std::string>& paths) {
    using namespace std;
    if ( sub_phases.empty() ) {
      if ( matrix ) {
        matrix->for_each_configuration( [&](auto a){ 
            run_command( paths, name );
          }
        );
      }else{
        run_command( paths, name );
      }
    }
  
    parse_artifacts( paths, "artifacts/"s+ name );

    for( auto& sub_phase : sub_phases ){
      sub_phase.execute(paths);
    }
  }

  std::vector<Phase> sub_phases;

  auto get_name() {
      return name;
  }

  auto set_run_configuration( ConfigurationMatrix& m) {
    matrix = new ConfigurationMatrix(m);
  }

private:
  void parse_artifacts(std::vector<std::string>& paths, std::string folder){
    using namespace std::experimental::filesystem;
    if ( !exists( folder ) ) return;
    for( auto& p : directory_iterator(folder) ){
      if ( is_directory( p ) ) {
	parse_artifacts( paths, p.path() );
      }
      if ( is_regular_file( p ) ) {
	paths.push_back( p.path() );
	std::cout << "added " << p << " to the list of files to source" << std::endl;
      }
    }
  }

  void run_command( std::vector<std::string>& paths, std::string command ) {
    using namespace std;
    std::ofstream out( "source.this" );

    // build source file
    for( auto& path : paths ){
      std::ifstream in( path );
      std::string line;
      while( getline( in, line ) )  {
	out << line << std::endl;
      }
    }
    
    system( (g_base_dir.string() + "run_wrapper.sh "s + "source.this"s + " "s + command + " "s + g_files_folder).c_str() ) ;
  }


  std::string name;
  std::vector<std::string> artifacts;
  ConfigurationMatrix* matrix = nullptr;
};


