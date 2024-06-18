#!/bin/sh

# Error checks
set -o nounset
set -o pipefail

# Redirect output to stderr.
exec 1>&2

a_check_didnt_pass=no

# Launch checks
for check in script/checks/*; do
	./$check
	if [[ $? != 0 ]]; then
		echo "Check '$check' didn't pass"
		a_check_didnt_pass=yes
	fi
done

# Error out if a check didn't pass
test "$a_check_didnt_pass" == 'no'
