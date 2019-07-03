cmake_minimum_required(VERSION 2.8.9)
project(ncmpi_vol)

find_package(MPI)


include_directories(include)
include_directories(${H5_DIR}/include)
include_directories(${PNC_DIR}/include)
include_directories(SYSTEM ${MPI_INCLUDE_PATH})

LINK_DIRECTORIES(${H5_DIR}/lib)
LINK_DIRECTORIES(${PNC_DIR}/lib)

file(GLOB SOURCES "src/*.cpp")

add_library(ncmpi_vol STATIC ${SOURCES})

install(TARGETS ncmpi_vol DESTINATION lib)
install(FILES include/ncmpi_vol.h DESTINATION include)

add_subdirectory(test)
add_subdirectory(examples)
