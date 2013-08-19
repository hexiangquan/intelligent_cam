#!/bin/sh
# update.sh:
#		Check updates for application, 
# History:
# 2013/08/17	SK Sun	Create

# Show usage of this program
Usage() 
{
	PROGRAM=`basename $0`
	echo -e "$PROGRAM -- srcipts for checking system and application updates" 
	echo -e "running applications by following steps:"
	echo -e "Usage: $PROGRAM"
	echo -e "Example: ./$PROGRAM \n"
}

UPDATE_PACK="update.tar.gz"
UDDATE_PROG="run.sh"

# UpdatePacket -- Check update packet and run this packet
# If a update packet exist, unpack and run the update script
UpdatePacket()
{
	if [ -e "$UPDATE_PACK" ]; then
		echo -e "detect update package..."
		tar zvxf $UPDATE_PACK
		if [ $0 == "0" ] && [ -f "$UPDATE_PROG" ]; then
			chmod +x $UPDATE_PROG
			./$UPDATE_PROG	
		fi
		rm $UPDATE_PACK
	fi
}

# Go to app install dir
UPDATE_DIR="/home/root/update"

if [ ! -d $UPDATE_DIR ]; then
	echo -e "update dir: $UPDATE_DIR not exist..."
	exit 1
fi

cd $UPDATE_DIR

UpdatePacket;

# Update kernel 
KERNEL="uImage"
if [ -e "$KERNEL" ]; then
	echo -e "Install kernel image ..."
	flash_eraseall /dev/mtd2
	nandwrite -pm /dev/mtd2 $KERNEL 
	echo -e "Write kernel complete."
	rm $KERNEL
fi

# Update kernel modules
MOD_DIR="/lib/modules/2.6.32.17-davinci1/kernel/drivers/dsp"
MOD="*.ko"

#if [ "${##*.}" = "ko" ]; then
	echo -e "Updating kernel modules..."
	mv $MOD $MOD_DIR 2> /dev/null
	depmod -a
#fi

# Make scripts and programs executable
chmod +x * 2> /dev/null

# Update shell scripts
if [ -e "rc.local" ]; then
	echoe -e "Updating rc.local"
	mv rc.local /etc
fi	

if [ -e "loadmodule-rc" ]; then
	echoe -e "Updating loadmodule-rc"
	mv loadmodule-rc /etc/init.d
fi	

# Move all left to install dir
INSTALL_DIR="/home/root/"

echo -e "Copy application and scripts..."
#if [ -d $INSTALL_DIR ] && [ -e * ]; then
	mv * $INSTALL_DIR 2> /dev/null
#fi

sync

exit 0


