#!/bin/bash

git submodule init
git submodule sync
git submodule update

export PYTHON=/usr/local/bin/python3.10
export BB_PYTHON=/usr/local/bin/python3.10

# local.conf won't exist until this step on first execution
source poky/oe-init-build-env

CONFLINE="MACHINE = \"raspberrypi4-64\""

cat conf/local.conf | grep "${CONFLINE}" > /dev/null
local_conf_info=$?

if [ $local_conf_info -ne 0 ];then
	echo "Append ${CONFLINE} in the local.conf file"
	echo ${CONFLINE} >> conf/local.conf
	
else
	echo "${CONFLINE} already exists in the local.conf file"
fi

# Enable rm_work to reduce disk usage
RMWORK_LINE='INHERIT += "rm_work"'
EXCLUDE_LINE='RM_WORK_EXCLUDE += "core-image-aesd"'
EXCLUDE_LINE_2='RM_WORK_EXCLUDE += "aesd-assignments"'

grep -q "${RMWORK_LINE}" conf/local.conf
if [ $? -ne 0 ]; then
    echo "Appending rm_work inherit to local.conf"
    echo ${RMWORK_LINE} >> conf/local.conf
else
    echo "rm_work already enabled in local.conf"
fi

grep -q "${EXCLUDE_LINE}" conf/local.conf
if [ $? -ne 0 ]; then
    echo "Excluding core-image-aesd from rm_work"
    echo ${EXCLUDE_LINE} >> conf/local.conf
else
    echo "core-image-aesd already excluded from rm_work"
fi

grep -q "${EXCLUDE_LINE_2}" conf/local.conf
if [ $? -ne 0 ]; then
    echo "Excluding aesd-assignments from rm_work"
    echo ${EXCLUDE_LINE_2} >> conf/local.conf
else
    echo "aesd-assignments already excluded from rm_work"
fi

bitbake-layers show-layers | grep "meta-aesd" > /dev/null
layer_info=$?

if [ $layer_info -ne 0 ];then
	echo "Adding meta-aesd layer"
	bitbake-layers add-layer ../meta-aesd
else
	echo "meta-aesd layer already exists"
fi

set -e
bitbake core-image-aesd


