cmake_minimum_required(VERSION 3.14)

project(tgbot)

set(CMAKE_CXX_STANDARD             20)
set(CMAKE_MODULE_PATH              "${CMAKE_SOURCE_DIR}/tools/cmake")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}")
set(CMAKE_EXPORT_COMPILE_COMMANDS  ON)

set(Clang_DIR "/usr/lib/llvm-16/lib/cmake/clang")
set(LLVM_DIR "/usr/lib/llvm-16/lib/cmake/llvm")
find_package(Clang CONFIG)

find_package(Poco QUIET COMPONENTS Foundation Net JSON)
if (NOT Poco_FOUND)
  message(STATUS "Seems like POCO is not installed on your machine.")
  message(STATUS "Tasks that depend on POCO are disabled.")
endif()

if (APPLE)
  link_directories(/usr/local/lib)
  link_directories(/usr/local/opt/openssl/lib)

  include_directories(/usr/local/include)
  include_directories(/usr/local/opt/openssl/include)
endif()

file(GLOB SOLUTION_SRC CONFIGURE_DEPENDS "telegram/*.cpp")

add_library(telegram ${SOLUTION_SRC})
target_link_libraries(telegram PocoNet PocoNetSSL PocoFoundation PocoJSON PocoUtil)

target_include_directories(telegram PUBLIC .)


add_executable(bot-run telegram/main.cpp)
target_link_libraries(bot-run PRIVATE telegram)
