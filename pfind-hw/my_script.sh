#/bin/sh -v
#
# My test script to test my submission for pfind.
# It makes the program, performs a few sample tests
# of my own, runs the lib215-provided test script
# and saves the typescript file.
#

# compile program
make clean
make pfind

# my sample tests

#demo pathname out of order
./pfind -name foo .
find -name foo .

#prints the starting path
./pfind . -name .
find . -name .

#creates a subdirectory, and searches ".."
mkdir burrow
cd burrow
../pfind ..
find ..
cd ..
rm -r burrow

#run the course test-script
~lib215/hw/pfind/test.pfind
