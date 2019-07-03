# Summary

This is a prototype implementation of HDF5 VOL that use PnetCDF for underlaying I/O operation.
It enable applications to access NetCDF rormated file using HDF5 API.

## How to build
* This library use cmake to mamage to handle build process
* Steps
  + Create build directory
  + Run cmake to generate makefile
    + Run cmake in build folder, set source directory to project directory
    + Set PNC_DIR to PnetCDF install path
    + Set H5_DIRt o HDF5 install path
  + Compile the library
    + make 
    + make install
      + set DESTDIR to install directory
* Example
    ```
    ~/Desktop/ncmpi_vol$ mkdir build
    ~/Desktop/ncmpi_vol$ cd build
    ~/Desktop/ncmpi_vol/build$ cmake .. -DPNC_DIR=Path/to/PnetCDF/install -DH5_DIR=Path/to/HDF5/install
    -- The C compiler identification is GNU 8.1.0
    -- The CXX compiler identification is GNU 8.1.0
    -- Check for working C compiler: /usr/bin/cc
    -- Check for working C compiler: /usr/bin/cc -- works
    -- Detecting C compiler ABI info
    -- Detecting C compiler ABI info - done
    -- Detecting C compile features
    -- Detecting C compile features - done
    -- Check for working CXX compiler: /usr/bin/c++
    -- Check for working CXX compiler: /usr/bin/c++ -- works
    -- Detecting CXX compiler ABI info
    -- Detecting CXX compiler ABI info - done
    -- Detecting CXX compile features
    -- Detecting CXX compile features - done
    -- Found MPI_C: /usr/lib/openmpi/lib/libmpi.so (found version "3.0")
    -- Found MPI_CXX: /usr/lib/openmpi/lib/libmpi_cxx.so (found version "3.0")
    -- Found MPI: TRUE (found version "3.0")
    -- Configuring done
    -- Generating done
    -- Build files have been written to: /home/khl7265/Desktop/ncmpi_vol/build
    ~/Desktop/ncmpi_vol/build$ make
    Scanning dependencies of target ncmpi_vol
    [  9%] Building CXX object CMakeFiles/ncmpi_vol.dir/src/ncmpi_vol.cpp.o
    [ 18%] Building CXX object CMakeFiles/ncmpi_vol.dir/src/ncmpi_vol_att.cpp.o
    [ 27%] Building CXX object CMakeFiles/ncmpi_vol.dir/src/ncmpi_vol_dataset.cpp.o
    [ 36%] Building CXX object CMakeFiles/ncmpi_vol.dir/src/ncmpi_vol_file.cpp.o
    In file included from /usr/lib/openmpi/include/openmpi/ompi/mpi/cxx/mpicxx.h:41,
                    from /usr/lib/openmpi/include/mpi.h:2673,
                    from /home/khl7265/Desktop/ncmpi_vol/include/ncmpi_vol.h:34,
                    from /home/khl7265/Desktop/ncmpi_vol/src/ncmpi_vol_file.cpp:1:
    /home/khl7265/Desktop/ncmpi_vol/src/ncmpi_vol_file.cpp: In function ‘herr_t H5VL_ncmpi_file_get(void*, H5VL_file_get_t, hid_t, void**, __va_list_tag*)’:
    /home/khl7265/Desktop/ncmpi_vol/src/ncmpi_vol_file.cpp:241:42: warning: ‘H5I_type_t’ is promoted to ‘int’ when passed through ‘...’
                    type = va_arg(arguments, H5I_type_t);
                                            ^
    /home/khl7265/Desktop/ncmpi_vol/src/ncmpi_vol_file.cpp:241:42: note: (so you should pass ‘int’ not ‘H5I_type_t’ to ‘va_arg’)
    /home/khl7265/Desktop/ncmpi_vol/src/ncmpi_vol_file.cpp:241:42: note: if this code is reached, the program will abort
    /home/khl7265/Desktop/ncmpi_vol/src/ncmpi_vol_file.cpp: In function ‘herr_t H5VL_ncmpi_file_specific(void*, H5VL_file_specific_t, hid_t, void**, __va_list_tag*)’:
    /home/khl7265/Desktop/ncmpi_vol/src/ncmpi_vol_file.cpp:359:42: warning: ‘H5I_type_t’ is promoted to ‘int’ when passed through ‘...’
                    type = va_arg(arguments, H5I_type_t);
                                            ^
    /home/khl7265/Desktop/ncmpi_vol/src/ncmpi_vol_file.cpp:359:42: note: if this code is reached, the program will abort
    [ 45%] Building CXX object CMakeFiles/ncmpi_vol.dir/src/ncmpi_vol_group.cpp.o
    [ 54%] Building CXX object CMakeFiles/ncmpi_vol.dir/src/ncmpi_vol_util.cpp.o  
    [ 63%] Linking CXX static library libncmpi_vol.a
    [ 63%] Built target ncmpi_vol
    Scanning dependencies of target test_all
    [ 72%] Building CXX object test/CMakeFiles/test_all.dir/test.cpp.o
    [ 81%] Linking CXX executable test_all
    [ 81%] Built target test_all
    Scanning dependencies of target create_open
    [ 90%] Building CXX object examples/CMakeFiles/create_open.dir/create_open.cpp.o
    [100%] Linking CXX executable create_open
    [100%] Built target create_open
    ~/Desktop/ncmpi_vol/build$ make install DESTDIR=Install/path
    [ 63%] Built target ncmpi_vol
    [ 81%] Built target test_all
    [100%] Built target create_open
    Install the project...
    -- Install configuration: ""
    -- Installing: /home/khl7265/.local/ncmpi_vol/usr/local/lib/libncmpi_vol.a
    -- Installing: /home/khl7265/.local/ncmpi_vol/usr/local/include/ncmpi_vol.h
    ```

## How to use
* Include library header
  + include "ncmpi_vol.h" in the source file that register the PnetCDF VOL with HDF5
  + "ncmpi_vol.h" is located in the include directory under the install path
* Link library
  + link "libncmpi_vol.a"
  + "libncmpi_vol.a" is located in the lib directory under the install path
+ Use PnetCDF vol in application
  + Register PnetCDF VOL callback structure using H5VLregister_connector
    + Callback structure is call "H5VL_ncmpi_g"
  + Set PnetCDF in file creation property list to use PnetCDF VOL
  + See example program /examples/create_open.c

## Implmentation
* File
* Dataset
  + Dataset is mapped to NetCDF varaible
  + HDF5 has no concept of dimensions
    + The VOL create corresponding dimension in NetCDF file according to size in dataspacce
    + Dimensions are hidden from the application
* Attribute
  + Attributes maps to attribute in NetCDF
* Group
  + NetCDF does not have the concept of groups
  + All objects are directly attached to the file (root group)
  + The VOL simulate group by prepend the path of the group to the name of objects
    + Groups are treated as no more than a prefix
    + Objects can be allocated by full path or by name and the group contains it

## Known problem
* HDF5 developer branch required
  + VOL is not yet in stable release at the time of the writing
  + There can be ongoing change to VOL interface that make this prototype outdated
* Only simple selection is supported in dataspace slicing
  + At most 1 subarray can be selected at a time
* C++ compiler needed
  + Due to used of constant initilizer, a C++ compiler is required

## Future work
* Support multiple selection
  + Current implmentation assums there are only 1 selection 