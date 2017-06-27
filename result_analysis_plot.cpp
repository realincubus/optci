

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


template <typename T>
void make_gnuplot_multiplot( MultiDArray<T>& mda ){

  std::ofstream out("outfile.plt");

  auto max = mda.max();

  out << "set terminal png" << std::endl;
  out << "set palette defined ( 0 \"red\", 1 \"white\", " << max << " \"blue\" )" << std::endl;
  out << "set multiplot layout " << mda.size() << ",1 margins 0.1,0.98,0.1,0.98 spacing 0.08,0.08" << std::endl;

  for( auto& plot_data2d : mda ){
    // TODO insert the multiplot origin and size calculation here !! not needed right now
    make_2d_matrix_plot( out, plot_data2d );
  }
  out << "unset multiplot" << std::endl;
}

void generate_gnuplot_matrix_plot( MultiDArray<double>& mda ){
  make_gnuplot_multiplot(mda);
}
