

#include <experimental/filesystem>
#include <iostream>
#include <algorithm>
#include <fstream>
#include <unistd.h>

#include "ConfigurationMatrix.hpp"

using namespace std::experimental::filesystem;
using namespace std;

// TODO remove global var
path g_base_dir;
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
    ofstream out( "source.this" );

    // build source file
    for( auto& path : paths ){
      ifstream in( path );
      std::string line;
      while( getline( in, line ) )  {
	out << line << endl;
      }
    }
    
    system( (g_base_dir.string() + "run_wrapper.sh "s + "source.this"s + " "s + command + " "s + g_files_folder).c_str() ) ;
  }


  std::string name;
  std::vector<std::string> artifacts;
  ConfigurationMatrix* matrix = nullptr;
};


void parse_folder( std::vector<Phase>& phases, std::string folder ) {
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




void add_config_information( Phase& phase ) {

  if ( phase.get_name() == "004_base/001_run" ) {
    // get this information from hwloc
    ConfigurationMatrix m;
    Axis cores("CORES");
    cores.add_value("1");
    cores.add_value("2");
    cores.add_value("4");
    cores.add_value("8");

    Axis sockets("SOCKETS");
    sockets.add_value("0");
    sockets.add_value("0-1");

    m.add_axis(sockets);
    m.add_axis(cores);

    phase.set_run_configuration(m); 
  }

  for( auto&& element : phase.sub_phases ){
      add_config_information( element );
  }
  
}

int main(int argc, char** argv){

   path p = argv[0];

   auto install_dir = canonical(p).remove_filename();
   std::cout << "install_dir " << install_dir << std::endl;
   g_base_dir = install_dir;

  // TODO add sanity check
  path hook_folder = argv[1];
  path phases_folder = hook_folder / "phases";
  path files_folder = hook_folder / "files";

  // TODO remove 
  g_files_folder = canonical(files_folder);

  Phase root( phases_folder );

  parse_folder( root.sub_phases, phases_folder );

  add_config_information( root );

  root.print();

  try {
    remove_all( "artifacts" );
  }catch (...){
    std::cout << "nothing to remove" << std::endl;
  }

  try {
    remove( "source.this" );
  }catch (...){
    std::cout << "nothing to remove" << std::endl;
  }

  std::vector<std::string> paths;
  paths.push_back(g_base_dir.string() + "source_optci");
  root.execute(paths);


  return 0;
}
