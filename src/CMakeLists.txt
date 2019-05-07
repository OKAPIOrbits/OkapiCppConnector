set(Casablanca_LIBRARIES "-lboost_system -lboost_thread -lboost_chrono -lcrypto -lssl -lcpprest")

# Build the neptune executable based on the given source files
add_library(okapi-connector SHARED okapiConnector.cpp)

# link all the libraries needed
target_link_libraries(okapi-connector ${Casablanca_LIBRARIES})

# Install the libraries into the lib directory and executrables to bin
install(TARGETS okapi-connector
	LIBRARY DESTINATION lib)
