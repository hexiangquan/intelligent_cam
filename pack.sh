#!/bin/sh
# Discription:
#		Pack and compress project 
# History:
# 2012/8/15		Sun 	Created

cd ..

DIR="HDCAM"
DATE=`date +%Y.%m.%d`
PRJ="hdcam"
DST="/home/sun/work/ext-disc/Projects/HDCAM/ARM/Application"
TAR_FILE="$PRJ.$DATE.tar.gz"

tar zcf $TAR_FILE $DIR

if [ $? == 0  ] && [ -d "$DST" ]; then
	cp -p $TAR_FILE $DST 
fi

exit 0
