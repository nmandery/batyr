#!/bin/bash
# build the header containing all resources as binary strings

set -eu

pushd static >/dev/null

echo "Generating header with the static resources"

# compress css.in files
find . -type f -name '*.css.in' | while read -r CSSIN; do
    CSSOUT=${CSSIN%.*}
    python ../tools/cssmin.py <"$CSSIN" >"$CSSOUT"
done 

# compress js.in files
find . -type f -name '*.js.in' | while read -r JSIN; do
    JSOUT=${JSIN%.*}
    python ../tools/jsmin.py <"$JSIN" >"$JSOUT"
done


# sort the files to keep the same order in the header
# and keep diffs in the version control system as small
# as possible
python ../tools/resources-to-header.py \
		--output ../http_resources.h \
        $(find . -type f ! -name '*.in' | grep -v '.swp' |sed 's#^\./##g' | sort)

popd >/dev/null
