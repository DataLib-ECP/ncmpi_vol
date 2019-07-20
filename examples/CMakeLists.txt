cmake_minimum_required(VERSION 2.8.9)
project (test_all)

include_directories(include)
include_directories(${H5_DIR}/include)
include_directories(${PNC_DIR}/include)
include_directories(SYSTEM ${MPI_INCLUDE_PATH})
include_directories(${CMAKE_SOURCE_DIR}/include)

link_directories(${CMAKE_BINARY_DIR})
link_directories(${H5_DIR}/lib)
link_directories(${PNC_DIR}/lib)

set ( PROJECT_LINK_LIBS ncmpi_vol hdf5 hdf5_hl pnetcdf zlib)

add_executable(create_open create_open.cpp)
add_executable(multiselection multiselection.cpp)

target_link_libraries(create_open ${PROJECT_LINK_LIBS} ${MPI_C_LIBRARIES} ${MPI_CXX_LIBRARIES})
target_link_libraries(multiselection ${PROJECT_LINK_LIBS} ${MPI_C_LIBRARIES} ${MPI_CXX_LIBRARIES})