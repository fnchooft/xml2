#!/bin/sh

CSV2_BIN=./csv2
TOCSV_BIN=./2csv
INPUT=tests/data.csv
EXPECTED_FLAT=tests/expected_csv.flat
TEMP_FLAT=tests/output_csv.flat
TEMP_CSV=tests/output_roundtrip.csv

# ./2csv file/record field0 field1 field2  < tests/expected_csv.flat  

FAILURES=0

# Helper function for cleaner code
run_test() {
    local NAME="$1"
    local CMD="$2"
    local EXPECTED="$3"
    local ACTUAL="$4"

    # Run the command string
    eval "$CMD"

    echo "$CMD"

    # Compare results
    if diff -q "$EXPECTED" "$ACTUAL" > /dev/null; then
        echo "[PASS] $NAME"
        rm "$ACTUAL"
    else
        echo "[FAIL] $NAME"
        echo "       Diff output:"
        diff "$EXPECTED" "$ACTUAL"
        FAILURES=$((FAILURES + 1))
    fi
}

# Check binaries
if [ ! -f "$CSV2_BIN" ] || [ ! -f "$TOCSV_BIN" ]; then
    echo "Error: CSV binaries not found."
    exit 1
fi

echo "Starting CSV Tests..."

# Test 1: csv2 (CSV -> Flat)
run_test "csv2 Conversion" \
         "$CSV2_BIN < $INPUT > $TEMP_FLAT" \
         "$EXPECTED_FLAT" \
         "$TEMP_FLAT"

# Test 2: 2csv (Flat -> CSV)
# Note: We use the expected flat file as input to test 2csv independently
run_test "2csv Conversion" \
         "$TOCSV_BIN file/record field0 field1 field2 < $EXPECTED_FLAT > $TEMP_CSV" \
         "$INPUT" \
         "$TEMP_CSV"

# Test 3: Round Trip (CSV -> csv2 -> 2csv -> CSV)
run_test "CSV Round Trip" \
         "$CSV2_BIN < $INPUT | $TOCSV_BIN file/record field0 field1 field2 > $TEMP_CSV" \
         "$INPUT" \
         "$TEMP_CSV"

# Final Status
if [ "$FAILURES" -gt 0 ]; then
    echo "CSV Tests Failed: $FAILURES errors."
    exit 1
else
    echo "All CSV Tests Passed."
    exit 0
fi