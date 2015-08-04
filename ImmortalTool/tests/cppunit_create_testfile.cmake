file(READ "${IN}" listfile NEWLINE_CONSUME)

string(REGEX REPLACE "[ \n]+" ";" TESTS ${listfile})

foreach( test ${TESTS} )
  set (test ${test})
  list(APPEND _all_test " ${test}")
  list(APPEND testfile "
add_test( ${test} ${BIN} ${test} )
")
endforeach( test )
list(APPEND testfile "
set_tests_properties( 
   ${_all_test} 
 PROPERTIES ENVIRONMENT 
 \"PATH=${PATH}\"
)
")

file(WRITE "${OUT}" ${testfile})

#vim: ts=2 sw=2 et
