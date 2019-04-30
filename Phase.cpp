

#include "Phase.hpp"
#include <string>

using namespace std;

int g_phase_id = 1;
int g_max_phases;

Phase::Phase( std::string phase_name ) :
    name(phase_name ) 
{
  
}

void Phase::print(int indent) {
  
  for (int i = 0; i < indent; ++i){
    std::cout << " ";
  }
  std::cout << "phase " << name  << std::endl;
  for( auto& sub_phase : sub_phases ){
    sub_phase.print(indent+2); 
  }
  
}

void 
Phase::execute(std::vector<std::string>& paths ){
  std::cout << "Step " << g_phase_id++ << " " << this->get_name() << " running" << std::endl;
  build_configuration();
  print();

  execute_recursive( paths );
}

void 
Phase::execute_recursive (std::vector<std::string>& paths) {

  using namespace std;
  if ( sub_phases.empty() ) {
    run_command( paths, name );
  }

  parse_artifacts( paths, "artifacts/"s+ name );

  for( auto& sub_phase : sub_phases ){
    sub_phase.execute_recursive(paths);
  }
}

std::vector<Phase> sub_phases;

std::string Phase::get_name() {
    return name;
}

void Phase::set_run_configuration( ConfigurationMatrix& m) {
  matrix = m;
  has_matrix_var = true;
}

void 
Phase::for_each_phase( std::string p_name, std::function<void(Phase&)> f)
{
  if ( p_name == name ){
    f( *this );
  }
  for( auto&& sub_phase : sub_phases ){
    sub_phase.for_each_phase( p_name, f );
  }
}


void 
Phase::parse_artifacts(std::vector<std::string>& paths, std::string folder){
  using namespace std::experimental::filesystem;
  if ( !exists( folder ) ) return;
  for( auto& p : directory_iterator(folder) ){
    if ( is_directory( p ) ) {
      parse_artifacts( paths, p.path() );
    }
    if ( is_regular_file( p ) ) {
      auto f = std::find_if( begin(paths), end(paths), 
          [&](std::string& v){ 
            return v == p.path().string(); 
          } 
      );

      if ( f == paths.end() )  {
        paths.push_back( p.path() );
        std::cout << "added " << p << " to the list of files to source" << std::endl;
      }else{
        //std::cout << "already in list" << std::endl;
      }
    }
  }
}

void Phase::build_source_file_from_path(std::vector<std::string>& paths){
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

void Phase::run_in_pipe( std::string command, std::ostream& to ) {
  FILE* in = nullptr;
  if ( !(in = popen(command.c_str(), "r") ) ){
  }else{
    constexpr int size = 4096;
    char buff[size];
    while (fgets(buff, sizeof(buff), in) != NULL){
      std::string line(&buff[0]);
      to << line;
    } 
  }
  pclose(in);
}

void Phase::run_command( std::vector<std::string>& paths, std::string command ) {
  using namespace std;

  std::cout << std::endl;
  std::cout << "Step " << g_phase_id++ << "/" << g_max_phases  << " : " << this->get_name()  << std::endl;
  std::cout << std::endl;
#if 0
  if ( matrix ) {
    matrix->for_each_configuration( [&](auto configuration){ 
      build_source_file_from_path(paths);

      // append more stuff for likwid or numactl
      std::ofstream out( "source.this", ios::app );
      std::string config_tuple;
      
      auto conf = config;
      for( auto& _config : configuration ){
        conf.add( _config.first, _config.second );
      }

#if 0
      bool first = true;
      for( auto& pair : configuration ){
        out << "export " << pair.first << "=" << pair.second << std::endl;
        if ( !first ) {
          config_tuple += "_";
        }else{
          first=false;
        }
        config_tuple += pair.second;
      }
#endif
      config_tuple = conf.to_string();
      out << "export OPTCI_CONFIG_TUPLE=" << config_tuple << endl;
      out.close();
      std::string full_command = g_base_dir.string() + "run_wrapper.sh "s + "source.this"s + " \""s + command + "\" "s + g_files_folder;
      run_in_pipe( full_command.c_str(), cout );
    });
  }else{
#endif


    build_source_file_from_path(paths);
    // TODO remove doublication
    std::ofstream out( "source.this", ios::app );
    for( auto& pair : config ){
      out << "export " << pair.first << "=" << pair.second << std::endl;
    }

    auto config_tuple = config.to_string();
    out << "export OPTCI_CONFIG_TUPLE=" << config_tuple << endl;
    out.close();
    //std::cout << "running " << this->get_name() << " with config" << std::endl;

    std::string full_command = g_base_dir.string() + "run_wrapper.sh "s + "source.this"s + " \""s + command + "\" "s + g_files_folder;
    run_in_pipe( full_command.c_str(), cout );
  //}
}

// use the matrix to build a config and pass this down to all sub_phases
void Phase::build_configuration(){

  //std::cout << "running " << __PRETTY_FUNCTION__  << " for " << this->get_name()<< std::endl;

  if ( has_matrix() && !this->sub_phases.empty() ) {
    auto phases_base_copy = this->sub_phases;
    this->sub_phases.clear();

    matrix.for_each_configuration( [&](auto conf){ 
      auto phases_copy = phases_base_copy;
      Phase c("config_manifestation_"s + conf.to_string() );
      for( auto& sub_phase : phases_copy ){
        sub_phase.config_merge( conf );
        c.sub_phases.push_back( sub_phase );
      }
      this->sub_phases.push_back( c );
    });
    has_matrix_var = false;
  }

  if ( has_matrix() && this->sub_phases.empty() ) {
    has_matrix_var = false;
    Phase this_copy = *this;
    matrix.for_each_configuration( [&](auto conf){ 
      Phase c("config_manifestation_"s + conf.to_string() );
      auto manifestation = this_copy;
      manifestation.config_merge( conf );
      c.sub_phases.push_back( manifestation );
      this->sub_phases.push_back(c);
    });
  }

  for( auto& sub_phase : sub_phases ){
    sub_phase.build_configuration(); 
  }
  

}


void Phase::config_merge( Configuration& other_config ){
  for( auto&& conf : other_config ){
    this->config.add( conf.first, conf.second );
  }
}



int 
Phase::count_runnable_phases(){
  if ( sub_phases.empty() ) {
    if ( has_matrix_var ){
      int configurations = 0;
      matrix.for_each_configuration( [&](auto c){ 
          configurations++;
        }
      );
      return configurations;
    }else{
      return 1;
    }
  }else{
    int sum = 0;
    for( auto&& phase : sub_phases ){
      sum += phase.count_runnable_phases();   
    }
    return sum;
  }
}

