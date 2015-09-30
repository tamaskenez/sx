link_libraries(sx)

foreach(t abbrev algorithm array_par array_view)
	add_executable(test-${t} ${t}.cpp)
	add_test(${t} test-${t})
endforeach()
