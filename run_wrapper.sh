#!/bin/bash 

source $1

BASE=$PWD

if [ -z ${CHECKOUT_FOLDER+x} ]; then 
  echo "var is unset"; 
else 
  cd $CHECKOUT_FOLDER
fi

$BASE/$2
