

#include "ConfigurationParser.hpp"
#include <yaml-cpp/yaml.h>
#include <hwloc.h>

namespace fs = std::experimental::filesystem;
using namespace std;

std::tuple<int,int,int> get_topology() {
  hwloc_topology_t topology;
  hwloc_cpuset_t cpuset;
  hwloc_obj_t obj;
  /* Allocate and initialize topology object. */
  hwloc_topology_init(&topology);
  /* ... Optionally, put detection configuration here to ignore
     some objects types, define a synthetic topology, etc....  
     The default is to detect all the objects of the machine that
     the caller is allowed to access.  See Configure Topology
     Detection. */
  /* Perform the topology detection. */
  hwloc_topology_load(topology);
  /* Optionally, get some additional topology information
     in case we need the topology depth later. */
  /*****************************************************************
   * First example:
   * Walk the topology with an array style, from level 0 (always
   * the system level) to the lowest level (always the proc level).
   *****************************************************************/
#if 0
  int topodepth = hwloc_topology_get_depth(topology);
  for (int depth = 0; depth < topodepth; depth++) {
    printf("*** Objects at level %d\n", depth);
    char string[128];
    for (int i = 0; i < hwloc_get_nbobjs_by_depth(topology, depth); i++) {
        hwloc_obj_snprintf(string, sizeof(string), topology,
                   hwloc_get_obj_by_depth(topology, depth, i),
                   "#", 0);
        printf("Index %u: %s\n", i, string);
    }
  }
#endif

  auto get_number_of = [&](auto t, auto& out){ 
    auto depth = hwloc_get_type_depth(topology, t);
    if (depth == HWLOC_TYPE_DEPTH_UNKNOWN) {
      std::cout << "could not get the number of sockets" << std::endl;
    } else {
      out = hwloc_get_nbobjs_by_depth(topology, depth);
    }
  };

  // TODO calculations apply for totaly symetric systems 
  //      change if needed
  int number_of_sockets=-1;
  get_number_of( HWLOC_OBJ_SOCKET, number_of_sockets );
  
  int number_of_cores=-1;
  get_number_of( HWLOC_OBJ_CORE, number_of_cores );
  int cores_per_socket = number_of_cores / number_of_sockets;

  int number_of_processing_units=-1;
  get_number_of( HWLOC_OBJ_PU, number_of_processing_units );
  int threads_per_core = number_of_processing_units / number_of_cores;
  
  hwloc_topology_destroy(topology);

  return make_tuple( number_of_sockets, cores_per_socket, threads_per_core );
}

void handle_once( const YAML::Node& node, Phase& root )
{
  std::vector<std::string> phases;
  if (node["scaling_phases"]) {
    const YAML::Node& sp = node["scaling_phases"];
    phases = sp.as<std::vector<std::string>>();
  }


  ConfigurationMatrix matrix;
  if (node["matrix"]){
    const YAML::Node& m = node["matrix"];

    if ( m.size() == 0 ) {
      auto val = m.as<std::string>();

      std::array<std::string,4> auto_types = {
        "auto",
        "auto_even",
        "auto_odd",
        "auto_pow2"
      };

      if ( std::find( begin(auto_types), end(auto_types), val ) != auto_types.end() ) {
        auto [s, c, t] = get_topology();
        // stupid reassign due to limitiation of structured bindings
        auto sockets = s;
        auto cores_per_socket = c;
        auto threads_per_core = t;

        Axis socket_axis("SOCKETS");
        Axis cores_per_socket_axis("CORES_PER_SOCKET");
        Axis threads_per_core_axis("THREADS_PER_CORE");

        auto add_values = [&]( auto p ){ 
          for (int s = 0; s < sockets; s++){
            if ( !p(s+1) ) continue;
            socket_axis.add_value( to_string(s+1) );
          }

          for (int c = 0; c < cores_per_socket; c++){
            if ( !p(c+1) ) continue;
            cores_per_socket_axis.add_value( to_string(c+1) );
          }

          for (int t = 0; t < threads_per_core; t++){
            if ( !p(t+1) ) continue;
            threads_per_core_axis.add_value( to_string(t+1) );
          }
        };


        auto is_dont_care = [](auto a){ return true; };
        auto is_even = [](auto a){ return a % 2 == 0; };
        auto is_odd = [](auto a){ return a % 2 == 1; };
        auto is_pow2 = [](auto a){ return (a & (a - 1)) == 0; };

        if ( val == "auto" ) {
          add_values( is_dont_care );
        }

        if ( val == "auto_even" ) {
          add_values( is_even );
        }

        if ( val == "auto_odd" ) {
          add_values( is_odd );
        }

        if ( val == "auto_pow2" ) {
          add_values( is_pow2 );
        }

        matrix.add_axis( socket_axis );
        matrix.add_axis( cores_per_socket_axis );
        matrix.add_axis( threads_per_core_axis );
      }
    }else{
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
  }

 
  for( auto& phase : phases ){
    bool found = false;
    root.for_each_phase( phase, [&](Phase& p){ 
      std::cout << "setting run configuration for phase " << p.get_name()  << std::endl;
      p.set_run_configuration(matrix);
      found = true;
    });

    if ( !found ) {
      std::cout << "could not find " << phase << std::endl;
    }
  }


}

void add_config_information( Phase& root ) {

  std::cout << "addibg config information to phases" << std::endl;

  // check wether a config.yaml file exists
  if (!fs::exists(g_config_file) )  {
    std::cout << "no config file provided not adding extra info to phases" << std::endl;
    return;
  } 

  YAML::Node config = YAML::LoadFile(g_config_file);

  std::cout << "config.size " << config.size() << std::endl;

  bool multi = false;
  for (int i = 0 ; i < config.size() ; i++){
    const YAML::Node& node = config[i];
    if ( node["scaling_phases"]) {
      std::cout << "got phase" << std::endl;
      handle_once( node, root );
      multi = true;
    }
  }

  if ( !multi ) {
    handle_once( config, root );
  }

  //root.pass_down_matrix();



}












