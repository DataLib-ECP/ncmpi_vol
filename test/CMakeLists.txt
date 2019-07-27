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

add_executable(test_allvol allvol.cpp)
add_executable(test_multiblockselection multiblockselection.cpp)
add_executable(test_multipointselection multipointselection.cpp)

target_link_libraries(test_allvol ${PROJECT_LINK_LIBS} ${MPI_C_LIBRARIES} ${MPI_CXX_LIBRARIES})
target_link_libraries(test_multiblockselection ${PROJECT_LINK_LIBS} ${MPI_C_LIBRARIES} ${MPI_CXX_LIBRARIES})
target_link_libraries(test_multipointselection ${PROJECT_LINK_LIBS} ${MPI_C_LIBRARIES} ${MPI_CXX_LIBRARIES})

ADD_TEST(allvol test_allvol)
ADD_TEST(multiblockselection test_multiblockselection)
ADD_TEST(multipointselection test_multipointselection)