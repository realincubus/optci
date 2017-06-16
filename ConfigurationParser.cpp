

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

void add_config_information( Phase& root ) {

  // TODO check wether a config.yaml file exists
  if (!fs::exists(g_config_file) )  {
    std::cout << "no config file provided not adding extra info to phases" << std::endl;
    return;
  } 

  YAML::Node config = YAML::LoadFile(g_config_file);

  std::vector<std::string> phases;
  if (config["scaling_phases"]) {
    const YAML::Node& sp = config["scaling_phases"];
    phases = sp.as<std::vector<std::string>>();
  }


  ConfigurationMatrix matrix;
  if (config["matrix"]){
    const YAML::Node& m = config["matrix"];

    if ( m.size() == 0 ) {
      auto val = m.as<std::string>();
      if ( val == "auto" ) {
        auto [sockets, cores_per_socket, threads_per_core] = get_topology();
        Axis socket_axis("SOCKETS");
        for (int s = 0; s < sockets; s++){
          socket_axis.add_value( to_string(s) );
        }
        matrix.add_axis( socket_axis );

        Axis cores_per_socket_axis("CORES_PER_SOCKET");
        for (int c = 0; c < cores_per_socket; c++){
          cores_per_socket_axis.add_value( to_string(c) );
        }

        matrix.add_axis( cores_per_socket_axis );

        Axis threads_per_core_axis("THREADS_PER_CORE");
        for (int t = 0; t < threads_per_core; t++){
          threads_per_core_axis.add_value( to_string(t) );
        }

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
    root.for_each_phase( phase, [&](Phase& p){ 
        std::cout << "setting run configuration" << std::endl;
      p.set_run_configuration(matrix);
    });
  }

}
