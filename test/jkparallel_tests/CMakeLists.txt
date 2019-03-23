find_package(Catch2 REQUIRED)

AddTestTarget(JKParallelTests)

target_link_libraries(JKParallelTests PUBLIC Catch2::Catch2)
target_link_libraries(JKParallelTests PUBLIC JKParallel)

AddSourcesToTarget(JKParallelTests
	GROUP "Source Files/Tests"
	SCOPE PRIVATE
	FILES
		"${CMAKE_CURRENT_LIST_DIR}/main.cpp"
)