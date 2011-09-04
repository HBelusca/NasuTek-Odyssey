#!/bin/bash
if [ "x$ROS_ARCH" == "x" ]
then
  echo Could not detect RosBE.
  exit 1
fi

BUILD_ENVIRONMENT=MinGW
ARCH=$ROS_ARCH
ODYSSEY_SOURCE_DIR=$(cd `dirname $0` && pwd)
ODYSSEY_OUTPUT_PATH=output-$BUILD_ENVIRONMENT-$ARCH

if [ "$ODYSSEY_SOURCE_DIR" == "$PWD" ]
then
  echo Creating directories in $ODYSSEY_OUTPUT_PATH
  mkdir -p $ODYSSEY_OUTPUT_PATH
  cd "$ODYSSEY_OUTPUT_PATH"
fi

mkdir -p host-tools
mkdir -p odyssey

echo Preparing host tools...
cd host-tools
if [ -f CMakeCache.txt ]
then
  rm -f CMakeCache.txt
fi

ODYSSEY_BUILD_TOOLS_DIR="$PWD"
cmake -G "Unix Makefiles" -DARCH=$ARCH "$ODYSSEY_SOURCE_DIR"

echo Preparing odyssey...
cd ../odyssey
if [ -f CMakeCache.txt ]
then
  rm -f CMakeCache.txt
fi

cmake -G "Unix Makefiles" -DENABLE_CCACHE=0 -DPCH=0 -DCMAKE_TOOLCHAIN_FILE=toolchain-gcc.cmake -DARCH=$ARCH -DODYSSEY_BUILD_TOOLS_DIR="$ODYSSEY_BUILD_TOOLS_DIR" "$ODYSSEY_SOURCE_DIR"

echo Configure script complete! Enter directories and execute appropriate build commands\(ex: make, makex, etc...\).

