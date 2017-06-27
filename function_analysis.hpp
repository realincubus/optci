
#pragma once

#include <vector>
#include <functional>

namespace one_d_function_analysis{

  template < typename U, typename P >
  inline bool adjacent_compare( std::vector<U>& v, P p ){

    U last;

    bool first = true;
    for( auto& y : v ){
      if (first){
        last = y;
        first = false;
      }else{
        if ( p( y, last ) ) {
          return false;
        } 
      }
    }
    return true;
  }

  // very strict comparisons 
  template < typename U >
  inline bool is_monotonically_decreasing( std::vector<U>& v ){
    return adjacent_compare( v, std::greater_equal<U>() );
  }

  template < typename U >
  inline bool is_monotonically_increasing( std::vector<U>& v ){
    return adjacent_compare( v, std::less_equal<U>() );
  }

  template < typename U, typename V >
  inline auto maximum_combination( std::vector<std::pair<U,V>>& v ){
    std::pair<U,V>* ret = nullptr;
    V max = std::numeric_limits<V>::lowest();
    for( auto& pair : v ){
      auto [x,y] = pair;
      if ( y > max ) {
        ret = &pair;
        max = y;
      }
    }
    return ret;
  }

  template < typename U, typename V >
  inline auto minimum_combination( std::vector<std::pair<U,V>>& v ){
    std::pair<U,V>* ret = nullptr;
    V min = std::numeric_limits<V>::max();
    for( auto& pair : v ){
      auto [x,y] = pair;
      if ( y < min ) {
        ret = &pair;
        min = y;
      }
    }
    return ret;
  }

  template < typename U, typename V >
  inline auto no_change( std::vector<std::pair<U,V>>& v, double relative_tolerance ){
    auto min = minimum_combination( v );
    auto max = maximum_combination( v );
    
    if ( min && max ) {
      auto minv = min->second;
      auto maxv = max->second;
      auto distance = maxv-minv;
      //std::cout << "distance " << distance << " absolute tolerance " << relative_tolerance * maxv << std::endl;
      if ( distance < (relative_tolerance * maxv) ) {
        return true;
      }
    }

    return false;
  }


}
