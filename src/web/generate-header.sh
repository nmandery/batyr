#!/bin/bash
# build the header containing all assets as binary strings

set -e

pushd static >/dev/null

echo "Generating header with the static assets"

# sort the files to keep the same order in the header
# and keep diffs in the version control system as small
# as possible
python ../tools/assets2header.py \
		--output ../httpassets.h \
        $(find . -type f | grep -rv '.swp' |sed 's#^\./##g' | sort)

popd >/dev/null
