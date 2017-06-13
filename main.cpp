

#include <experimental/filesystem>
#include <iostream>
#include <algorithm>
#include <fstream>
#include <unistd.h>

#include "ConfigurationMatrix.hpp"
#include "Phase.hpp"
#include "ConfigurationParser.hpp"

using namespace std::experimental::filesystem;
using namespace std;

std::experimental::filesystem::path g_base_dir;
std::string g_files_folder;
std::string g_config_file;


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
  g_config_file = canonical(hook_folder) / "config.yaml" ;

  Phase root( phases_folder );

  parse_folder( root.sub_phases, phases_folder );

  add_config_information( root );

  //root.print();

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
