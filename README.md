# Summary

This library is a prototype implementation of HDF5 VOL that uses PnetCDF for underlying I/O operation.
It enables applications to access the NetCDF formatted file using HDF5 API.

## Build PnetCDF
* VOL is not yet in stable release at the time of the writing
  + Use develop branch
* Steps
  + Download PnetCDF
  + Run autoconf
  + Configure HDF5
    + Defualt paramemter is sufficent
  + Compile and install
* Example
    ```
    git clone https://github.com/live-clones/hdf5.git -b developo
    cd hdf5
    autoreconf -i
    ./configure --prefix=${HOME}/hdf5_dev
    make
    make install
    ```

## Build HDF5 with VOL support
* VOL is not yet in stable release at the time of the writing
  + Use develop branch
* Steps
  + Clone develop branch from HDF5 repository
  + Run autoconf
  + Configure HDF5
    + Defualt paramemter is sufficent
  + Compile and install
* Example
    ```
    git clone https://github.com/live-clones/hdf5.git -b developo
    cd hdf5
    autoreconf -i
    ./configure --prefix=${HOME}/hdf5_dev
    make
    make install
    ```

## Building the PnetCDF VOL library
* Requirement
  + C++ compiler
    + Due to used of constant initializer, a C++ compiler is required
  + HDF5 with VOL support
    + Provides header that defines the VOL template structure
  + PnetCDF
    + This VOL use PnetCDF to access NetCDF files
  + Cmake utility
    + This library uses CMake to manage the build process
* Steps
  + Create a build directory
  + Run CMake to generate makefile
    + Run CMake in the build folder, set source directory to project directory
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
                             ...
    -- Build files have been written to: /home/khl7265/Desktop/ncmpi_vol/build
                             ...
    [100%] Built target create_open
    Install the project...
    -- Install configuration: ""
    -- Installing: /home/khl7265/.local/ncmpi_vol/usr/local/lib/libncmpi_vol.a
    -- Installing: /home/khl7265/.local/ncmpi_vol/usr/local/include/ncmpi_vol.h
    ```

## Using the PnetCDF VOL library
* Include library header
  + include "ncmpi_vol.h" in the source file that registers the PnetCDF VOL with HDF5
  + "ncmpi_vol.h" is located in the include directory under the install path
* Link library
  + link "libncmpi_vol.a"
  + "libncmpi_vol.a" is located in the lib directory under the install path
+ Use PnetCDF vol in application
  + Register PnetCDF VOL callback structure using H5VLregister_connector
    + Callback structure is call "H5VL_ncmpi_g"
  + Set PnetCDF in file creation property list to use PnetCDF VOL
  + See example program /examples/create_open.c

## Limitation
* Memory space selection is not supported
  + H5S_SEL_ALL is assumed
* Interleaving file space selection is not supported
  + The VOL assumes data is accessed in the order they are selected
  + HDF5 semantic requires that data is accessed in the order they are saved in the file
* H5DWrite and H5DRead only schedule the I/O operation but does not perform them
  + H5Fflush must be called to complete the actual I/O operation
  + Before H5Fflush is called, the buffer passed to H5DWrite and H5DRead cannot be used
* All metadata operation must be called collectively
  + all_coll_metadata_ops must be set in file access property list
  + coll_metadata_write must be set in file access property list

## Future work
* Support interleaving selections
* Support memory space selections