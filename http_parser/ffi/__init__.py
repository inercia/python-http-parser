"""
libhttp_parser ffi module

Usage;

>>> from http_parser.ffi import C
>>> dir(C)
"""




from __future__ import absolute_import
import sys, os

from cffi import FFI



__HERE__ = os.path.dirname(__file__)

# where we have libuv installed
LIBHTTP_PARSER_DIR           = os.path.join(__HERE__, '..', '..', 'libhttp-parser')
LIBHTTP_PARSER_INC_DIR       = LIBHTTP_PARSER_DIR
LIBHTTP_PARSER_LIB_DIR       = LIBHTTP_PARSER_DIR

LIBHTTP_PARSER_FFI_H_FILE    = os.path.join(__HERE__, '_ffi.h')
LIBHTTP_PARSER_FFI_C_FILE    = os.path.join(__HERE__, '_ffi.c')
LIBHTTP_PARSER_FFI_C_EXTRA_FILE = os.path.join(__HERE__, '_ffi_extra.c')

FFI_SO              = '_libhttp_parser.so'
FULL_FFI_SO         = os.path.join(os.path.dirname(__file__), FFI_SO)

EXTENSION_PACKAGE   = 'http_parser.ffi'      # must match the package defined in setup.py


_ffi_so = None
C = None


# check if we need any extra libraries...
cflags = []
ldflags = []
extra_libs = []

if sys.platform in ['linux', 'linux2']:
    extra_libs += ['rt']

if sys.platform in ['darwin']:
    cflags += [' -arch x86_64 -arch i386 -framework CoreServices ']
    ldflags += [' -framework CoreServices ']


## NOTE: do not ask me why, but we need to set the env flags or distutils will ignore them...
if len(cflags) > 0:         os.environ["CFLAGS"] = ' '.join(cflags)
if len(ldflags) > 0:        os.environ["LDFLAGS"] = ' '.join(ldflags)


ffi = FFI()
ffi.cdef(open(LIBHTTP_PARSER_FFI_H_FILE).read())

## evaluate if we have already compiled the ffi... it should be done just once
if _ffi_so is None:
    _ffi_so = os.path.exists(FULL_FFI_SO)

if C is None:
    C = ffi.verify(open(LIBHTTP_PARSER_FFI_C_FILE).read(),
                   include_dirs = [LIBHTTP_PARSER_INC_DIR],
                   sources = [LIBHTTP_PARSER_FFI_C_EXTRA_FILE],
                   libraries = ['http_parser'] + extra_libs,
                   library_dirs = [LIBHTTP_PARSER_LIB_DIR],
                   ext_package = EXTENSION_PACKAGE)


