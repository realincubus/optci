#!/bin/bash

. ../source_optci

OPTCI_RUN sleep 1

export SOCKETS=2
export CORES_PER_SOCKET=2
export THREADS_PER_CORE=2

OPTCI_PIN_RUN sleep 1 
