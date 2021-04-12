.DEFAULT_GOAL := all
.ONESHELL:
.SHELLFLAGS := -ec
SHELL := /bin/bash

ROS_DISTRO ?= melodic

# ROS sourcing
define source
	source /opt/ros/${ROS_DISTRO}/setup.bash
endef

# Faster build using all cores on tegra
define build
(
	if [ -x "$$(command -v nvpmodel)" ]  # if there's nvpmodel command,
	then
		last_nvpmode=$$(nvpmodel -q | sed -n 3p)   # get last mode
		if [ 0 -ne $${last_nvpmode} ]              # if it's not MaxN,
		then
			sudo nvpmodel -m 0                     # set MaxN
			function cleanup {
				sudo nvpmodel -m $${last_nvpmode}  # on exit from build, revert to the last mode
			}
			trap cleanup EXIT
		fi
	fi
	catkin build ${1} --cmake-args ${2}
)
endef