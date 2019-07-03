/*********************************************************************
 *
 *  Copyright (C) 2019, Northwestern University and Argonne National Laboratory
 *  See COPYRIGHT notice in top-level directory.
 *
 *********************************************************************/
/* $Id$ */

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * This example shows how to opee file using PnetCDF VOL.
 *
 *    To compile:
 *        mpicc -O2 create_open.c -o create_open -lpnetcdf
 *
 * Example commands for MPI run and outputs from running ncmpidump on the
 * netCDF file produced by this example program:
 *
 *    % mpiexec -n 4 ./create_open /pvfs2/wkliao/testfile.nc
 *
 *    % ncmpidump /pvfs2/wkliao/testfile.nc
 *    netcdf testfile {
 *    // file format: CDF-1
 *    }
 *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include <stdio.h>
#include <stdlib.h>

#include <hdf5.h>
#include "ncmpi_vol.h"

int main(int argc, char **argv) {  
    int rank, np;
    const char *file_name;  
    hid_t fapl_id, pnc_vol_id;
    hid_t file_id;
    H5VL_ncmpi_info_t pnc_vol_info; // PnetCDF VOL callback

    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &np);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    if (argc > 1){
        file_name = argv[1];
    }
    else{
        file_name = "test.nc";
    }
    if(rank == 0) printf("Writing file_name = %s at rank 0 \n", file_name);

    // PnetCDF VOL require MPI-IO and parallel access
    // Set MPI-IO and parallel access proterty.
    fapl_id = H5Pcreate(H5P_FILE_ACCESS); 
    H5Pset_fapl_mpio(fapl_id, MPI_COMM_WORLD, MPI_INFO_NULL);
    H5Pset_all_coll_metadata_ops(fapl_id, 1);
    H5Pset_coll_metadata_write(fapl_id, 1);

    // Resgiter PnetCDF VOL
    pnc_vol_id = H5VLregister_connector(&H5VL_ncmpi_g, H5P_DEFAULT); 
    pnc_vol_info.comm = MPI_COMM_WORLD;
    H5Pset_vol(fapl_id, pnc_vol_id, &pnc_vol_info);

    // Create file
    file_id = H5Fcreate(file_name, H5F_ACC_TRUNC, H5P_DEFAULT, fapl_id);     
    
    /*
     * Use HDF5 as ususal.
     * PnetCDF VOL supports attribute, datasest, and group.
     */

    // Close file
    H5Fclose(file_id);

    // Open file
    file_id = H5Fopen(file_name, H5F_ACC_RDONLY, fapl_id);     
    
    /*
     * Use HDF5 as ususal.
     * PnetCDF VOL supports attribute, datasest, and group.
     */

    // Close file
    H5Fclose(file_id);

    H5VLclose(pnc_vol_id);
    H5Pclose(fapl_id);
    
    MPI_Finalize();

    return 0;  
}  


