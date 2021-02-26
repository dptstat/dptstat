# dptstat
 This program uses the 4chan API to gather all posts in current /dpt/ threads, and performs analytics on them using for loops and some very crude pattern matching.

 The program will only compile on windows, because the http_toolbox.hpp header is not yet linux compatible. 
 
 There are no makefiles or build scripts, since I'm letting trusty codeblocks handle the build process for me. 
 
 If you want to build this code, you need to link to wininet, and you need the boost 1.75 headers (Boost.JSON).
