Converts office files for [Overview](https://github.com/overview/overview-server)
using [LibreOffice](https://www.libreoffice.org/).

# Methodology

This program always outputs `0.json` and `0.blob`.

The output `0.json` has `wantOcr:false`.

These metadata fields from the input document will be written to the
output PDF:

* `Title`
* `Author`
* `Subject`
* `Keywords`
* `Creation Date`
* `Modification Date`

We use custom C++ with
[LibreOfficeKit](https://docs.libreoffice.org/libreofficekit.html) and
[QPDF](http://qpdf.sourceforge.net/). This is painful and janky. Here's why:

* `soffice.bin` is slow to load, imposing big overhead (hence LOK).
* `soffice.bin` does not preserve metadata when converting to PDF (hence QPDF).
* `soffice.bin` cannot even _extract_ metadata. The only move is to convert to
  `.odt` and then read the result as a zipfile -- costing a second invocation
  of `soffice.bin` just to read metadata (hence _not_ using `soffice.bin`).

We create the LibreOffice profile directory _once_ and reset it with every
conversion. This is for security: once an invocation is complete, all its data
is wiped.

# Testing

Write to `test/test-*`. `docker build .` will run the tests.

Each test has `input.blob` (which means the same as in production) and
`input.json` (whose contents are `$1` in `do-convert-single-file`). The files
`stdout`, `0.json` and `0.blob` in the test directory are expected values. If
actual values differ from expected values, the test fails.

PDF is a tricky format to get exactly right. You may need to use the Docker
image itself to generate expected output files. For instance, here is how we
build `test-odt/0.blob`:

1. Wrote `test/test-odt/{input.json,input.blob,0.json,stdout}`
1. Ran `docker build .`. The end of the output looked like this:
    Step 12/13 : RUN [ "/app/test-convert-single-file" ]
     ---> Running in f65521f3a30c
    1..1
    not ok 1 - test-odt
TODO        do-convert-single-file wrote /tmp/test-do-convert-single-file912093989/0-thumbnail.jpg, but we expected it not to exist
TODO    ...
TODO1. `docker cp f65521f3a30c:/tmp/test-do-convert-single-file912093989/0-thumbnail.jpg test/test-jpg-ocr/`
TODO1. `docker rm -f f65521f3a30c`
