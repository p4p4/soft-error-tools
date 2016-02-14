option(FORENSIC_CPPUNIT_SUITE "should Cppunit run suites instead of single tests?" off)
set(TEST_TIMEOUT 5 CACHE STRING "timeout for testcases")

get_filename_component(_CPPUNIT_TEST_DIR ${CMAKE_CURRENT_LIST_FILE} PATH)

set(_CPPUNIT_GENERATED_TESTS "")

macro(add_cppunit_test target_name)
  add_executable(${target_name} ${ARGN})

  get_target_property(${target_name}_LOCATION ${target_name} LOCATION)

  if(WLF_CPPUNIT_SUITE)
    set(_CPPLIST --list)
  else(WLF_CPPUNIT_SUITE)
    set(_CPPLIST --list-single)
  endif(WLF_CPPUNIT_SUITE)

  add_custom_command(
    OUTPUT 
      "${CMAKE_CURRENT_BINARY_DIR}/${target_name}.ctest"
      "${CMAKE_CURRENT_BINARY_DIR}/${target_name}.list"
    COMMAND ${target_name} ${_CPPLIST}
        > "${CMAKE_CURRENT_BINARY_DIR}/${target_name}.list"
    COMMAND ${CMAKE_COMMAND} 
        -DIN="${CMAKE_CURRENT_BINARY_DIR}/${target_name}.list"
        -DOUT="${CMAKE_CURRENT_BINARY_DIR}/${target_name}.ctest"
        -DWD="${CMAKE_CURRENT_SOURCE_DIR}"
        -DPATH="$ENV{PATH}"
        -DBIN=${${target_name}_LOCATION}
        -DTIMEOUT=${TEST_TIMEOUT}
        -P ${_CPPUNIT_TEST_DIR}/cppunit_create_testfile.cmake 
    DEPENDS ${target_name} ${_CPPUNIT_TEST_DIR}/cppunit_create_testfile.cmake
  )
  list(APPEND _CPPUNIT_GENERATED_TESTS "${target_name}.ctest")
endmacro(add_cppunit_test)

function(cppunit_ctest_file filename)
  list(REMOVE_DUPLICATES _CPPUNIT_GENERATED_TESTS)
  file(WRITE "${filename}_" "")
  foreach(ctest ${_CPPUNIT_GENERATED_TESTS})
    file(APPEND "${filename}_" "include(${ctest})\n")
  endforeach(ctest)

  add_custom_target( test_script ALL 
    COMMAND ${CMAKE_COMMAND} -E copy_if_different "${filename}_" ${filename}
    DEPENDS ${_CPPUNIT_GENERATED_TESTS}
  )
endfunction(cppunit_ctest_file)
