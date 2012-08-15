#!/bin/sh
# Discription:
#		Pack and compress project 
# History:
# 2012/8/15		Sun 	Created


PRJ="hdcam"
DST="/home/sun/work/ext-disc/Projects/HDCAM/ARM/Application"
DIR="HDCAM"

DATE=`date +%Y.%m.%d`
TAR_FILE="$PRJ.$DATE.tar.gz"

cd ..

echo -e "tar dir $DIR to $TAR_FILE..."
tar zcf $TAR_FILE $DIR

if [ -d "$DST" ]; then
	echo -e "copy $TAR_FILE to $DST..."
	cp -p $TAR_FILE $DST 
fi

echo -e "done.\n"

exit 0
