## NCMPI_VOL - an HDF5 VOL Plugin for Accessing NetCDF Classic-based Files in Parallel

This software repository contains source codes implementing an [HDF5](https://www.hdfgroup.org) Virtual Object Layer ([VOL](https://bitbucket.hdfgroup.org/projects/HDFFV/repos/hdf5doc/browse/RFCs/HDF5/VOL/developer_guide/main.pdf)) plugin that allows applications to use HDF5 APIs to read and write NetCDF classic files in parallel. This plugin is built on top of [PnetCDF](https://parallel-netcdf.github.io), a parallel I/O library that provides parallel access to NetCDF files.

### Software Requirements
* [HDF5 develop branch](https://bitbucket.hdfgroup.org/scm/hdffv/hdf5.git)
  + Note that HDF5 VOL has not yet been released officially at the time of this writing
  + Parallel I/O support (--enable-parallel) is required
* [PnetCDF](https://parallel-netcdf.github.io/wiki/Download.html) version 1.12.0 and later
* MPI C and C++ compilers
  + The plugin uses the constant initializer, a C++ compiler supporting std 98 is required
* Cmake utility

### Building Steps
* Build PnetCDF
  + Full instructions for building PnetCDF can be found in file `INSTALL` come with all official releases.
  + Example configure and make commands are given below:
    ```
    % cd pnetcdf-1.12.1
    % ./configure --prefix=${HOME}/PnetCDF
    % make -j4 install
    ```
    The PnetCDF library is now installed under folder `${HOME}/PnetCDF`.
* Build HDF5 with VOL and parallel I/O support
  + Clone the develop branch from HDF5 repository
  + Run command ./autogen.sh
  + Configure HDF5 with parallel I/O enabled
  + Run make install
  + Example commands are given below. This example will install
    the HD5 library under folder `${HOME}/HDF5`.
    ```
    % git clone https://bitbucket.hdfgroup.org/scm/hdffv/hdf5.git
    % cd hdf5
    % git checkout develop
    % ./autogen
    % ./configure --prefix=${HOME}/HDF5 --enable-parallel CC=mpicc
    % make -j4 install
    ```
* Build this VOL plugin, `ncmpi_vol`
  + Clone this VOL plugin repository
  + Create a new build folder
  + Run CMake and make
  + Example commands are given below.
    ```
    % git clone https://github.com/DataLib-ECP/ncmpi_vol.git
    % cd ncmpi_vol
    % mkdir build
    % cd build
    % cmake .. -DPNC_DIR=${HOME}/PnetCDF -DH5_DIR=${HOME}/HDF5 -DCMAKE_INSTALL_PREFIX=${HOME}/NCMPI_VOL
    % make install
    ```
    The VOL plugin library is now installed under folder `${HOME}/NCMPI_VOL`.

### Compile user programs that use this VOL plugin
* Include header file.
  + Add the following line to your C/C++ source codes.
    ```
    #include <ncmpi_vol.h>
    ```
  + Header file `ncmpi_vol.h` is located in folder `${HOME}/NCMPI_VOL/include`
  + Add `-I${HOME}/NCMPI_VOL/include` to your compile command line. For example,
    ```
    % mpicc prog.c -o prog.o -I${HOME}/NCMPI_VOL/include
    ```
* Library file.
  + The library file, `libncmpi_vol.a`, is located under folder `${HOME}/NCMPI_VOL/lib`.
  + Add `-L${HOME}/NCMPI_VOL/lib -lncmpi_vol` to your compile/link command line. For example,
    ```
    % mpicc prog.o -o prog -L${HOME}/NCMPI_VOL/lib -lncmpi_vol \
                           -L${HOME}/HDF5/lib -lhdf5 \
                           -L${HOME}/PnetCDF/lib -lpnetcdf
    ```

### Use ncmpi_vol plugin in user programs
  + Register VOL callback structure using `H5VLregister_connector`
  + Callback structure is named `H5VL_ncmpi_g`
  + Set a file creation property list to use ncmpi_vol
  + For example,
    ```
    fapl_id = H5Pcreate(H5P_FILE_ACCESS); 
    H5Pset_fapl_mpio(fapl_id, comm, info);
    H5Pset_all_coll_metadata_ops(fapl_id, true);
    H5Pset_coll_metadata_write(fapl_id, true);
    pnc_vol_id = H5VLregister_connector(&H5VL_ncmpi_g, H5P_DEFAULT);
    H5Pset_vol(fapl_id, pnc_vol_id, &pnc_vol_info);
    ```
  + See a full example program in `examples/create_open.c`

### Current limitations
  + HDF5 memory space selection is not supported
    + `H5S_SEL_ALL` is assumed
    + Supporting memory space of noncontiguous user buffers is a future work.
  + Semantics changed for APIs `H5DWrite` and `H5Dread`
    + `H5DWrite` and `H5Dread` are now nonblocking. They simply post the requests and users are required to flushed them later.
    + `H5Fflush` must be called to complete the I/O requests
    + Once the requests are posted, users are required not to change the contents of user buffers before the call to ` H5Fflush`.
  + All metadata operations must be called collectively
    + `H5Pset_all_coll_metadata_ops` must be called to set the file access property list to make metadata operations collective.
    + `H5Pset_coll_metadata_write` is also required.

### References
* [HDF5 VOL application developer manual](https://bitbucket.hdfgroup.org/projects/HDFFV/repos/hdf5doc/browse/RFCs/HDF5/VOL/developer_guide/main.pdf)
* [HDF5 VOL plug-in developer manual](https://bitbucket.hdfgroup.org/projects/HDFFV/repos/hdf5doc/browse/RFCs/HDF5/VOL/user_guide)
* [HDF5 VOL RFC](https://bitbucket.hdfgroup.org/projects/HDFFV/repos/hdf5doc/browse/RFCs/HDF5/VOL/RFC)
* [ExaHDF5 PnetCDF VOL](https://bitbucket.hdfgroup.org/projects/HDF5VOL/repos/pnetcdf-vol/browse)
  + Developed by [ExaHDF5](https://sdm.lbl.gov/exahdf5) team
  + Currently supports read-only operations
  + Supports selection in memory space

### Project funding supports:
Ongoing development and maintenance of NCMPI_VOL is supported by the Exascale Computing Project (17-SC-20-SC), a joint project of the U.S. Department of Energy's Office of Science and National Nuclear Security Administration, responsible for delivering a capable exascale ecosystem, including software, applications, and hardware technology, to support the nation's exascale computing imperative.
