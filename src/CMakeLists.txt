
file(GLOB SOURCES "*.cpp")
IF(${SHARED})
  ADD_LIBRARY(${PROJECT_NAME} SHARED ${SOURCES})
ELSE()
  ADD_LIBRARY(${PROJECT_NAME} ${SOURCES})
endif()

INSTALL(TARGETS ${PROJECT_NAME})
