#!/bin/bash

# A script to check for and prepend a license header to all .cpp and .h files
# in the current directory.

# --- Configuration ---
# The name of the file containing the license text.
LICENSE_FILE="LICENSE"

# --- Script Logic ---

# 1. Check if the license file exists.
# If not, print an error to stderr and exit.
if [ ! -f "$LICENSE_FILE" ]; then
    echo "Error: License file '$LICENSE_FILE' not found in the current directory." >&2
    exit 1
fi

# 2. Prepare the header text.
# Read the license file and format it as a C/C++ block comment.
# 'printf' is used for safe and reliable string formatting.
# We add two newlines at the end for clean spacing in the source files.
HEADER_TEXT="$(cat "$LICENSE_FILE")"$'\n\n'

# 3. Calculate the exact byte size of the header for comparison.
# 'echo -n' is used to avoid counting a trailing newline character.
HEADER_BYTES=$(echo -n "$HEADER_TEXT" | wc -c)

# 4. Find all .cpp and .h files in the current directory only (-maxdepth 1).
# The 'while read' loop is a safe way to handle filenames with spaces.
find . -maxdepth 1 -type f \( -name "*.cpp" -o -name "*.h" \) | while IFS= read -r file; do
    
    # Extract the head of the file, equal in size to our license header.
    FILE_HEAD=$(head -c "$HEADER_BYTES" "$file")

    # 5. Compare the file's beginning with the prepared header text.
    if [ "$FILE_HEAD" = "$HEADER_TEXT" ]; then
        echo "Header already exists in: $file"
    else
        echo "Adding header to: $file"
        
        # Create a secure temporary file to build the new content.
        TMP_FILE=$(mktemp)
        
        # Write the header and the original file content to the temp file.
        # Using a subshell (...) with grouped redirection is efficient and safe.
        (printf "%s" "$HEADER_TEXT"; cat "$file") > "$TMP_FILE"
        
        # Replace the original file with the updated one.
        mv "$TMP_FILE" "$file"
    fi
done

echo "Script finished."
