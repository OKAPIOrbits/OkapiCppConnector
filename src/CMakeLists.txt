set(CMAKE_CXX_FLAGS "-std=c++14 -ggdb")

if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
	find_package(cpprestsdk CONFIG) # populates CPP_REST_INCLUDE_DIR and CPP_REST_LIBRARY
	set (CPP_REST_INCLUDE_DIR /usr/local/opt/cpprestsdk/include)
	#MESSAGE ("CPP REST DIR: ${CPP_REST_INCLUDE_DIR}")
	include_directories(SYSTEM ${CPP_REST_INCLUDE_DIR})
	include_directories(SYSTEM /usr/local/include)
endif()

if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
	set(Casablanca_LIBRARIES "-L/usr/local/lib -lboost_system -lboost_thread-mt -lboost_chrono -lcrypto -L/usr/local/opt/openssl/lib -lssl -lcpprest")
else()
	set(Casablanca_LIBRARIES "-lboost_system -lboost_thread -lboost_chrono -lcrypto -lssl -lcpprest")
endif()

# Build the neptune executable based on the given source files
add_library(okapi-connector SHARED OkapiConnector.cpp)

# link all the libraries needed
target_link_libraries(okapi-connector ${Casablanca_LIBRARIES})

add_executable(okapi-connector-test OkapiConnectorTest.cpp)
target_link_libraries(okapi-connector-test okapi-connector)

# Install the libraries into the lib directory and executrables to bin
install(TARGETS okapi-connector okapi-connector-test
	LIBRARY DESTINATION lib
	RUNTIME DESTINATION bin)
