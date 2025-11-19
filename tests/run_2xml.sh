#!/bin/sh

# Path to the binary (built in the root directory)
XML_BIN=./2xml
FAILURES=0

# Exit immediately if binary is missing
if [ ! -f "$XML_BIN" ]; then
    echo "Error: $XML_BIN not found. Did you run 'make'?"
    exit 1
fi

# Run xml2 against input.xml and redirect output to a temporary file
$XML_BIN < tests/expected.txt > tests/output.xml.normal
$XML_BIN -f < tests/expected.txt > tests/output.xml.formatted


# Compare the output with the expected result
if diff tests/expected-normal.xml tests/output.xml.normal; then
    echo "Test Passed (normal)"
    rm tests/output.xml.normal
else
    echo "Test Failed: 2xml normal did NOT match expected."
    FAILURES=$((FAILURES + 1))
fi

# Compare the output with the expected result
if diff tests/expected-format.xml tests/output.xml.formatted; then
    echo "Test Passed (formatted)"
    rm tests/output.xml.formatted
else
    echo "Test Failed: 2xml formatted did NOT match expected."
    FAILURES=$((FAILURES + 1))
fi

rm tests/output.xml.*

if [ "$FAILURES" -gt 0 ]; then
    echo "FAILED: $FAILURES test(s) did not match."
    exit 1
fi

echo "SUCCESS: All tests passed."
exit 0