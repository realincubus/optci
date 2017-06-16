

#include "Phase.hpp"


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
Phase::execute (std::vector<std::string>& paths) {
  std::cout << this->get_name() << " running" << std::endl;
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

std::string Phase::get_name() {
    return name;
}

void Phase::set_run_configuration( ConfigurationMatrix& m) {
  matrix = new ConfigurationMatrix(m);
}

void 
Phase::for_each_phase( std::string p_name, std::function<void(Phase&)> f)
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


void 
Phase::parse_artifacts(std::vector<std::string>& paths, std::string folder){
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

  if ( matrix ) {
    matrix->for_each_configuration( [&](auto configuration){ 
      build_source_file_from_path(paths);

      // append more stuff for likwid or numactl
      std::ofstream out( "source.this", ios::app );
      std::string config_tuple;
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
      out << "export OPTCI_CONFIG_TUPLE=" << config_tuple << endl;
      out.close();
      std::string full_command = g_base_dir.string() + "run_wrapper.sh "s + "source.this"s + " \""s + command + "\" "s + g_files_folder;
      run_in_pipe( full_command.c_str(), cout );
    });
  }else{
    // TODO remove doublication
    build_source_file_from_path(paths);
    std::string full_command = g_base_dir.string() + "run_wrapper.sh "s + "source.this"s + " \""s + command + "\" "s + g_files_folder;
    run_in_pipe( full_command.c_str(), cout );
  }
}


void Phase::pass_down_matrix(){
  std::cout << "pass_down_matrix" << std::endl;

  if ( matrix ) {
    for( auto& sub_phase : sub_phases ){
      std::cout << this->get_name() << " have matrix passing down to " << sub_phase.get_name() << std::endl;
      sub_phase.merge_matrix( *matrix );
    }
  }
  for( auto&& sub_phase : sub_phases ){
    sub_phase.pass_down_matrix();
  }
  
}

void Phase::merge_matrix(ConfigurationMatrix& parent_matrix ) {
  if ( matrix ) {
    for( auto& axis : parent_matrix ){
      std::cout << "merging axis" << std::endl;
      matrix->add_axis(axis);
    }
  }else{
    std::cout << "adding copy of parent matrix" << std::endl;
    this->matrix = new ConfigurationMatrix(parent_matrix); 
  }
}





