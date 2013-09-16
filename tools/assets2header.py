#!/usr/bin/env python
"""

Usage:
assets2header.py

# TODO
escaping of filenames and mimetypes
cleanup

"""

import sys
import os.path
import subprocess
import argparse
from contextlib import contextmanager
import hashlib

def slug(text, encoding=None,
         permitted_chars='abcdefghijklmnopqrstuvwxyz0123456789_',
         replace_char='_'):
    if isinstance(text, str):
        text = text.decode(encoding or 'ascii')
    text = map(lambda x: x if x in permitted_chars else replace_char, text)
    while (2*replace_char) in text:
        text = text.replace(2*replace_char, replace_char)
    return ''.join(text)


def c_escape(s):
    return s.replace('\\', '\\\\').replace('"', '\"')

def fail(msg, rc=1):
    sys.stderr.write(msg)
    sys.stderr.write('\n')
    sys.exit(rc)


class Asset(object):
    filename=None
    filesize=0
    indenting = 4

    def __init__(self, filename):
        self.indenting = 4
        self.filename = filename
        self.filesize = os.path.getsize(self.filename)

    def slugname(self):
        return slug(self.filename).lower().strip()

    def cvar_data(self):
        return "asset_%s_data" % (self.slugname(),)

    def etag(self):
        m = hashlib.sha1()
        m.update(open(self.filename).read())
        return m.hexdigest()

    def mimetype(self):
        if self.filename.endswith('.js'):
            return 'application/javascript'
        elif self.filename.endswith('.css'):
            return 'text/css'
        elif self.filename.endswith('.html'):
            return 'text/html'
        return subprocess.check_output(['file', '-b', '-i', self.filename]).strip()

    @classmethod
    def declaration(self):
        return """#include <stddef.h>

struct asset_info {
   const char * filename;
   const char * mimetype;
   const unsigned char * data;
   size_t size_in_bytes;
   const char * etag;
};
"""

    def write_data(self, fh):
        # the binary data of the file
        fh.write("static unsigned char %s[] = {\n%s" % (self.cvar_data(), self.indenting * ' '))
        fh_in = open(self.filename, 'r')
        data = fh_in.read()
        data_len = len(data)
        i = 0
        for i in range(data_len):
            next_i = i + 1
            fh.write("%#x%s" % (ord(data[i]), ', ' if next_i != data_len else ''))
            if next_i != data_len:
                if (next_i%12) == 0:
                    fh.write("\n%s" % (self.indenting * ' ',))
        fh.write("\n};\n")


    def metadata(self):
        return """%(indenting)s{ "%(filename)s", "%(mimetype)s", %(cvar_data)s, %(filesize)d, "%(etag)s" }""" % {
            'filename': self.filename,
            'cvar_data': self.cvar_data(),
            'mimetype': self.mimetype(),
            'filesize': self.filesize,
            'indenting': self.indenting*' ',
            'etag': self.etag()
        }


@contextmanager
def writeable_headerfile(filename, language='C'):
    with open(filename, 'w') as fh:

        h_name = slug(os.path.basename(filename).lower())
        guard_name = "__%s__" % (h_name.upper())

        # header guard start
        if language == 'C':
            fh.write("#ifndef %s\n#define %s\n\n" % (guard_name, guard_name))
            fh.write("#ifdef __cplusplus\nextern \"C\" {\n#endif\n\n")


        yield fh

        # header guard end
        if language == 'C':
            fh.write("#ifdef __cplusplus\n}\n#endif\n\n")
            fh.write("\n#endif // %s\n" % (guard_name,))
        fh.close()


if __name__ == '__main__':
    aparser = argparse.ArgumentParser(description=sys.modules[__name__].__doc__)
    aparser.add_argument('files', metavar='F', type=str, nargs='+',
                               help='a file to include in the asset header')
    aparser.add_argument('--output', dest='output', type=str, required=True,
                            help='the file to write the header to')

    args = aparser.parse_args()

    with writeable_headerfile(args.output) as fh:

        fh.write(Asset.declaration())

        asset_infos = []
        args.files.sort()
        for f in args.files:
            asset = Asset(f)
            asset.write_data(fh)
            asset_infos.append(asset.metadata())

        # write an integer containing the number of assets in this file
        fh.write("\n\nstatic size_t assets_count = %d;\n" % len(asset_infos))

        # write an assets struct listing all assets of this file
        fh.write("\nstatic struct asset_info assets[] = {\n")
        fh.write(',\n'.join(asset_infos))
        fh.write("\n};\n\n")
