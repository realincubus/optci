#!/bin/bash 

. $1

BASE=$PWD

if [ -z ${CHECKOUT_FOLDER+x} ]; then 
  echo "var is unset"; 
else 
  cd $CHECKOUT_FOLDER
fi

export ARTIFACT=$BASE/artifacts/$2
mkdir -p $ARTIFACT

$BASE/$2
