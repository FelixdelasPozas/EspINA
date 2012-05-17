#!/bin/bash

# Release Script 1.1
. ~/.espina-release.conf

CURRENT_DIR=`pwd`
echo $CURRENT_DIR

if [ "$QUAZIP_DIR" = "" ]; then
	echo "Quazip build path unspecified. Please, add path info"
	return
fi

if [ "$ITK_DIR" = "" ]; then
	echo "ITK build path unspecified. Please, add path info"
	return
fi

if [ "$PARAVIEW_DIR" = "" ]; then
	echo "Paraview build path unspecified. Please, add path info"
	return
fi

if [ "$ESPINA_DIR" = "" ]; then
	echo "espina path unspecified. Please, add path info"
	return
fi

#if [ "$REPO" = "" ]; then
#	echo "Git repository unspecified. Please, add repository info"
#	return
#fi

if [ "$DIST" = "" ]; then
	echo "Ubuntu Distribution unspecified. Please, add distribution info"
	return
fi


# QUAZIP
echo "\033[32mGenare Quazip package\033[37m"
cd $QUAZIP_DIR
make package
PACKAGE=quazip_1.0_$ARCH.deb
mv quazip*.deb $PACKAGE

echo "\033[32mAdding $PACKAGE to repository\033[37m"
scp $PACKAGE $USER@bb13:
ssh $USER@bb13 "cd /var/www/packages/$DIST; reprepro includedeb $DIST ~/$PACKAGE"
ssh $USER@bb13 "rm ~/$PACKAGE"

# ITK-REVIEW
echo "\033[32mGenare ITK-Review package\033[37m"
cd $ITK_DIR
make
cp $CURRENT_DIR/itk-review.postinst postinst
cpack -G DEB -D CPACK_DEBIAN_PACKAGE_MAINTAINER="CeSViMa" \
-D CPACK_DEBIAN_PACKAGE_NAME="ITK-Review" \
-D CPACK_DEBIAN_PACKAGE_DESCRIPTION="Insight Toolkit with Review Libraries" \
-D CPACK_DEBIAN_PACKAGE_CONTROL_EXTRA="postinst"
PACKAGE=`ls *.deb`

echo "\033[32mAdding $PACKAGE to repository\033[37m"
scp $PACKAGE $USER@bb13:
ssh $USER@bb13 "cd /var/www/packages/$DIST; reprepro includedeb $DIST ~/$PACKAGE"
ssh $USER@bb13 "rm ~/$PACKAGE"

# VTK PARAVIEW
echo "\033[32mGenare VTK-Paraview package\033[37m"
cd $PARAVIEW_DIR
mkdir -p debian-vtk/lib
cp bin/libvtk*so* debian-vtk/lib
cp bin/libQVTK*so* debian-vtk/lib
cp bin/libVPIC*so* debian-vtk/lib
mkdir -p debian-vtk/DEBIAN
cp $CURRENT_DIR/vtk-paraview.control debian-vtk/DEBIAN/control
dpkg -b debian-vtk
PACKAGE=vtk-paraview_5.8_$ARCH.deb
mv debian-vtk.deb $PACKAGE

echo "\033[32mAdding $PACKAGE to repository\033[37m"
scp $PACKAGE $USER@bb13:
ssh $USER@bb13 "cd /var/www/packages/$DIST; reprepro includedeb $DIST ~/$PACKAGE"
ssh $USER@bb13 "rm ~/$PACKAGE"


# PARAVIEW
echo "\033[32mGenare Paraview package\033[37m"
cd $PARAVIEW_DIR
mkdir -p debian-paraview/lib
PARALIBS=`find bin -not -iname lib*vtk* | grep /lib`
cp $PARALIBS debian-paraview/lib
rm debian-paraview/lib/libVPIC*
mkdir -p debian-paraview/DEBIAN
cp $CURRENT_DIR/paraview.control debian-paraview/DEBIAN/control
dpkg -b debian-paraview
PACKAGE=paraview_3.14.1_$ARCH.deb
mv debian-paraview.deb $PACKAGE

echo "\033[32mAdding $PACKAGE to repository\033[37m"
scp $PACKAGE $USER@bb13:
ssh $USER@bb13 "cd /var/www/packages/$DIST; reprepro includedeb $DIST ~/$PACKAGE"
ssh $USER@bb13 "rm ~/$PACKAGE"

# ESPINA
echo "\033[32mGenare EspINA package\033[37m"
cd $ESPINA_DIR
make package
PACKAGE=`ls *.deb`

echo "\033[32mAdding $PACKAGE to repository\033[37m"
scp $PACKAGE $USER@bb13:
ssh $USER@bb13 "cd /var/www/packages/$DIST; reprepro includedeb $DIST ~/$PACKAGE"
ssh $USER@bb13 "rm ~/$PACKAGE"

# ESPINA SEGMHA IMPORTER
echo "\033[32mGenare EspINA Segmha Importer package\033[37m"
cd $ESPINA_SEGMHA_DIR
make package
PACKAGE=`ls *.deb`

echo "\033[32mAdding $PACKAGE to repository\033[37m"
scp $PACKAGE $USER@bb13:
ssh $USER@bb13 "cd /var/www/packages/$DIST; reprepro includedeb $DIST ~/$PACKAGE"
ssh $USER@bb13 "rm ~/$PACKAGE"
