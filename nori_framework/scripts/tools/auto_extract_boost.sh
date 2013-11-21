#!/bin/sh

# Script to extract required boost headers
ROOT_DIR="$1"
OUTPUT_DIR="$2"
BOOST_PATH=/usr/include

# what is the output?
if [ $# -lt 2 ]; then
  echo "Where shall I output the files?"
  echo "Usage: ./extract_boost_auto.sh \$base_directory \$output_directory"
  exit 1
fi

if [ ! -d $ROOT_DIR ]; then
  echo "The root is not a directory!"
  echo "Aborting."
  exit 2
fi

# does the output directory exist?
if [ ! -d "$OUTPUT_DIR" ]; then
  echo "The output directory does not exist!"
  echo "I don't work under these conditions..."
  echo "Aborting."
  exit 1
fi

# what are the files of interest?
files_src=$(find "$ROOT_DIR"/src -type f)
files_inc=$(find "$ROOT_DIR"/include/nori -type f)
files="$files_src $files_inc"

# maybe use scan and a recursive call on sources instead?
set -x
bcp --scan --boost=$BOOST_PATH $files "$OUTPUT_DIR"
