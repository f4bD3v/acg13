#! /bin/sh

YUM=$(yum --version)
if [ $? -eq 0 ]; then
  # yum-based
  . ./linux/yum-config.sh
fi

# basic build environment
SCRIPT_DIR=$(cd $(dirname $0); pwd -P)
BUILD_DIR="$SCRIPT_DIR"/../build
mkdir -p $BUILD_DIR

echo "Created build directory: $BUILD_DIR"

# build file
BUILD_FILE=$SCRIPT_DIR/linux/build.sh
ERROR=0
if [ ! -e "$BUILD_FILE" ]; then
  echo "Seems that the build file is not available!"
  ERROR=1
else
  cp -n "$BUILD_FILE" "$BUILD_DIR"
  echo "Build script: $BUILD_DIR/build.sh"
fi

# run file
RUN_FILE=$SCRIPT_DIR/linux/run.sh
if [ ! -e "$RUN_FILE" ]; then
  echo "Seems that the run file is not available!"
  ERROR=1
else
  cp -n "$RUN_FILE" "$BUILD_DIR"
  echo "Run script: $BUILD_DIR/run.sh"
fi

if [ $ERROR -eq 0 ]; then
  echo << EOF
To run a scene:
  * go into the build directory
  * build nori or simply run a scene (automatically build if needed)

To run nori on a scene:

  cd path_to_build_dir
  ./run.sh ../path_to_scene/myscene.xml

That's it!
EOF
else
  echo "There seems to be a trouble with the scripts."
fi
