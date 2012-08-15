#!/bin/sh
# run_app:
#		Check updates and run applications, if running failed, 
#		check and run backup app. 
# History:
# 2012/08/14	SK Sun	Create

# Check arguments
if [ $# -lt 4 ]; then
	echo -e "$0 -- srcipts for running applicaiton\n"
	echo -e "           1. check updates dir, copy update app to current dir if possible"
	echo -e "           2. run each app"
	echo -e "           3. if run cur app failed, try run app in backup dir"
	echo -e "\n  Usage: run_app APP_DIR UPDATE_SUB_DIR BACKUP_SUB_DIR APP1 APP2 ... "
	echo -e "  Example: run_app.sh /home/root update backup iCamera camCtrlSrv\n"
	exit 1
fi

# Get process PID:
# @ Name -- process name
GetPID() 
{
	PsName=$1
	PID=`ps |grep $PsName |grep -v grep |sed -n 1p |awk '{print $1}'`
	echo $PID
}

# CheckAppStatus -- Check if app process is still running
# @ Name -- Process name
# @ Backup_Dir -- Dir that includes backup app
# If current app is not running, this fuction will try running backup verison
CheckAppStatus()
{
	APP=$1
	PID=`GetPID $APP`

	if [ "-$PID" == "-" ]; then
		echo "Process $APP is not running."
		# Run backup app if exist
		APP="$2/$1"
		if [ -f "$APP" ]; then
			echo -e "Running backup app: $APP.\n"
			$APP
		else
			echo -e "Can't find backup: $APP\n"
		fi
	else
		echo -e "Process $APP running ok.\n"
	fi
}

# CheckUpdate -- Check update file and overwrite current version
# @ Name -- Process Name
# @ Update_Dir -- Dir that includes update file
CheckUpdate()
{
	APP=$1
	DIR=$2

	# Check update dir exist and update app file exist
	if [ -d "$DIR" ] && [ -f "$DIR/$APP" ]; then
		mv "$DIR/$APP" .
		echo "updating $APP done ..."
	fi
}

# Check if dir and file exits
if [ ! -d "$1" ]; then
	echo -e "$1 is not a dir\n"
	exit 1
fi

# Go to app dir
cd $1

UPDATE_DIR="$2"
BACKUP_DIR="$3"

# Check update, run each app and check status
for APP in $*; do
	if [ ! -d "$APP" ]; then
		echo -e "\n Run app $APP"
		CheckUpdate "$APP" "$UPDATE_DIR";
		./$APP &
		sleep 1
		CheckAppStatus "$APP" "$BACKUP_DIR";
	fi
done

exit 0

