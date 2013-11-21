#!/bin/sh

# Script to extract required boost headers
OUTPUT_DIR="$1"
BOOST_PATH=/usr/include

# what is the output?
if [ $# -lt 1 ]; then
  echo "Where shall I output the files?"
  echo "Usage: ./extract_boost.sh \$output_directory"
  exit 1
fi

# does the output directory exist?
if [ ! -d $OUTPUT_DIR ]; then
  echo "The output directory does not exist!"
  echo "I don't work under these conditions..."
  echo "Aborting."
  exit 1
fi

# maybe use scan and a recursive call on sources instead?
bcp --boost=$BOOST_PATH \
  boost/bind.hpp \
  boost/function.hpp \
  boost/math/distributions/chi_squared.hpp \
  boost/math/distributions/students_t.hpp \
  boost/math/special_functions/fpclassify.hpp \
  boost/scoped_ptr.hpp \
  boost/static_assert.hpp \
  boost/tuple/tuple.hpp \
  boost/unordered_map.hpp \
  boost/variant.hpp \
  $OUTPUT_DIR
