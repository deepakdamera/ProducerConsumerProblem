#!/bin/bash

echo $(echo $(time ./mainSpinLock_w_print 200 100 100 1000000) | sed 's/[^0-9]//g');
