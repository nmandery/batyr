#!/bin/bash
# build the header containing all resources as binary strings

set -eu

pushd static >/dev/null

DO_COMPRESS=yes

echo "Generating header with the static resources"

cat >css/batyr.css.in \
    css/batyr.in/pure.css \
    css/batyr.in/font-awesome.css \
    css/batyr.in/style.css

# compress css.in files
find . -type f -name '*.css.in' ! -path '*.in/*' | while read -r CSSIN; do
    CSSOUT=${CSSIN%.*}
    if [ "$DO_COMPRESS" == "yes" ]; then
        python ../tools/cssmin.py <"$CSSIN" >"$CSSOUT"
    else
        cp "$CSSIN" "$CSSOUT"
    fi
done 


cat >js/lib.js \
    js/lib.in/jquery-1.11.0.min.js \
    js/lib.in/moment.min.js \
    js/lib.in/angular.min.js \
    js/lib.in/angular-route.min.js

cat >js/app.js.in \
    js/app.in/angular-notify.js \
    js/app.in/app.js \
    js/app.in/controllers.js

# compress js.in files
find . -type f -name '*.js.in' ! -path '*.in/*' | while read -r JSIN; do
    JSOUT=${JSIN%.*}
    if [ "$DO_COMPRESS" == "yes" ]; then
        python ../tools/jsmin.py <"$JSIN" >"$JSOUT"
        #java -jar ../tools/closure-compiler.jar --js "$JSIN" --js_output_file "$JSOUT"
    else
        cp "$JSIN" "$JSOUT"
    fi
done


# sort the files to keep the same order in the header
# and keep diffs in the version control system as small
# as possible
python ../tools/resources-to-header.py \
		--output ../http_resources.h \
        $(find . -type f ! -name '*.swp' | grep -v '.in/' | sed 's#^\./##g' | sort)

popd >/dev/null
