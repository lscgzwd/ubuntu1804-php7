#!/bin/bash
make clean && make -j4 DEBUG=1 && make install
rm images/augment-*
php t.php
eog images/augment-* &
