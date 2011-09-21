#!/bin/bash

# Release Script 1.0

CAJAL_DIR=
ESPINA_DIR=
REPO=
DIST=

if [ "$CAJAL_DIR" = "" ]; then
	echo "libCajal path unspecified. Please, add path info"
	return
fi

if [ "$ESPINA_DIR" = "" ]; then
	echo "espina path unspecified. Please, add path info"
	return
fi

if [ "$REPO" = "" ]; then
	echo "Git repository unspecified. Please, add repository info"
	return
fi
if [ "$DIST" = "" ]; then
	echo "Ubuntu Distribution unspecified. Please, add distribution info"
	return
fi


echo "\033[32mUPDATING LIBCAJAL\033[37m"
cd $CAJAL_DIR
echo "\033[32mPulling from release repository:\033[37m"
git pull $REPO release-1.0
cd build
echo "\033[32mUpdating libCajal's CMake Configuration\033[37m"
cmake ..
echo "\033[32mCompiling libCajal\033[37m"
make -j2

echo "\033[32mUPDATING ESPINA\033[37m"
cd $ESPINA_DIR
echo "\033[32mStashing Local Changes\033[37m"
git stash
echo "\033[32mPulling from release repository:\033[37m"
git pull $REPO release-1.0
echo "\033[32mUnstashing Local Changes\033[37m"
git stash pop
cd build
echo "\033[32mUpdating EspINA's CMake Configuration\033[37m"
cmake ..
echo "\033[32mPackaging EspINA\033[37m"
RES=`make -j8 package | grep .deb` 

RES="CPack: - package: /home/jorge/produccion/espina/build/EspINA-0.99.0-Linux.deb generated."

TAIL=${RES#*package:}
DEB=${TAIL%generated*}
ARQDEB=amd64_${DEB##*/}
#set -- $RES //Alternativa
#echo $4

echo "\033[32mAdding $DEB to repository\033[37m"
scp $DEB bb13:$ARQDEB
ssh bb13 "cd /var/www/packages/ubuntu; reprepro includedeb $DIST ~/$ARQDEB"
