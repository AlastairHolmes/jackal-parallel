# Create a library
AddLibrary(JKParallel STATIC)
AddTargetToGroup(JKParallel JKParallel)

install(TARGETS JKParallel EXPORT JKParallel
	LIBRARY DESTINATION lib
	ARCHIVE DESTINATION lib
	RUNTIME DESTINATION bin
	INCLUDES DESTINATION include)
	
find_package(JKUtil REQUIRED)

target_link_libraries(JKParallel PUBLIC JKUtil::JKUtil)
	
AddIncludeDirectoryToTarget(JKParallel "${CMAKE_CURRENT_SOURCE_DIR}/.." INSTALL_INTERFACE "include")

target_compile_features(JKParallel INTERFACE
	cxx_std_17)

# Header Files

	AddSourcesToTarget(JKParallel
		GROUP "Header Files\\Parallel"
		SCOPE PRIVATE
		INSTALL_DESTINATION "include/jkparallel"
		FILES
			"${CMAKE_CURRENT_LIST_DIR}/queue.h"
			"${CMAKE_CURRENT_LIST_DIR}/sources.h"
			"${CMAKE_CURRENT_LIST_DIR}/sinks.h"
			"${CMAKE_CURRENT_LIST_DIR}/queues.h"
			"${CMAKE_CURRENT_LIST_DIR}/blocking_queue.h"
			"${CMAKE_CURRENT_LIST_DIR}/async.h"
			"${CMAKE_CURRENT_LIST_DIR}/spinlock.h"
			"${CMAKE_CURRENT_LIST_DIR}/spmc_queue.h"
			"${CMAKE_CURRENT_LIST_DIR}/mpsc_queue.h"
			"${CMAKE_CURRENT_LIST_DIR}/parallel_allocator.h"
			"${CMAKE_CURRENT_LIST_DIR}/parallel_cache_allocator.h")
			
# Source Files

	AddSourcesToTarget(JKParallel
		GROUP "Source Files\\Parallel"
		SCOPE PRIVATE
		FILES
			"${CMAKE_CURRENT_LIST_DIR}/parallel_cache_allocator.cpp"
			"${CMAKE_CURRENT_LIST_DIR}/spinlock.cpp"
			"${CMAKE_CURRENT_LIST_DIR}/parallel_allocator.cpp")