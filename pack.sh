#!/bin/sh
# Discription:
#		Pack and compress project 
# History:
# 2012/8/15		Sun 	Created


PRJ="hdcam"
EXT_DISC="/home/sun/work/ext-disc"
DST="$EXT_DISC/Projects/HDCAM/ARM/Application"

# Get current dir name
DIR=`pwd`
DIR=`basename $DIR`

DATE=`date +%Y.%m.%d`
TAR_FILE="$PRJ.$DATE.tar.gz"

cd ..

echo -e "tar dir $DIR to $TAR_FILE..."
tar zcf $TAR_FILE $DIR

if [ -d "$DST" ]; then
	echo -e "copy $TAR_FILE to $DST..."
	cp -p $TAR_FILE $DST 
fi

if [ "$1" == "umount" ]; then
	echo -e "umount ext disc..."
	sync
	sudo umount $EXT_DISC
fi

echo -e "done.\n"

exit 0
