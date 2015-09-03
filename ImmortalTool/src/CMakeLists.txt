if(APPLE)
  add_definitions(-DSYSTEMTYPE="Darwin")
elseif(UNIX)
  add_definitions(-DSYSTEMTYPE="Linux")
else(APPLE)
  add_definitions(-DSYSTEMTYPE="unknown")
endif(APPLE)

file(STRINGS extrasources.make project_FILES)


foreach(fil ${project_FILES})
  string(REPLACE " " "" fil ${fil})
  if(NOT ${fil} MATCHES main.cpp)
   list(APPEND project_SRC ${fil})
  endif(NOT ${fil} MATCHES main.cpp)
endforeach(fil)

include_directories("${AIGER_INCLUDE_DIR}")
#include_directories("${DEPQBF_INCLUDE_DIR}")
#include_directories("${RAREQS_INCLUDE_DIR}")
#include_directories("${BLOQQER_INCLUDE_DIR}")
include_directories("${LINGELING_INCLUDE_DIR}")
include_directories("${MINISAT_INCLUDE_DIR}")
include_directories("${PICOSAT_INCLUDE_DIR}")

# set the default build type to 'debug'.
SET(CMAKE_BUILD_TYPE Debug CACHE STRING "default to debug" FORCE)


SET_SOURCE_FILES_PROPERTIES(MiniSatApi.cpp PROPERTIES COMPILE_FLAGS "${COMPILE_FLAGS} -Wno-parentheses -D __STDC_LIMIT_MACROS -D __STDC_FORMAT_MACROS")

add_library(${PROJECT}
##       SHARED
       STATIC
        ${project_SRC})
#target_link_libraries(${PROJECT} ${DEPQBF_LIBRARIES})
#target_link_libraries(${PROJECT} ${RAREQS_LIBRARIES})
#target_link_libraries(${PROJECT} ${BLOQQER_LIBRARIES})
target_link_libraries(${PROJECT} ${LINGELING_LIBRARIES})
target_link_libraries(${PROJECT} ${MINISAT_LIBRARIES})
target_link_libraries(${PROJECT} ${PICOSAT_LIBRARIES})
set_target_properties(${PROJECT} PROPERTIES COMPILE_FLAGS "${COMPILE_FLAGS} -fPIC -DNDEBUG -O3")

add_executable(${PROJECT}-bin main)
target_link_libraries(${PROJECT}-bin ${AIGER_LIBRARIES})
#target_link_libraries(${PROJECT}-bin ${DEPQBF_LIBRARIES})
#target_link_libraries(${PROJECT}-bin ${RAREQS_LIBRARIES})
#target_link_libraries(${PROJECT}-bin ${BLOQQER_LIBRARIES})
target_link_libraries(${PROJECT}-bin ${LINGELING_LIBRARIES})
target_link_libraries(${PROJECT}-bin ${MINISAT_LIBRARIES})
target_link_libraries(${PROJECT}-bin ${PICOSAT_LIBRARIES})
target_link_libraries(${PROJECT}-bin ${PROJECT})
set_target_properties(${PROJECT}-bin PROPERTIES COMPILE_FLAGS "${COMPILE_FLAGS} -fPIC -DNDEBUG")


## DEBUG currently not used
#add_executable(${PROJECT}-debug ${project_SRC} main)
#target_link_libraries(${PROJECT}-debug ${AIGER_LIBRARIES})
#target_link_libraries(${PROJECT}-debug ${DEPQBF_LIBRARIES})
#target_link_libraries(${PROJECT}-debug ${RAREQS_LIBRARIES})
#target_link_libraries(${PROJECT}-debug ${BLOQQER_LIBRARIES})
#target_link_libraries(${PROJECT}-debug ${LINGELING_LIBRARIES})
#target_link_libraries(${PROJECT}-debug ${MINISAT_LIBRARIES})
#target_link_libraries(${PROJECT}-debug ${PICOSAT_LIBRARIES})
#target_link_libraries(${PROJECT}-debug ${PROJECT})
#set_target_properties(${PROJECT}-debug PROPERTIES COMPILE_FLAGS "${COMPILE_FLAGS} -fPIC")

