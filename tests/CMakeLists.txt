include(FetchContent)
FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG v1.14.0
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
TARGET_LINK_LIBRARIES(test_all gtest gtest_main)
