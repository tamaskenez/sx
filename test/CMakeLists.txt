link_libraries(sx)

foreach(t abbrev)
	add_executable(test-${t} ${t}.cpp)
	add_test(${t} test-${t})
endforeach()
