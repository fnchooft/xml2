# xml2

This is a git clone of the xml2 sources at http://dan.egnor.name/xml2/

The xml2- and csv2-tools convert xml and csv to a more universal flat format.

This flat-format is easily manipulated with tools such as sed,awk etc. See the 
[doc]-folder for more information.

The inverse tools 2xml and 2csv convert the flat-format back into its respective
forms.

## Build

```bash
autoreconf -i
./configure --prefix $PWD/install
make all
```

## Tests

Make sure you built the code, see above.

```bash
make check
```

Make sure to check the [tests]-folder for the test-input.

## Documentation

See the [doc]-folder for more information.

## New features

Check the git-log to see simple new additions for the tools.

## Original code 

- http://dan.egnor.name/xml2/
  - No longer online
