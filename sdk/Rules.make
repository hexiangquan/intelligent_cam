# Define target platform.
PLATFORM=dm368
#PLATFORM=x86

# The installation directory of the DVSDK.
DVSDK_INSTALL_DIR=/home/sun/work/dvsdk_dm368_4_02_00_06

# Where the Codec Engine package is installed.
CE_INSTALL_DIR=$(DVSDK_INSTALL_DIR)/codec-engine_2_26_02_11

# Where the codecs are installed.
CODEC_INSTALL_DIR=$(DVSDK_INSTALL_DIR)/codecs-dm365_4_02_00_00

# Where DMAI package is installed.
DMAI_INSTALL_DIR=$(DVSDK_INSTALL_DIR)/dmai_2_20_00_15

# Where the SDK demos are installed
DEMO_INSTALL_DIR=$(DVSDK_INSTALL_DIR)/dvsdk-demos_4_02_00_01

# Where the DVTB package is installed.
DVTB_INSTALL_DIR=$(DVSDK_INSTALL_DIR)/dvtb_4_20_18

# Where the Framework Components package is installed.
FC_INSTALL_DIR=$(DVSDK_INSTALL_DIR)/framework-components_2_26_00_01

# Where the DM365mm module is installed.
DM365MM_MODULE_INSTALL_DIR=$(DVSDK_INSTALL_DIR)/dm365mm-module_01_00_03

# Where the PSP is installed.
PSP_INSTALL_DIR=$(DVSDK_INSTALL_DIR)/psp

# Where the MFC Linux Utils package is installed.
LINUXUTILS_INSTALL_DIR=$(DVSDK_INSTALL_DIR)/linuxutils_2_26_01_02
CMEM_INSTALL_DIR=$(LINUXUTILS_INSTALL_DIR)

# Where the XDAIS package is installed.
XDAIS_INSTALL_DIR=$(DVSDK_INSTALL_DIR)/xdais_6_26_01_03

# Where the RTSC tools package is installed.
XDC_INSTALL_DIR=$(DVSDK_INSTALL_DIR)/xdctools_3_16_03_36

# The directory that points to your kernel source directory.
LINUXKERNEL_INSTALL_DIR=$(DVSDK_INSTALL_DIR)/psp/linux-2.6.32.17-psp03.01.01.39

# Where temporary Linux headers and libs are installed.
LINUXLIBS_INSTALL_DIR=$(DVSDK_INSTALL_DIR)/linux-devkit/arm-none-linux-gnueabi/usr

# The prefix to be added before the GNU compiler tools (optionally including # path), i.e. "arm_v5t_le-" or "/opt/bin/arm_v5t_le-".
CSTOOL_DIR=/opt/CodeSourcery/Sourcery_G++_Lite/
CSTOOL_PREFIX=$(CSTOOL_DIR)/bin/arm-none-linux-gnueabi-

MVTOOL_DIR=$(CSTOOL_DIR)
MVTOOL_PREFIX=$(CSTOOL_PREFIX)

# Project dir
PRJ_DIR=/home/sun/work/HDCAM

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

# version of kernel
KERNEL_VERSION = 2.6.32.17-davinci1

# Target kernel module install dir
TARGET_MOD_DIR=$(TARGET_FILESYS_DIR)/lib/modules/$(KERNEL_VERSION)/kernel

# SDK external resources dir
SDK_RES_DIR=$(PRJ_DIR)/sdk/resource

# SDK linked lib header dir
SDK_LIB_INC_DIR=$(SDK_RES_DIR)/include

# SDK linked lib dir
#SDK_LD_LIB_DIR=$(SDK_RES_DIR)/lib

# Where to output configuration files
ALG_XDC_CFG=$(PRJ_DIR)/sdk/alg/alg_config

# XDC compiler options
ALG_XDC_CFLAGS=$(ALG_XDC_CFG)/compiler.opt

# Output linker file
ALG_XDC_LFILE=$(ALG_XDC_CFG)/linker.cmd

