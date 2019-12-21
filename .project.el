(c++-mode . ((filetypes . (cpp-project))
             (includes . ("include"
                          "winlib"
                          "/usr/include/postgresql"))
             (defines . ("_FILE_OFFSET_BITS=64"
                         "__LINUX__"))
             (flags . ("-xc++"
                       "-std=c++11"))
             (sources . ("src"))
             (buildcmd . "make -f makefile")))
