
# the python interpreter
PYTHON=python
#PYTHON=pypy

# the libuv directory
LIBUV_DIR=libhttp_parser


##############################
# building
##############################

all: build

.PHONY: build
build: setup.py
	@echo ">>> Building up..."
	$(PYTHON) setup.py build_ext $@

##############################
# testing
##############################

.PHONY: tests
tests: test
test:
	@echo ">>> Running all tests..."
	@PYTHONPATH=`pwd` nosetests -v --with-isolation -a '!perf' -w tests

##############################
# cleaning
##############################

clean:
	@echo ">>> Cleaning up..."
	rm -rf build dist core.* http_parser/*.pyc
	rm -rf *.egg-info __pycache__ Library
	rm -f `find . -name '*.pyc'` `find http_parser -name '*.so'` `find http_parser -name '*.a'`
	rm -rf `find . -name '__pycache__'`

distclean: clean
	make -C libhttp-parser clean
	rm -rf `find libhttp-parser -name '*.o'`

##############################
# documentations
##############################

.PHONY: doc
doc: docs

docs: docs-html

docs-html: all
	@echo ">>> Making HTML documentation..."
	LANG=en_US.UTF-8  LC_ALL=en_US.UTF-8  make -C doc html

##############################
# redistribution
##############################

redist: dist

.PHONY: dist
dist: clean
	@echo ">>> Making redistributable package..."
	$(PYTHON) setup.py bdist
	@echo ">>> redistributable package left in dist/"

.PHONY: egg
egg: clean
	@echo ">>> Making redistributable egg..."
	$(PYTHON) setup.py bdist_egg
	@echo ">>> redistributable egg left in dist/"

dist-debug: clean
	DISTUTILS_DEBUG=1 make dist

sdist:
	@echo ">>> Making redistributable sources package..."
	$(PYTHON) setup.py sdist
	@echo ">>> redistributable package left in dist/"

sdist-debug: clean
	DISTUTILS_DEBUG=1 make sdist

sdist-upload: sdist
	@echo ">>> Uploading redistributable sources package to PyPI..."
	$(PYTHON) setup.py sdist upload

sdist-register: sdist
	@echo ">>> Registering package at PyPI..."
	$(PYTHON) setup.py register
