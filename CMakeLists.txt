cmake_minimum_required(VERSION 3.4.1)

project(xsolarbridge)

add_executable(
    xsolarbridge
    src/CMqttBridge.cpp
    src/main.cpp
)

set(BUILD_SHARED_LIBS OFF)
add_subdirectory(poco)
include_directories(
   include
   poco/ApacheConnector/include
   poco/CppParser/include
   poco/CppUnit/include
   poco/Crypto/include
   poco/Data/include
   poco/Encodings/include
   poco/Foundation/include
   poco/JSON/include
   poco/MongoDB/include
   poco/Net/include
   poco/NetSSL_OpenSSL/include
   poco/NetSSL_Win/include
   poco/openssl/build/include
   poco/PDF/include
   poco/Redis/include
   poco/SevenZip/include
   poco/Util/include
   poco/XML/include
   poco/Zip/include
)

target_link_libraries(xsolarbridge PUBLIC Poco::Util Poco::JSON Poco::XML mosquitto)