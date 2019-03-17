#!/bin/bash
# .sh
# 
# Quick Description:
# 
# 
# Last Updated :
# Created      :
# 
# Author:
# Kaiwan N Billimoria
# kaiwan -at- kaiwantech -dot- com
# kaiwanTECH
# 
# License:
# MIT License.
# 
name=$(basename $0)
PFX=$(dirname `which $0`)
source ${PFX}/common.sh || {
 echo "${name}: fatal: could not source ${PFX}/common.sh , aborting..."
 exit 1
}

########### Globals follow #########################
# Style: gNameOfGlobalVar


########### Functions follow #######################



start()
{


}

##### 'main' : execution starts here #####

[ $# -ne 1 ] && {
  echo "Usage: ${name} "
  exit 1
}
start
exit 0
