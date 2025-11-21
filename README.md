# xml2

This is a git clone of the xml2 sources at http://dan.egnor.name/xml2/.

The **xml2**- and **csv2**-tools convert xml and csv to a more universal flat-format.

This flat-format is easily manipulated with tools such as sed,awk etc. See the
[doc](./doc)-folder for more information.

The inverse tools **2xml** and **2csv** convert the flat-format back into its
respective form.

## Examples

### CSV to Flat

Cmd:

```bash
csv2 < Input > Output
```

- [Input](./tests/data.csv)
- [Output](.//tests/expected_csv.flat)

The Output file is no in flat-format.

Use the following command to get back:

```bash
2csv file/record field0 field1 field2 < Output > Input.recreated
diff Input Input.recreated
```

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

Make sure to check the [tests](tests)-folder for the test-input.

## Documentation

See the [doc](./doc)-folder for more information.

## New features

Check the git-log to see simple new additions for the tools.

## Original code 

- http://dan.egnor.name/xml2/
  - No longer online


## Simple Makefile

Sometimes the entire autoconf-setup is overkill, compile the tools with
vanilla [Makefile.simple](./Makefile.simple):

To install in $HOME/bin:

```bash
make -f Makefile.simple all
make -f Makefile.simple install PREFIX=$HOME
```

To uninstall from $HOME/bin:

```bash
make -f Makefile.simple uninstall PREFIX=$HOME
```
