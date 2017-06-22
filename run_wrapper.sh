#!/bin/bash 

. $1

BASE=$PWD

if [ -z ${CHECKOUT_FOLDER+x} ]; then 
  echo "" 
else 
  cd $CHECKOUT_FOLDER
fi

export ARTIFACT=$BASE/artifacts/$2
export FILES=$3

mkdir -p $ARTIFACT

export PATH=$PATH:$PWD  

$BASE/$2
