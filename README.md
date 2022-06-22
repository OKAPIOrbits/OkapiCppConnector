# OkapiCppConnector
Provides basic routines to init your OKAPI account as well as send requests and receive results.

### What you need to build
* a C++ compiler: e.g. gcc or clang
* CMake: we used Verison 3.2
* a text editor or IDE of your choice
* a terminal
* libraries: boost_system, boost_thread, boost_chrono, crypto, ssl and cpprest
* this was tested with Ubuntu 18.04

### The build process
To build the library, create a `build` directory in your main folder, change into it and run the two commands `cmake ../` and `make install`. This will generate a shared object (.so or .dylib) in the `lib` folder. You can now use the library. The runnable example `okapi-connector-test` is available in the `bin`directory. 

For more information on the API and more examples, visit www.okapiorbits.space/documentation/

NOTE: This repository is currently not maintained. For a more recent version, please contact sven@okapiorbits.com
