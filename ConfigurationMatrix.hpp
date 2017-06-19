#pragma once


#include <string>
#include <vector>
#include <functional>
#include <iostream>
#include <utility>

struct Configuration {
  public:
    void add( std::string key, std::string value ){
      configuration.push_back( std::make_pair( key, value ) );
    }

    auto begin() {
        /* code */
        return configuration.begin();
    }
    auto end() {
        /* code */
        return configuration.end();
    }

    auto empty() {
      return configuration.empty();
    }

    auto& operator[](int index){
      return configuration[index];
    }

    auto to_string(){
      std::string conf_string;
      bool first = true;
      for( auto&& conf : configuration ){
        if ( first ) {
          first = false;
        }else{
          conf_string += "_";
        }
        conf_string += conf.second;
      }
      return conf_string;
    }

  private:
    std::vector<std::pair<std::string,std::string>> configuration;
};


class Axis {
public:
    Axis (std::string _name) :
      name(_name)
    {
    }
    virtual ~Axis () {}

    auto add_value(std::string v) {
      values.push_back(v);
    }

    std::string name;
    std::vector<std::string> values;
private:
};

class ConfigurationMatrix {
public:
    ConfigurationMatrix () 
    {
    }
    virtual ~ConfigurationMatrix () {}

    auto add_axis( Axis& a) {
      axis.push_back(a);
    }

    void for_each_configuration( std::function<void(Configuration)> f ){
      using namespace std;

      std::function<void(Configuration,int)> combine_axis = [&](Configuration config, int i) { 
        auto& a = axis[i];
        for( auto& element : a.values ){
          auto element_config = config;
          if ( i+1 < axis.size() ) {
            element_config.add( a.name, element);
            combine_axis( element_config, i + 1 );
          }else {
            for( auto& exp : element_config ){
              //std::cout << "export " << exp.first << "=" << exp.second << std::endl;
            }
            //cout << "export " << a.name << "=" << element << endl;
            //std::cout << std::endl;
            element_config.add( a.name, element );
            f ( element_config );
          }
        }
        
      };

      Configuration config;
      combine_axis( config, 0 );
    }

    auto begin() {
        /* code */
        return axis.begin();
    }
    auto end() {
        /* code */
        return axis.end();
    }

private:
    std::vector<Axis> axis;
};
