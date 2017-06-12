
#include <experimental/filesystem>
#include <fstream>
#include <regex>
#include <iostream>
#include <sstream>
#include <vector>

namespace fs = std::experimental::filesystem;
using namespace std;



int main(int argc, char** argv){
  
  fs::path p =  argv[1];

  ifstream in(p);
  std::string line;
  regex begin_match("^Sorted summary for file.*", std::regex::grep );
  regex other_match("^[[:space:]]*Percent.*", std::regex::grep );
  regex empty_match("^[[:space:]]*$", std::regex::egrep );
  regex dash_match("^-+", std::regex::egrep );

  auto get_perf_data_from_line = [](auto line){ 
    stringstream sstr(line);
    double percent;
    std::string path_and_line;
    sstr >> percent >> path_and_line;

    stringstream palstr(path_and_line);
    std::string file;
    int line_number;
    if ( getline(palstr, file, ':') ) {
      palstr >> line_number;
    }else{
      // TODO error handling
    }

    return make_tuple(percent, file, line_number);
  };

  vector<decltype(get_perf_data_from_line(""))> results;

  bool perf_data_read = false;
  while( getline( in, line ) ) {
    
    // search for the begin
    if ( regex_match( line, begin_match ) ) { 
      perf_data_read = true;
      continue;
    }

    // search for the end
    if ( regex_match( line, other_match ) ) { 
      perf_data_read = false;
    }

    if ( regex_match( line, empty_match ) ) {
      continue;
    } 
    if ( regex_match( line, dash_match ) ) {
      continue;
    } 


    if ( perf_data_read ) {
      std::cout << "line: " << line << std::endl;
      auto ret = get_perf_data_from_line( line );
      results.push_back(ret);
    }

  }

  std::sort( begin(results), end(results), [](auto a, auto b){ return std::get<0>(a) > std::get<0>(b); } );

  for( auto& result : results ){
    std::cout << "percent " << std::get<0>(result) << " file " << std::get<1>(result) << " line_number " << std::get<2>(result) << std::endl;
  }


  // write in a format that the optimizer can read
  ofstream out("hotspots.out");
  for( auto& result : results ){
    out << std::get<0>(result) << " " << std::get<1>(result) << " " << std::get<2>(result) << std::endl;
  }
     

  return 0;
}








