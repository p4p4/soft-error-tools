## Source for the tests
file(STRINGS extrasources.make test_FILES)

foreach(fil ${test_FILES})
  string(REPLACE " " "" fil ${fil})
  list(APPEND test_SRC ${fil})
endforeach(fil)

include(CppunitTest.cmake)

include_directories("${AIGER_INCLUDE_DIR}")
include_directories("${DEPQBF_INCLUDE_DIR}")
include_directories("${BLOQQER_INCLUDE_DIR}")
include_directories("${LINGELING_INCLUDE_DIR}")
include_directories("${MINISAT_INCLUDE_DIR}")
include_directories("${PICOSAT_INCLUDE_DIR}")
include_directories(${cppunit_INCLUDE_DIRS})

SET_SOURCE_FILES_PROPERTIES(ParallelLearner.cpp PROPERTIES COMPILE_FLAGS "${COMPILE_FLAGS} -std=c++0x")
SET_SOURCE_FILES_PROPERTIES(Options.cpp PROPERTIES COMPILE_FLAGS "${COMPILE_FLAGS} -std=c++0x")
SET_SOURCE_FILES_PROPERTIES(MiniSatApi.cpp PROPERTIES COMPILE_FLAGS "${COMPILE_FLAGS} -Wno-parentheses -D __STDC_LIMIT_MACROS -D __STDC_FORMAT_MACROS")


## build the tester binary
add_cppunit_test(${PROJECT}-tester Main.cpp ${test_SRC})
target_link_libraries(${PROJECT}-tester ${PROJECT})
target_link_libraries(${PROJECT}-tester ${cppunit_LIBRARIES})
target_link_libraries(${PROJECT}-tester ${DEPQBF_LIBRARIES})
target_link_libraries(${PROJECT}-tester ${BLOQQER_LIBRARIES})
target_link_libraries(${PROJECT}-tester ${LINGELING_LIBRARIES})
target_link_libraries(${PROJECT}-tester ${MINISAT_LIBRARIES})
target_link_libraries(${PROJECT}-tester ${PICOSAT_LIBRARIES})
set_target_properties(${PROJECT}-tester PROPERTIES COMPILE_FLAGS "${COMPILE_FLAGS} -fPIC")

## add_test(UnitTest ${EXECUTABLE_OUTPUT_PATH}/${PROJECT}-tester)

cppunit_ctest_file( "${CMAKE_CURRENT_BINARY_DIR}/CTestTestfile.cmake" )

# create a symlink to the data directory so that tests 
# can execute in build folder
execute_process(COMMAND ${CMAKE_COMMAND} -E create_symlink ${CMAKE_CURRENT_SOURCE_DIR}/data ${CMAKE_CURRENT_BINARY_DIR}/data )

# vim: ts=2 sw=2 et
