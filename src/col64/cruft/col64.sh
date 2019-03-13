dasm col64.dasm -f3 -otest -llist
cp test col64.xex
axe -w test test.atr
cat test hello.xex > test2
axe -w test2 test.atr
atari800 -atari test.atr
