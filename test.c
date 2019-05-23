#include <stdio.h>
#include <stdlib.h>

#include <hdf5.h>
#include "ncmpi_vol.h"

#define N 10

int main(int argc, char **argv) {  
    int i, j;  
    int rank, np;
    const char *file_name;  
    char dataset_name[]="data";  
    hid_t file_id, datasetId, dataspaceId, file_space, memspace_id;
    hid_t pnc_fapl, pnc_vol_id;  
    hsize_t dims[2], start[2], count[2];  
    H5VL_ncmpi_info_t pnc_vol_info;

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

    //Register DataElevator plugin 
    pnc_fapl = H5Pcreate(H5P_FILE_ACCESS); 
    H5Pset_fapl_mpio(pnc_fapl, MPI_COMM_WORLD, MPI_INFO_NULL);
    pnc_vol_id = H5VLregister_connector(&H5VL_ncmpi_g, H5P_DEFAULT); 
    pnc_vol_info.comm = MPI_COMM_WORLD;
    H5Pset_vol(pnc_fapl, pnc_vol_id, &pnc_vol_info);

    // Create file
    file_id  = H5Fcreate(file_name, H5F_ACC_TRUNC, H5P_DEFAULT, pnc_fapl);     

    /*
    dims [0]    = np;
    dims [1]    = N;
    dataspaceId = H5Screate_simple(2, dims, NULL);   
    datasetId   = H5Dcreate(file_id,dataset_name,H5T_NATIVE_INT,dataspaceId,H5P_DEFAULT,H5P_DEFAULT,H5P_DEFAULT);  
    memspace_id = H5Screate_simple(1, dims + 1, NULL);
    file_space = H5Dget_space(datasetId);
    start[0] = rank;
    start[1] = 0;
    count[0] = 1;
    count[1] = N;
    H5Sselect_hyperslab(file_space, H5S_SELECT_SET, start, NULL, count, NULL);

    H5Dwrite(datasetId, H5T_NATIVE_INT, memspace_id, file_space, H5P_DEFAULT, data);  

    H5Sclose(file_space);
    H5Sclose(memspace_id);
    H5Sclose(dataspaceId);  
    H5Dclose(datasetId);  
    */

    H5Fclose(file_id);
    
    MPI_Finalize();

    return 0;  
}  


