# Summary

This library is a prototype implementation of HDF5 VOL that uses PnetCDF for underlying I/O operation.
It enables applications to access the NetCDF formatted file using HDF5 API.

## How to build
* Requirement
  + C++ compiler
    + Due to used of constant initializer, a C++ compiler is required
  + HDF5 developer branch
    + VOL is not yet in stable release at the time of the writing
* This library uses CMake to manage the build process
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

## How to use
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

## Implmentation
* File
* Dataset
  + Dataset is mapped to NetCDF varaible
  + HDF5 has no concept of dimensions
    + The VOL create the corresponding dimension in NetCDF file according to size in dataspace
    + Dimensions are hidden from the application
* Attribute
  + Attributes maps to attribute in NetCDF
* Group
  + NetCDF does not have the concept of groups
  + All objects are directly attached to the file (root group)
  + The VOL simulate group by prepending the path of the group to the name of objects
    + Groups are treated as no more than a prefix
    + Objects can be allocated by full path or by name and the group contains it

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