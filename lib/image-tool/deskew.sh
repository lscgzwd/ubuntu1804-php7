#!/bin/bash
./deskew-image $1 $2 --horizontal $(./extract-lines --horizontal $2 | ./skew-offset) --vertical $(./extract-lines --vertical $2 | ./skew-offset)
