#!/bin/sh
# Program:
#		This script mount nand flash, installs application, kernel modules, etc.
# History:
# 2012/08/18	Sun		Created

# Show usage of this program
Usage() 
{
	PROGRAM=`basename $0`
	echo -e "$PROGRAM -- srcipts for install rootfs and applications" 
	echo -e  "\tuse [all] for erase flash and install kernel, rootfs and apps"
	echo -e  "\tuse [app] for only install applications"
	echo -e  "\tuse [kernel] for only install kernel"
	echo -e "Usage: $PROGRAM [all/app] "
	echo -e "Example: ./$PROGRAM restart\n"
	exit 0
}

# Operate by arguments
if [ "$1" != "all" ] && [ "$1" != "app" ] && [ "$1" != "kernel" ]; then
	Usage $0
fi

MNT="/mnt"
ROOTFS="/boot/rootfs.tar.gz"
KERNEL="/boot/uImage"

InstallKernel()
{
	if [ -e "$KERNEL" ]; then
		echo -e "Install kernel image ..."
		flash_eraseall /dev/mtd2
		nandwrite -pm /dev/mtd2 $KERNEL 
	else
		echo -e "Kernel file [$KERNEL] does not exit..."
	fi
}


if [ "$1" == "kernel" ]; then
	InstallKernel;
	exit 0
fi

if [ "$1" == "all" ]; then
	InstallKernel;
	echo -e "Erase mtd block"
	flash_eraseall /dev/mtd3
	echo -e "Making UBI filesystem..."
	ubiattach -m 3
	ubimkvol /dev/ubi0 -s 50MiB -N rootfs
	mount -t ubifs ubi0:rootfs $MNT
	if [ -e "$ROOTFS" ]; then
		echo -e "Extracting rootfs to $MNT..."
		tar zxf $ROOTFS -C $MNT
	else
		echo -e "Can't find rootfs file: [$ROOTFS], abort!"
		exit 1
	fi
else 
	# Mount flash
	echo -e "\nMount nand flash..."
	ubiattach -m 3
	mount -t ubifs ubi0:rootfs $MNT
fi

# Copy application
echo -e "\nInstall applications..."
INSTALL_DIR="/home/root"
APP_LIST="iCamera camCtrlSrv camBroadcast dspSrv"
MISC_LIST="run_app.sh fpga.rbf"
LIST_ALL="$APP_LIST $MISC_LIST"

for APP in $LIST_ALL; do
	if [ -e "$INSTALL_DIR/$APP" ] ; then
		echo -e "Install [$APP] to $MNT/$INSTALL_DIR"
		cp "$INSTALL_DIR/$APP" "$MNT/$INSTALL_DIR"
	fi
done

# Mkdir for update and backup
UPDATE_DIR="$MNT/$INSTALL_DIR/update"
BACKUP_DIR="$MNT/$INSTALL_DIR/backup"
if [ ! -d $UPDATE_DIR ]; then
	echo -e "make dir $UPDATE_DIR"
	mkdir -p $UPDATE_DIR
fi

if [ ! -d $BACKUP_DIR ]; then
	echo -e "make dir $BACKUP_DIR"
	mkdir -p $BACKUP_DIR
fi

for APP in $APP_LIST; do
	if [ ! -e "$BACKUP_DIR/$APP" ]; then
		echo -e "Copy [$APP] to $BACKUP_DIR"
		cp -p "$INSTALL_DIR/$APP" "$BACKUP_DIR"
	fi
done

# Copy kernel modules
echo -e "\nInstall kernel modules..."
MOD_DIR="/lib/modules/2.6.32.17-davinci1/kernel/drivers/dsp"
cd $MOD_DIR
MOD="*.ko"
cp $MOD "$MNT/$MOD_DIR"

# Copy shell scripts
echo -e "\nInstall shell scripts..."
SH_LIST="/etc/rc.local /etc/init.d/loadmodule-rc"
for SH in $SH_LIST; do
	if [ -e "$SH" ]; then
		echo -e "Copy scripts $SH"
		cp -p $SH "${MNT}/$SH"
	fi
done

echo -e "\nlink rc.local..."
cd "$MNT/etc/rc5.d"
LN_FILE="S99rclocal"
if [ -e $LN_FILE ]; then
	rm -f $LN_FILE
fi
ln -s ../rc.local $LN_FILE

# Sync with file system
sync

echo -e "\ndone."

exit 0


