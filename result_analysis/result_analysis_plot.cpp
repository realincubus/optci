

#include "result_analysis_plot.hpp"

#include <fstream>
#include <regex>
#include <iostream>

template <typename T>
void make_2d_matrix_plot( std::ostream& out, MultiDArray<T>& mda ){
  using namespace std;
  static int map_ctr = 0;
  std::string map_name = "$map_"s + to_string(map_ctr++);
  out << map_name << "<< EOD" << std::endl;
  for( auto& plot_data : mda ){
    for( int i = 0; i < plot_data.size(); i++ ){
      out << plot_data.get_value( { i } ) << " ";
    }
    out << std::endl;
  }
  out << "EOD" << std::endl;
  out << "set view map" << endl;
  out << "splot '" << map_name << "' matrix with image" << std::endl;
}


void make_gnuplot_multiplot( MultiDArray<double>& mda ){
  using namespace std;

  //std::ofstream out("outfile.plt2");

  auto max = mda.max();

  cout << "set terminal png" << std::endl;
  cout << "set palette defined ( 0 \"red\", 1 \"white\", " << max << " \"blue\" )" << std::endl;
  cout << "set multiplot layout " << mda.size() << ",1 margins 0.1,0.98,0.1,0.98 spacing 0.08,0.08" << std::endl;

  for( auto& plot_data2d : mda ){
    // TODO insert the multiplot origin and size calculation here !! not needed right now
    make_2d_matrix_plot( cout, plot_data2d );
  }
  cout << "unset multiplot" << std::endl;
}

void generate_gnuplot_matrix_plot( MultiDArray<double>& mda ){
  make_gnuplot_multiplot(mda);
}
