

#include "ConfigurationParser.hpp"
#include <gtest/gtest.h>
#include <experimental/filesystem>
#include <yaml-cpp/yaml.h>

namespace fs = std::experimental::filesystem;



TEST( ADD_CONFIG_TO_PHASES, Positive ) {

  // TODO remove this intialization stuff ;
  fs::path p = "../../bin_optci";

  auto install_dir = canonical(p).remove_filename();
  std::cout << "install_dir " << install_dir << std::endl;
  g_base_dir = install_dir;

  fs::path hook_folder = "config";
  fs::path phases_folder = hook_folder / "phases";

  g_files_folder = canonical(hook_folder) / "files";
  g_config_file = canonical(hook_folder / "config.yaml" );

  Phase root("config/phases");

  parse_folder( root.sub_phases, phases_folder );

  add_config_information( root );

  std::vector<std::string> paths;
  root.execute(paths);

  EXPECT_TRUE( fs::exists("artifacts/config/phases/001_example_phase/r_0_1" ) );
  EXPECT_TRUE( fs::exists("artifacts/config/phases/001_example_phase/r_0_2" ) );
  EXPECT_TRUE( fs::exists("artifacts/config/phases/001_example_phase/r_0_4" ) );
  EXPECT_TRUE( fs::exists("artifacts/config/phases/001_example_phase/r_0_8" ) );
  EXPECT_TRUE( fs::exists("artifacts/config/phases/001_example_phase/r_0_16" ) );

  EXPECT_TRUE( fs::exists("artifacts/config/phases/001_example_phase/r_1_1" ) );
  EXPECT_TRUE( fs::exists("artifacts/config/phases/001_example_phase/r_1_2" ) );
  EXPECT_TRUE( fs::exists("artifacts/config/phases/001_example_phase/r_1_4" ) );
  EXPECT_TRUE( fs::exists("artifacts/config/phases/001_example_phase/r_1_8" ) );
  EXPECT_TRUE( fs::exists("artifacts/config/phases/001_example_phase/r_1_16" ) );

  EXPECT_TRUE( fs::exists("artifacts/config/phases/002_example_phase/r_0_1" ) );
  EXPECT_TRUE( fs::exists("artifacts/config/phases/002_example_phase/r_0_2" ) );
  EXPECT_TRUE( fs::exists("artifacts/config/phases/002_example_phase/r_0_4" ) );
  EXPECT_TRUE( fs::exists("artifacts/config/phases/002_example_phase/r_0_8" ) );
  EXPECT_TRUE( fs::exists("artifacts/config/phases/002_example_phase/r_0_16" ) );

  EXPECT_TRUE( fs::exists("artifacts/config/phases/002_example_phase/r_1_1" ) );
  EXPECT_TRUE( fs::exists("artifacts/config/phases/002_example_phase/r_1_2" ) );
  EXPECT_TRUE( fs::exists("artifacts/config/phases/002_example_phase/r_1_4" ) );
  EXPECT_TRUE( fs::exists("artifacts/config/phases/002_example_phase/r_1_8" ) );
  EXPECT_TRUE( fs::exists("artifacts/config/phases/002_example_phase/r_1_16" ) );



}

TEST( GET_TOPOLOGY, Positive ) {
  auto [ a,b,c] = get_topology();  
  std::cout << "s " << a << " c " << b << " t " << c << std::endl;
}

TEST( ADD_COFIG_AUTO, Positive ) {

  // TODO remove this intialization stuff ;
  fs::path p = "../../bin_optci";

  auto install_dir = canonical(p).remove_filename();
  std::cout << "install_dir " << install_dir << std::endl;
  g_base_dir = install_dir;

  fs::path hook_folder = "config";
  fs::path phases_folder = hook_folder / "phases";

  g_files_folder = canonical(hook_folder) / "files";
  g_config_file = canonical(hook_folder / "config_auto.yaml" );

  Phase root("config/phases");

  parse_folder( root.sub_phases, phases_folder );

  add_config_information( root );

  std::vector<std::string> paths;
  root.execute(paths);

}

TEST( NESTED_PHASES, Positive ) {

  // TODO remove this intialization stuff ;
  fs::path p = "../../bin_optci";

  auto install_dir = canonical(p).remove_filename();
  std::cout << "install_dir " << install_dir << std::endl;
  g_base_dir = install_dir;

  fs::path hook_folder = "config_nested";
  fs::path phases_folder = hook_folder / "phases";

  g_files_folder = canonical(hook_folder) / "files";
  g_config_file = canonical(hook_folder / "config.yaml" );

  Phase root("config_nested/phases");

  parse_folder( root.sub_phases, phases_folder );

  add_config_information( root );

  std::vector<std::string> paths;
  root.execute(paths);

}





