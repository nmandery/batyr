#!/usr/bin/env python
# generate the manual html partial
# usage: generate-manual-html.py MARKDOWN_FILE HTML_TEMPLATE OUTPUT

import sys
import subprocess

template_html = open(sys.argv[2]).read()
rendered_manual = subprocess.check_output(['markdown', '-f', '+autolink', sys.argv[1]])
open(sys.argv[3], 'w').write(template_html.replace('@MANUAL_HTML@', rendered_manual))
