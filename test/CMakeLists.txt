cmake_minimum_required(VERSION 2.8.9)
project (test_all)

set ( PROJECT_LINK_LIBS ncmpi_vol.a )
link_directories(${CMAKE_BINARY_DIR})
link_directories(${PNC_DIR}/lib)
link_directories(${H5_DIR}/lib)

include_directories(${CMAKE_SOURCE_DIR}/include)

add_executable(test_all test.c)
target_link_libraries(test_all ${PROJECT_LINK_LIBS} )
target_link_libraries(test_all ${MPI_C_LIBRARIES})