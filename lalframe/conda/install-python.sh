#!/bin/bash
#
# Configure, built, test, and install the Python language bindings
# for a LALSuite subpackage.
#

set -e
pushd ${SRC_DIR}

# when running on gitlab-ci, we are not using a production
# build, so we don't want to use NDEBUG
export CPPFLAGS="${CPPFLAGS} -UNDEBUG"

# configure only python bindings and pure-python extras
./configure \
	--prefix=$PREFIX \
	--disable-swig-iface \
	--enable-swig-python \
	--enable-python \
	--disable-doxygen \
	--enable-silent-rules || { cat config.log; exit 1; }

# build
make -j ${CPU_COUNT} -C swig
make -j ${CPU_COUNT} -C python

# install
make -j ${CPU_COUNT} -C swig install-exec-am  # swig bindings
make -j ${CPU_COUNT} -C python install  # pure-python extras

popd
