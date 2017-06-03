

#include <experimental/filesystem>
#include <iostream>
#include <algorithm>
#include <unistd.h>

using namespace std::experimental::filesystem;
using namespace std;

void run_script ( std::string fname ) {
  system ( ("./run_wrapper.sh " + fname).c_str() );
}

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

  void execute ( ) {
    if ( sub_phases.empty() ) {
      run_script( name.c_str() );
    }
    for( auto& sub_phase : sub_phases ){
      sub_phase.execute();
    }
  }

  std::string name;
  std::vector<Phase> sub_phases;
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


int main(int argc, char** argv){

  // TODO add sanity check
  std::string hook_folder = argv[1];

  Phase root( hook_folder );

  parse_folder( root.sub_phases, hook_folder );

  root.print();

  root.execute();


  return 0;
}
