#!/bin/bash -e
echo "Compiling"
gcc vm.c -o vm
echo "Running vm"
./vm addresses.txt > out.txt    # change
echo "Comparing with correct.txt"
diff out.txt correct.txt
echo "Compare Complete"
