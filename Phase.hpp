#pragma once

#include <string>
#include <fstream>
#include <experimental/filesystem>
#include <functional>

#include "ConfigurationMatrix.hpp"


// TODO remove global var
extern std::experimental::filesystem::path g_base_dir;
extern std::string g_files_folder;
extern std::string g_config_file;

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
      run_command( paths, name );
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

  void for_each_phase( std::string p_name, std::function<void(Phase&)> f)
  {
    std::cout << "p_name " << p_name << std::endl;
    std::cout << "name " << name << std::endl;
    if ( p_name == name ){
      f( *this );
    }
    for( auto&& sub_phase : sub_phases ){
      sub_phase.for_each_phase( p_name, f );
    }
  }

  // this moves in a intermediate phase that acts as a matrix
  void apply_matrix(){
    int phase_number = 0;

    // make a copy of this phase
    auto old_phase = *this;

    old_phase->matrix = nullptr;

    //if ( this->sub_phases.empty() ) {
      Phase matrix_phase("matrix_phase");
      matrix->for_each_configuration( [&](auto configuration ) {
        matrix_phase.sub_phases.emplace_back( std::to_string(phase_number++) );
        auto& last = matrix_phase.sub_phases.back();
        last.sub_phases.push_back( old_phase );
      });

#if 0
    }else{
      Phase matrix_phase("matrix_phase");
      auto sub_phases = this->sub_phases;
      this->sub_phases.clear();
      matrix->for_each_configuration( [&](auto configuration ) {
        matrix_phase.sub_phases.emplace_back( std::to_string(phase_number++) );
        auto& last = matrix_phase.sub_phases.back();
        last.sub_phases = sub_phases;
      });
      this->sub_phases.push_back( matrix_phase );
    }
#endif
    this->sub_phases.push_back( matrix_phase );

    delete this->matrix;
    this->matrix = nullptr;
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

  void build_source_file_from_path(std::vector<std::string>& paths){
    std::ofstream out( "source.this" );

    // build source file
    for( auto& path : paths ){
      std::ifstream in( path );
      std::string line;
      while( getline( in, line ) )  {
	out << line << std::endl;
      }
    }
  }

  void run_command( std::vector<std::string>& paths, std::string command ) {
    using namespace std;

    if ( matrix ) {
      matrix->for_each_configuration( [&](auto configuration){ 
        build_source_file_from_path(paths);

        // append more stuff for likwid or numactl
        std::ofstream out( "source.this", ios::app );
        for( auto& pair : configuration ){
          out << "export " << pair.first << "=" << pair.second << std::endl;
        }
        out.close();
        std::string full_command = g_base_dir.string() + "run_wrapper.sh "s + "source.this"s + " \""s + command + "\" "s + g_files_folder;
        //std::cout << "full_command: " << full_command << std::endl;
        system( full_command.c_str() ) ;
      });
    }else{
      // TODO remove doublication
      build_source_file_from_path(paths);
      std::string full_command = g_base_dir.string() + "run_wrapper.sh "s + "source.this"s + " \""s + command + "\" "s + g_files_folder;
      //std::cout << "full_command: " << full_command << std::endl;
      system( full_command.c_str() ) ;
    }
  }



  std::string name;
  std::vector<std::string> artifacts;
  ConfigurationMatrix* matrix = nullptr;
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



