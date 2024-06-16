include(FetchContent)

FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG v1.14.0
  EXCLUDE_FROM_ALL
)
FetchContent_MakeAvailable(googletest)

add_executable(test_all
  test_mac.cpp
  test_ipv4.cpp
  test_cidr.cpp
  ../src/mac.cpp
  ../src/ipv4.cpp
  ../src/subnetmask.cpp
)
target_link_libraries(test_all PRIVATE gtest gtest_main)

file(GLOB SOURCES "../src/*.cpp")
add_executable(test_arp test_arp.cpp ${SOURCES})
target_link_libraries(test_arp PRIVATE gtest gtest_main)
