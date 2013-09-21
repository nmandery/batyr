#!/bin/bash
# build the header containing all resources as binary strings

set -e

pushd static >/dev/null

echo "Generating header with the static resources"

# sort the files to keep the same order in the header
# and keep diffs in the version control system as small
# as possible
python ../tools/resources-to-header.py \
		--output ../http_resources.h \
        $(find . -type f | grep -v '.swp' |sed 's#^\./##g' | sort)

popd >/dev/null
