# Define target platform.
PLATFORM=dm368
#PLATFORM=x86

# The directory that points to your kernel source directory.
LINUXKERNEL_INSTALL_DIR=$(DVSDK_INSTALL_DIR)/psp/linux-2.6.32.17-psp03.01.01.39

# The prefix to be added before the GNU compiler tools (optionally including # path), i.e. "arm_v5t_le-" or "/opt/bin/arm_v5t_le-".
CSTOOL_DIR=/opt/CodeSourcery/Sourcery_G++_Lite/
CSTOOL_PREFIX=$(CSTOOL_DIR)/bin/arm-none-linux-gnueabi-

MVTOOL_DIR=$(CSTOOL_DIR)
MVTOOL_PREFIX=$(CSTOOL_PREFIX)

# Project dir
PRJ_DIR=/home/sun/work/HDCAM

# Target file system path
TARGET_FILESYS_DIR=/home/sun/work/targetfs/base-fs

# Target Exec dir
TARGET_EXEC_DIR=$(TARGET_FILESYS_DIR)/home/root 

# Target share lib dir
TARGET_LDLIB_DIR=$(TARGET_FILESYS_DIR)/usr/lib

