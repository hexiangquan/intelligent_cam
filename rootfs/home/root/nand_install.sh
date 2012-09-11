#!/bin/sh
# Program:
#		This script mount nand flash, installs application, kernel modules, etc.
# History:
# 2012/08/18	Sun		Created

MNT="/mnt"

# Mount flash
echo -e "\nMount nand flash..."
ubiattach -m 3
mount -t ubifs ubi0:rootfs $MNT

# Copy application
echo -e "\nInstall applications..."
INSTALL_DIR="/home/root"
APP_LIST="iCamera camCtrlSrv camBroadcast"
MISC_LIST="run_app.sh fpga.rbf"
LIST_ALL="$APP_LIST $MISC_LIST"
MNT="/mnt"

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
		cp -p "$APP" "$BACKUP_DIR"
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


