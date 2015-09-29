link_libraries(sx)

foreach(t abbrev algorithm array_par)
	add_executable(test-${t} ${t}.cpp)
	add_test(${t} test-${t})
endforeach()
