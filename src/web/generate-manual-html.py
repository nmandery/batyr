#!/usr/bin/env python
# generate the manual html partial
# usage: generate-manual-html.py MARKDOWN_FILE HTML_TEMPLATE OUTPUT

import sys
import subprocess
import os

markdown_binary = os.environ.get("MARKDOWN_EXECUTABLE", "markdown")

template_html = open(sys.argv[2]).read()
rendered_manual = subprocess.check_output([markdown_binary, '-f', '+autolink', sys.argv[1]])
open(sys.argv[3], 'w').write(template_html.replace('@MANUAL_HTML@', rendered_manual))
