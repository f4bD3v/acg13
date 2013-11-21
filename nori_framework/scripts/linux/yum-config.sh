#!/bin/sh

echo << EOF
Required packages for Fedora:
  * boost-devel
  * ilmbase-devel
  * OpenEXR-devel
  * OpenEXR
  * qt (version >= 4)
  * qt-devel
  * kernel-headers
  * g++
EOF

QT=$(qmake-qt4 --version)
if [ $? -gt 0 ]; then
  echo "You need QT4 to compile .pro files!"
fi

MAKE=$(make --version)
if [ $? -gt 0 ]; then
  echo "You need make to compile using Makefile!"
fi

sudo yum install OpenEXR OpenEXR-devel ilmbase-devel boost-devel
