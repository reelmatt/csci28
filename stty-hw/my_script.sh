#/bin/sh -v
#
# My test script to test my submission for sttyl.
# It makes the program, performs a few sample tests
# of my own, and runs the lib215-provided test script.
#

#-------------------------------------
#    compile program
#-------------------------------------
make clean

make sttyl


#-------------------------------------
#    my negative tests
#-------------------------------------

# lkj
./sttyl foobar




# demo pathname out of order
./pfind -name foo .

find -name foo .

# invalid type option
./pfind -type q

find -type q

# pathname cannot start with '-', treated as "-option"
./pfind -foopath

find -foopath

#-------------------------------------
#    remove tmp files
#-------------------------------------
rm my.output find.output

#-------------------------------------
#    check memory usage
#-------------------------------------
valgrind --tool=memcheck --leak-check=yes ./pfind .

#-------------------------------------
#    run the course test-script
#-------------------------------------
~lib215/hw/pfind/test.pfind
