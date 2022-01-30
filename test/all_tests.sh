#! /bin/bash

chmod +x *.bob

# execute all test functions and catch the out put of each test in a file
for i in test_*.bob
do
    zz=$( basename "$i" )

    (
        echo "$i"
        ../bin/bob2 ./$i
    ) |
    tee $zz.txt
done
