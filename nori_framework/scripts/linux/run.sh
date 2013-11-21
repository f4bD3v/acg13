#!/bin/sh

# this absolute directory
SCRIPT_PATH=$( cd $(dirname $0); pwd -P)
# path to Nori binary
NORI=$SCRIPT_PATH/bin-debug/nori
# path to libraries
LIBS=$SCRIPT_PATH/../lib

if [ ! -d $LIBS ]; then
  echo "It seems that lib/ is not available!"
fi

echo "Script in: $SCRIPT_PATH"
echo "Nori as: $NORI"
echo "Libraries in: $LIBS"

# build if needed
. $SCRIPT_PATH/build.sh

# parameters
SCENE="$1"
echo "Scene: $1"
SCENE_DIR=`dirname "$SCENE"`
SCENE_NAME=`basename "$SCENE"`
ARGS="$SCENE_NAME ${@:2}"

# call nori
(cd $SCENE_DIR; LD_LIBRARY_PATH=$LIBS $NORI $ARGS)
