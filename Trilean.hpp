#pragma once

struct Trilean {

  enum State{
    False,
    True,
    Undef
  };

  Trilean() :
    s(Undef)
  {

  }

  explicit Trilean ( bool _state ) {
    if ( _state ) {
      s = True;
    }else{
      s = False;
    }
  }

  operator bool() {
    if ( s == True ) {
      return true;
    }else{
      return false;
    }
  }

  auto& operator=( bool _state ){
    if ( _state ) {
      s = True;
    }else{
      s = False;
    }
    return *this;
  }

  const bool operator==( const bool _state ) const{
    if ( this->s == True && _state == true ) {
      return true;
    }else{
      return false;
    }
  }

  State s;
};
