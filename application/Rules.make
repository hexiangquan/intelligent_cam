# Define target platform.
PLATFORM=dm368
#PLATFORM=x86

# The installation directory of the DVSDK.
DVSDK_INSTALL_DIR=/home/sun/work/dvsdk_dm368_4_02_00_06

# The directory that points to your kernel source directory.
LINUXKERNEL_INSTALL_DIR=$(DVSDK_INSTALL_DIR)/psp/linux-2.6.32.17-psp03.01.01.39

# The prefix to be added before the GNU compiler tools (optionally including # path), i.e. "arm_v5t_le-" or "/opt/bin/arm_v5t_le-".
CSTOOL_DIR=/opt/CodeSourcery/Sourcery_G++_Lite/
CSTOOL_PREFIX=$(CSTOOL_DIR)/bin/arm-none-linux-gnueabi-

MVTOOL_DIR=$(CSTOOL_DIR)
MVTOOL_PREFIX=$(CSTOOL_PREFIX)

# Project dir
PRJ_DIR=/home/sun/work/HDCAM

# Application root dir
APP_ROOT=$(PRJ_DIR)/application

# Where to copy the resulting executables
SDK_LIB_DIR=$(PRJ_DIR)/common/lib

# Where to find include files
SDK_INC_DIR=$(PRJ_DIR)/common/include

# Target file system path
TARGET_FILESYS_DIR=/home/sun/work/targetfs

# Target Exec dir
TARGET_EXEC_DIR=$(TARGET_FILESYS_DIR)/home/root 

# Target share lib dir
TARGET_LDLIB_DIR=$(TARGET_FILESYS_DIR)/usr/lib


