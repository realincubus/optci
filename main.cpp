

#include <experimental/filesystem>
#include <iostream>
#include <algorithm>
#include <fstream>
#include <unistd.h>

#include "ConfigurationMatrix.hpp"
#include "Phase.hpp"

using namespace std::experimental::filesystem;
using namespace std;


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

#if BUILDING_LIB
int optci_run( int argc, char** argv )
#else
int main(int argc, char** argv)
#endif
{
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
