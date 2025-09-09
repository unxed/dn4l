#!/bin/bash

find . -maxdepth 1 \( -name "*.cpp" -o -name "*.h" -o -name "*.sh"\) -print0 | while IFS= read -r -d '' file; do
  if [ -f "$file" ]; then
    echo "Processing $file"
    
    sed -i -e 's/[ \t]\+$//' -e 's/^[ \t]\+$//' "$file"
  fi
done
