#!/usr/bin/env bash
REQUIRED_LIB="g++ libqt4-dev cmake libx11-dev libxxf86vm-dev libxrender-dev libxcomposite-dev libasound2-dev libphonon-dev"
sudo apt-get -y purge qtpanel
DATE=$(date +%Y%m%d%H%M)
ARCH=$(dpkg --print-architecture)
DEBIAN_REVISION=1
echo -n "${DATE}" > date.txt
echo -n "${ARCH}" > architecture.txt
if [ ! -d build ]; then
	mkdir build
fi
cd build
sudo apt-get -y install ${REQUIRED_LIB}
cmake ../
make package
sudo dpkg -i qtpanel_${DATE}-${DEBIAN_REVISION}_${ARCH}.deb
cd ..
rm -R -f build
rm date.txt
rm architecture.txt

