Converts office files for [Overview](https://github.com/overview/overview-server)
using [LibreOffice](https://www.libreoffice.org/).

# Methodology

This program always outputs `0.json` and `0.blob`.

The output `0.json` has `wantOcr:false`.

`soffice.bin` exits with status code 81 if a profile directory is missing --
and it creates the profile directory at the same time. The whole process takes
seconds. And if `soffice.bin` crashes, we deem the profile directory to have
been destroyed. So we recreate the profile directory when it's missing (because
recreating it always is so slow) and we delete it when `soffice.bin` crashes.

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
