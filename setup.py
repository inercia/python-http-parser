# coding=utf8

import os
import errno
import shutil
import subprocess
import sys

try:
    from setuptools import setup, find_packages, Extension
except ImportError:
    from distutils.core import setup, find_packages, Extension

__HERE__ = os.path.dirname(__file__)
__VERSION_FILE__ = os.path.join(__HERE__, 'VERSION')
__VERSION__ = open(__VERSION_FILE__).read().strip()
__README_FILE__ = os.path.join(__HERE__, 'README.md')


def rmtree (path):
    try:
        shutil.rmtree(path)
    except OSError as e:
        if e.errno != errno.ENOENT:
            raise


def exec_process (cmdline, silent = True, input = None, **kwargs):
    """
    Execute a subprocess and returns the returncode, stdout buffer and stderr buffer.
    Optionally prints stdout and stderr while running.
    """
    try:
        sub = subprocess.Popen(args = cmdline, stdin = subprocess.PIPE, stdout = subprocess.PIPE,
                               stderr = subprocess.PIPE, **kwargs)
        stdout, stderr = sub.communicate(input = input)
        returncode = sub.returncode
        if not silent:
            sys.stdout.write(stdout)
            sys.stderr.write(stderr)
    except OSError as e:
        if e.errno == errno.ENOENT:
            raise Exception('"%s" is not present on this system' % cmdline[0])
        else:
            raise

    if returncode != 0:
        raise Exception('Got return value %d while executing "%s", stderr output was:\n%s' % (
        returncode, " ".join(cmdline), stderr.rstrip("\n")))

    return stdout


class LibHttpParserBuild(object):
    libhttp_parser_dir = os.path.join(__HERE__, 'libhttp-parser')
    libhttp_parser_patches = []

    def __init__ (self):
        self.libhttp_parser_clean_compile = 0
        self.libhttp_parser_force_fetch = 0


    def get_extension (self):

        def patch_libhttp_parser ():
            if self.libhttp_parser_patches:
                print 'Patching libhttp-parser...'
                for patch_file in self.libhttp_parser_patches:
                    exec_process(['patch', '--forward', '-d', self.libhttp_parser_dir, '-p0', '-i',
                                  os.path.abspath(patch_file)])

        def build_libhttp_parser ():
            cflags = '-fPIC'
            env = os.environ.copy()
            env['CFLAGS'] = ' '.join(x for x in (cflags, env.get('CFLAGS', None)) if x)
            print 'Building libhttp_parser...'
            if sys.platform is 'win32':
                exec_process('cmd.exe /C vcbuild.bat release', silent = False, cwd = self.libhttp_parser_dir,
                             env = env, shell = True)
            else:
                exec_process(['make', 'package'], silent = False, cwd = self.libhttp_parser_dir, env = env)

        if sys.platform in ['linux', 'linux2', 'darwin']:
            self.libhttp_parser_lib = os.path.join(self.libhttp_parser_dir, 'libhttp_parser.a')
        else:
            self.libhttp_parser_lib = os.path.join(self.libhttp_parser_dir, 'Release', 'lib', 'libhttp_parser.lib')

        if self.libhttp_parser_clean_compile:
            if sys.platform is 'win32':
                exec_process('cmd.exe /C vcbuild.bat clean', cwd = self.libhttp_parser_dir, shell = True)
                rmtree(os.path.join(self.libhttp_parser_dir, 'Release'))
            else:
                exec_process(['make', 'clean'], cwd = self.libhttp_parser_dir)

        if not os.path.exists(self.libhttp_parser_lib):
            print 'libhttp_parser needs to be compiled.'
            patch_libhttp_parser()
            build_libhttp_parser()
        else:
            print 'No need to build libhttp_parser.'

        ## just set the CFFI-http_parser extension
        from http_parser.ffi import ffi
        try:
            return [ffi.verifier.get_extension()]
        except AttributeError:
            return []


libhttp_parser_build = LibHttpParserBuild()

setup(name = "python-http-parser",
      version = __VERSION__,
      author = "Alvaro Saurin",
      author_email = "alvaro.saurin@gmail.com",
      url = "http://github.com/inercia/python-http-parser",
      description = "Python interface for http-parser",
      long_description = open(__README_FILE__).read(),

      packages = find_packages(exclude = ['tests']),
      package_data = {'http_parser': ['_ffi.[ch]']},

      platforms = [
          "POSIX",
          "Microsoft Windows"
      ],

      ext_package = 'http_parser.ffi',
      ext_modules = libhttp_parser_build.get_extension(),

      zip_safe = False,

      classifiers = [
          "Development Status :: 4 - Beta",
          "Intended Audience :: Developers",
          "License :: OSI Approved :: MIT License",
          "Operating System :: POSIX",
          "Operating System :: Microsoft :: Windows",
          "Programming Language :: Python",
          "Programming Language :: Python :: 2",
          "Programming Language :: Python :: 2.6",
          "Programming Language :: Python :: 2.7",
      ],

)

