#!/bin/sh

# Path to the binary (built in the root directory)
XML2_BIN=./xml2

# Exit immediately if binary is missing
if [ ! -f "$XML2_BIN" ]; then
    echo "Error: $XML2_BIN not found. Did you run 'make'?"
    exit 1
fi

# Run xml2 against input.xml and redirect output to a temporary file
$XML2_BIN < tests/input.xml > tests/output.tmp

# Compare the output with the expected result
if diff tests/expected.txt tests/output.tmp; then
    echo "Test Passed"
    rm tests/output.tmp
    exit 0
else
    echo "Test Failed: Output does not match expected."
    rm tests/output.tmp
    exit 1
fi