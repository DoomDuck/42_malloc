#!/bin/sh

# Error checks
set -o errexit
set -o nounset
set -o pipefail

export LD_PRELOAD=libft_malloc.so
export LD_LIBRARY_PATH=$(pwd)

exec "$@"
