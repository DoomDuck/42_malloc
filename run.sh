#!/bin/sh

export LD_PRELOAD=libmalloc.so
export LD_LIBRARY_PATH=$(pwd)

exec "$@"
