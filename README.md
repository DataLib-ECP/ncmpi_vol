# Summary

A prototype implementation of PnetCDF VOL

## How to build
* Create build directory
* Config
  + Run cmake project directory in build folder
  + PNC_DIR: PnetCDF install path
  + H5_DIR: HDF5 install path
* Compile
  + make
  + make install
* Example
  ```
  ncmpi_vol$ rm -rf build/
  ncmpi_vol$ mkdir build
  ncmpi_vol$ cd build/
  ncmpi_vol/build$ cmake .. -DCMAKE_INSTALL_PREFIX=Install/path -DPNC_DIR=Path/to/PnetCDF/install -DH5_DIR=Path/to/HDF5/install
  make all install
  ```

## How to use
* Include header
  + ncmpi_vol.h
* Link library
  + libncmpi_vol.a
+ Register vol with HDF
  ```
  pnc_vol_id = H5VLregister_connector(&H5VL_ncmpi_g, H5P_DEFAULT); 
  pnc_vol_info.comm = MPI_COMM_WORLD;
  H5Pset_vol(pnc_fapl, pnc_vol_id, &pnc_vol_info);
  ```

## Known problem
* HDF5 developer branch required
  + VOL is not yet in stable release at the time of the writing
  + There can be ongoing change to VOL interface that make this prototype outdated
* Only simple selection is supported in dataspace slicing
  + At most 1 subarray can be selected at a time

## Future work
* Support multiple selection
  + Current implmentation assums there are only 1 selection 