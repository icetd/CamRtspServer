#! /usr/bin/bash
CURRENT_DIR=$(pwd)
X264_VERSION=20191217-2245
X264_SRC=x264-snapshot-${X264_VERSION}
X264_SRC_DIR=${CURRENT_DIR}/thirdparty/${X264_SRC}
X264_URL=http://download.videolan.org/pub/videolan/x264/snapshots/${X264_SRC}.tar.bz2
X264_PREFIX=${CURRENT_DIR}/thirdparty/x264

cd ${CURRENT_DIR}/thirdparty

if [ ! -d "${X264_PREFIX}" ]
then
	(wget -O "${X264_SRC}.tar" ${X264_URL} \
		&& tar -xf "${X264_SRC}.tar") || exit
	rm -f "${X264_SRC}.tar"
else
	echo "already build x264"
fi

if [ ! -d "${X264_PREFIX}" ]
then
	cd ${X264_SRC_DIR}
	(./configure --enable-shared --prefix=${X264_PREFIX}) || exit
	(make -j4 && make install) || exit
	cd ${CURRENT_DIR}
	(rm -rf "${X264_SRC_DIR}") || exit
else
	echo "already build x264"
fi
