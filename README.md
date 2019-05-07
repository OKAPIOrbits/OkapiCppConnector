# OkapiCppConnector
Provides basic routines to init your OKAPI account as well as send requests and receive results.

### What you need to build
* a C++ compiler: e.g. gcc
* cMake: we used Verison 3.2
* a text editor or IDE of your choice
* a terminal
* libraries: boost_system, boost_thread, boost_chrono, crypto, ssl and cpprest
* this was tested with Ubuntu 18.04

### The build process
To build the library, create a `<build>` directory in your main folder, change into it and run the two commands `cmake ../` and `make install`. This will generate a shared object (.so) in a `<lib>` folder. You can now use the library. 

For more information on how to integrate the code into an example program, visit www.okapiorbits.space/documentation
