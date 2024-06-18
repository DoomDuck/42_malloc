#!/bin/sh

# Error checks
set -o errexit
set -o nounset
set -o pipefail

# Redirect output to stderr.
exec 1>&2

has_invalid_header=no

for file in $(find -name '*.h'); do
	if [[ $(head -n 1 "$file") != '#pragma once' ]]; then
		echo "Missing '#pragma once' in $file"
		has_invalid_header=yes
	fi
done

# Error out on missing #pragma once
test "$has_invalid_header" == 'no'
