set(Casablanca_LIBRARIES "-lboost_system -lboost_thread -lboost_chrono -lcrypto -lssl -lcpprest")

# Build the neptune executable based on the given source files
add_library(cpp-connector SHARED okapiConnector.cpp)

# link all the libraries needed
target_link_libraries(tutorial ${Casablanca_LIBRARIES})

# Install the libraries into the lib directory and executrables to bin
install(LIBRARY DESTINATION lib)
