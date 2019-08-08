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

#define N 10

int main(int argc, char **argv) {  
    int rank, np;
    int i;
    const char *file_name;  
    hid_t fapl_id, pnc_vol_id;
    hid_t file_id, dspace_id, group_id, dset_id, dxpl_id, fatt_id, datt_id, gatt_id;
    hsize_t dim;
    int buf[N];
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

    // Collective I/O
    dxpl_id = H5Pcreate (H5P_DATASET_XFER);
    H5Pset_dxpl_mpio(dxpl_id, H5FD_MPIO_COLLECTIVE);
    
    // Define dataspace
    dim = N;
    dspace_id = H5Screate_simple(1, &dim, NULL);   // Dataset space

    // Create file
    file_id = H5Fcreate(file_name, H5F_ACC_TRUNC, H5P_DEFAULT, fapl_id);     
    fatt_id = H5Acreate(file_id, "File_Att", H5T_NATIVE_INT, dspace_id, H5P_DEFAULT, H5P_DEFAULT);

    // Define group
    group_id = H5Gcreate2(file_id, "G", H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT); 
    gatt_id = H5Acreate(group_id, "Group_Att", H5T_NATIVE_INT, dspace_id, H5P_DEFAULT, H5P_DEFAULT);

    // Define dataset
    dset_id  = H5Dcreate(group_id, "D", H5T_NATIVE_INT, dspace_id, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);  
    datt_id = H5Acreate(dset_id, "Dataset_Att", H5T_NATIVE_INT, dspace_id, H5P_DEFAULT, H5P_DEFAULT);

    // Write attribute
    for(i = 0; i < N; i++){
        buf[i] = np + 1 + i;
    }
    H5Awrite (fatt_id, H5T_NATIVE_INT, buf);
    for(i = 0; i < N; i++){
        buf[i] = np + 2 + i;
    }
    H5Awrite (gatt_id, H5T_NATIVE_INT, buf);
    for(i = 0; i < N; i++){
        buf[i] = np + 3 + i;
    }
    H5Awrite (datt_id, H5T_NATIVE_INT, buf);

    // Close handles
    H5Aclose(datt_id);
    H5Aclose(gatt_id);
    H5Aclose(fatt_id);
    H5Dclose(dset_id);
    H5Gclose(group_id);
    H5Fclose(file_id);

    // Open file
    file_id = H5Fopen(file_name, H5F_ACC_RDONLY, fapl_id);     
    fatt_id = H5Aopen(file_id, "File_Att", H5P_DEFAULT);

    // Open group
    group_id = H5Gopen1(file_id, "G"); 
    gatt_id = H5Aopen(group_id, "Group_Att", H5P_DEFAULT);

    // Open dataset
    dset_id  = H5Dopen2(group_id, "D", H5P_DEFAULT);  
    datt_id = H5Aopen(dset_id, "Dataset_Att", H5P_DEFAULT);
    
    // Read attribute
    for(i = 0; i < N; i++){
        buf[i] = 0;
    }
    H5Aread(fatt_id, H5T_NATIVE_INT, buf);
    for(i = 0; i < N; i++){
        if (buf[i] != np + 1 + i){
            printf("Rank %d: Error. Expect buf[%d] = %d, but got %d\n", rank, i, np + 1 + i, buf[i]);
        }
    }
    for(i = 0; i < N; i++){
        buf[i] = 0;
    }
    H5Aread(gatt_id, H5T_NATIVE_INT, buf);
    for(i = 0; i < N; i++){
        if (buf[i] != np + 2 + i){
            printf("Rank %d: Error. Expect buf[%d] = %d, but got %d\n", rank, i, np + 2 + i, buf[i]);
        }
    }
    for(i = 0; i < N; i++){
        buf[i] = 0;
    }
    H5Aread(datt_id, H5T_NATIVE_INT, buf);
    for(i = 0; i < N; i++){
        if (buf[i] != np + 3 + i){
            printf("Rank %d: Error. Expect buf[%d] = %d, but got %d\n", rank, i, np + 3 + i, buf[i]);
        }
    }

    // Close handles
    H5Aclose(datt_id);
    H5Aclose(gatt_id);
    H5Aclose(fatt_id);
    H5Dclose(dset_id);
    H5Gclose(group_id);
    H5Fclose(file_id);

    H5VLclose(pnc_vol_id);
    H5Pclose(fapl_id);
    H5Pclose(dxpl_id);
    
    MPI_Finalize();

    return 0;  
}  

