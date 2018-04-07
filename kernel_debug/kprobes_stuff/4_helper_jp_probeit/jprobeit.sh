#!/bin/bash
# jprobeit.sh
# Wrapper script to help setup the jprobe(s) on a given file and function.
# 
# Kaiwan N Billimoria
# License: MIT
# 
name=$(basename $0)
source ./common.sh || {
 echo "$name: could not source common.sh , aborting..."
 exit 1
}

########### Functions follow #######################

# Function to validate passed as first parameter
older-check_function()
{
FUNC=$1
if [ -z $FUNC ]; then
	echo
	echo "*** $name: function name null, aborting now.."
	exit 1
fi
ShowTitle "[ Validate the to-be-Jprobed function ${FUNC} ]"

# Attempt to find out if it's valid in the kernel.
# In any case, if the function is invalid, it will be caught on the 
# register_jprobe(), which will then fail..

# What about embedded system which don't have either SYMLOC ??
if [ ! -f /proc/kallsyms ]; then
  if [ ! -f /boot/System.map-$(uname -r) ]; then
  	echo
    echo "$name: WARNING! Both /proc/kallsyms and /boot/System.map-$(uname -r) not present!?
[Possibly an embedded system]. 
So, we'll Not attempt to check validity of ${FUNC} right now; if invalid, it will 
be subsequently caught in the register_jprobe().
	"
	return
  fi
fi

if [ -f /proc/kallsyms ]; then
  SYMLOC=/proc/kallsyms
elif [ -f /boot/System.map-$(uname -r) ]; then
  SYMLOC=/boot/System.map-$(uname -r)
fi

grep "[tT] ${FUNC}" ${SYMLOC} || {
 echo
 echo "*** $name: FATAL: Symbol '${FUNC}' not found!
 [Either it's invalid -or- Could it be static or inline?]. Aborting..."
 exit 1
 }
}

show_other_possibilities()
{
 num=$(grep "[tT] .*${1}" ${SYMLOC} |wc -l)
 [ ${num} -ge 1 ] && {
  echo " Did you mean one of these functions?"
  grep "[tT] .*${1}" ${SYMLOC}
 }
}

# Function to validate passed as first parameter
check_function()
{
FUNC=$1
if [ -z $FUNC ]; then
	echo
	echo "*** $name: function name null, aborting now.."
	exit 1
fi
ShowTitle "[ Validate the to-be-Kprobed function ${FUNC} ]"

# Attempt to find out if it's valid in the kernel.
# Ultimately, if the function is invalid, it will be caught on the 
# register_kprobe(), which will then fail..

grep -w -q "${FUNC}" /sys/kernel/debug/kprobes/blacklist && {
 echo
 echo "*** $name: FATAL: Function '${FUNC}' cannot be probed, it's blacklisted. Aborting..."
 exit 1
}

# Check for existance
# What about embedded system which don't have either SYMLOC ??
if [ ! -f /proc/kallsyms ]; then
  if [ ! -f /boot/System.map-$(uname -r) ]; then
  	echo
    echo "$name: WARNING! Both /proc/kallsyms and /boot/System.map-$(uname -r) not present!?
[Possibly an embedded system]. 
So, we'll Not attempt to check validity of ${FUNC} right now; if invalid, it will 
be subsequently caught in the register_kprobe().
	"
	return
  fi
fi

if [ -f /proc/kallsyms ]; then
  SYMLOC=/proc/kallsyms
elif [ -f /boot/System.map-$(uname -r) ]; then
  SYMLOC=/boot/System.map-$(uname -r)
fi

grep -w "[tT] *${FUNC}" ${SYMLOC} || {
 echo
 echo "*** $name: FATAL: Symbol '${FUNC}' not found!
 [Either it's invalid -or- Could it be static or inline?]."
 show_other_possibilities ${FUNC}
 echo "Aborting..."
 exit 1
 }
num=$(grep -w "[tT] *${FUNC}" ${SYMLOC} |wc -l)
[ ${num} -gt 1 ] && {
 echo
 echo "*** $name: FATAL: Symbol '${FUNC}' - multiple instances found!
 [Sorry, we currently do not handle this case...]"
 show_other_possibilities ${FUNC}
 echo "Aborting..."
 exit 1
 }
} # end check_function()

setup_workspace()
{
ShowTitle "[ Setting up the Work Space ]"
DESTFILE=jp_${FUNCTION}_$(date +%d%b%y).c

BASEFOLDER=${TOPDIR}/work_jps
JPDEST=jp_${FUNCTION}_$(date +%d%b%y)
[ -d ${BASEFOLDER}/${JPDEST} ] && {
  echo
  echo "$name: !WARNING! Everything under ${BASEFOLDER}/${JPDEST} will now be OVERWRITTEN !!!
Contents:"
  ls -l ${BASEFOLDER}/${JPDEST}/
  echo "Press [Enter] to continue, ^C to abort..."
  read

  # Safety first! make a quick backup :-)
  mkdir -p ${BKP}/${JPDEST}
  cd ${BASEFOLDER}/${JPDEST}/
  cp -af *.[chS] *.sh Makefile ${BKP}/${JPDEST}/
  #cp ../../convenient.h . || exit 1
  rm -f *.mod.c
  cd -
 }

rm -rf ${BASEFOLDER}/${JPDEST}
mkdir -p ${BASEFOLDER}/${JPDEST}/
cp -f ${SRC} ${BASEFOLDER}/${JPDEST}/${DESTFILE}

export JPFILE=${BASEFOLDER}/${JPDEST}/${DESTFILE}
[ ! -f ${JPFILE} ] && {
 echo "*** $name: Fatal: Final dest file not existing? Aborting..."
 exit 1
}

#echo "+++++++++++++ check ++++++++++++++"
#pwd
#echo "chown ${ORIG_USER}:${ORIG_USER} ${BASEFOLDER}/${JPDEST}/*"
#chown ${ORIG_USER}:${ORIG_USER} ${BASEFOLDER}/${JPDEST}/*
cd 

#echo "JPFILE = ${JPFILE}"
ls -l ${JPFILE}
}

buildit()
{
## Jprobes use the mirror principle: the jprobe function handler MUST
# have the identical signature of the func (being jprobed).

ShowTitle " [  make -> LKM $JPMOD.ko ] "
make || {
  echo "$name: failed to 'make'. Aborting..."
  cd ..
  exit 1
 }
echo " Build done!"
ls -l $JPMOD.ko
}

# Function to jprobe passed as first parameter
src_jprobe()
{
FUNC=$1
ShowTitle "[ Dynamically Updating source of ${DESTFILE} ]"
#echo "JPFILE = $JPFILE"
cd ${BASEFOLDER}/${JPDEST}

#--- Source file update with sed !
# Insertions
sed --in-place "2 a\
 * ${DESTFILE} " ${JPFILE}
#sed --in-place "3 c\
#  * ${DESTFILE} " ${JPFILE}

# get rid of the '.c' extension
JPMOD=$(echo dummy |awk -v str=${DESTFILE} '{print substr(str, 1, length(str)-2)}')
sed --in-place -e "s/\#define MYNAME xxx/\#define MYNAME \"${JPMOD}\"/" ${JPFILE}

# Replace all occurences of 'xxx' in template src copy with $FUNC
sed --in-place -e "s/xxx/${FUNC}/g" ${JPFILE} || {
 echo "$name: Fatal: sed failed to replace stuff."
 exit 1
 }

#--- Generate the Makefile
cat > Makefile << @MYMARKER@
# Makefile for Jprobe kernel module
# Dynamically generated by ${name} !
#  For ${JPMOD}.c

ifneq (\$(KERNELRELEASE),)
	EXTRA_CFLAGS += -DDEBUG -DUSE_FTRACE_PRINT
	obj-m := $JPMOD.o

else
	#########################################
	# To support cross-compiling for the ARM:
	# For ARM, invoke make as:
	# make ARCH=arm CROSS_COMPILE=arm-none-linux-gnueabi- 
	ifeq (\$(ARCH),arm)
	# Update 'KDIR' below to point to the ARM Linux kernel source tree
		KDIR ?= ~/3.14.34
	else
		KDIR ?= /lib/modules/\$(shell uname -r)/build 
	endif
	#########################################
	PWD   := \$(shell pwd)
default:
	\$(info Dynamic Makefile:)
	\$(info Building with KERNELRELEASE = \${KERNELRELEASE}) 
	\$(MAKE) -C \$(KDIR) M=\$(PWD) modules
endif
clean:
	\$(MAKE) -C \$(KDIR) SUBDIRS=\$(PWD) clean
@MYMARKER@

# adjust for your workspace
#ln -s ../../../../../convenient.h
ln -s ../../convenient.h
ln -s ../../showlog.sh

ShowTitle " [ Source update, Makefile generation done.  ]"
echo "
$name:
Currently, attempting a direct make (compile) fails. The reason is straight-forward: 
we must remember that the kernel module source file is a _template_ and not 
entirely working source. Jprobes use the mirror principle: the jprobe function 
handler MUST have the identical signature of the func (being jprobed).

So, for now at least, this is as much as the script can do for you!
Now:
 a) cd ${BASEFOLDER}/${JPDEST}
 b) sudo /bin/bash    <-- need to work as root
 c) edit the source (${JPMOD}.c)
 d) make   
 e) and try it out!    :-)
"
#buildit

cd ${TOPDIR}
}


### "main" here ###
check_root_AIA
TOPDIR=$(pwd)
SRC=${TOPDIR}/jp_helper_template.c
BKP=${TOPDIR}/backup

if [ $# -lt 1 ]; then
  echo "Usage: $name function_to_jprobe
 1st param: {function_to_jprobe} :                           [REQUIRED]
 This is the name of the function to jprobe.
"
# echo "Usage: $name {new_jp_lkm_pathname.c} function_to_jprobe
# 
# 1st param: {new_jp_lkm_pathname.c} :                        [REQUIRED]
#  This is the pathname of the copy of the jprobe 'template' source file
#  (the C source you will subsequently edit, writing your jprobe handler(s)).
# 
 exit 1
fi

#DESTFILE=$1
FUNCTION=$1
check_function ${FUNCTION}
setup_workspace
src_jprobe ${FUNCTION}

exit 0
