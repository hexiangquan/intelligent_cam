#!/bin/sh
# run_app:
#		Check updates and run applications, if running failed, 
#		check and run backup app. 
# History:
# 2012/08/14	SK Sun	Create
# 2012/08/24	SK Sun	Using app name in internal define

# Show usage of this program
Usage() 
{
	PROGRAM=`basename $0`
	echo -e "$PROGRAM -- srcipts for running applicaiton"
	echo -e "running applications by following steps:"
	echo -e "\t1. check updates dir, copy update app to current dir if possible"
	echo -e "\t2. run each app"
	echo -e "\t3. if run cur app failed, try run app in backup dir"
	echo -e "Usage: $PROGRAM [start/stop/restart] "
	echo -e "Example: ./$PROGRAM restart\n"
}

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
			$APP &
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
		chmod +wx $APP
		echo "updating $APP done ..."
	fi
}

# Application list and install dir define
INSTALL_DIR="/home/root"
UPDATE_DIR="update"
BACKUP_DIR="backup"
APP_LIST="iCamera camCtrlSrv"

# StopApp -- Check app process and kill it
# @ Name -- Process name
# If current app is not killed by SIG_INT, this fuction will kill it by SIG_ABORT
StopApp()
{
	for APP in $APP_LIST; do
		PID=`GetPID $APP`
		if [ "-$PID" != "-" ]; then
			echo "Killing process $APP ..."
			kill -2 $PID
			sleep 1
		fi

		PID=`GetPID $APP`
		if [ "-$PID" != "-" ]; then
			echo "Killing process $APP by SIG_ABORT"
			kill -9 $PID
		fi
	done
}


# StartApp -- Start running app process
# @ Name -- Process name
# @ UpdateDir -- Dir for update
# @ BackupDir -- Dir for backup
# If current app is not killed by SIG_INT, this fuction will kill it by SIG_ABORT
StartApp()
{
	for APP in $APP_LIST; do
		echo -e "\n----- Run app $APP -----"
		CheckUpdate "$APP" "$UPDATE_DIR";
		if [ -e "$APP" ]; then
			./$APP &
			sleep 1
		fi
		CheckAppStatus "$APP" "$BACKUP_DIR";
	done
}

# Go to app install dir
if [ ! -d $INSTALL_DIR ]; then
	echo -e "install dir: $INSTALL_DIR not exist..."
	exit 1
fi

cd $INSTALL_DIR

# Operate by arguments
case "$1" in
      start) 
            StartApp
             ;;
       stop) 
            StopApp           
             ;;
       restart) 
            StopApp           
            StartApp
             ;;
        *)
            Usage $0
             ;;
esac

exit 0

