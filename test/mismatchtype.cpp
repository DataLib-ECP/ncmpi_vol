#include <stdio.h>
#include <stdlib.h>

#include <hdf5.h>
#include "ncmpi_vol.h"

#define N 2
#define NT 8

int main(int argc, char **argv) {  
    int i, j;  
    int rank, np;
    const char *file_name;  
    hid_t types[] = {H5T_STD_I16BE, H5T_STD_I32BE, H5T_STD_I64BE,
                     H5T_STD_U16BE, H5T_STD_U32BE, H5T_STD_U64BE,
                     H5T_IEEE_F32BE, H5T_IEEE_F64BE};
    hid_t file_id, dspace_id, aspace_id;
    hid_t dset_ids[NT], att_ids[NT];
    hid_t fapl_id, pnc_vol_id, dxpl_id;  
    hsize_t dims[2], start[2], count[2];
    int buf[N * 8];
    char name[PNC_VOL_MAX_NAME];
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

    // Create file
    file_id  = H5Fcreate(file_name, H5F_ACC_TRUNC, H5P_DEFAULT, fapl_id);     

    // Define data space
    dims [0]    = np;
    dims [1]    = N;
    dspace_id = H5Screate_simple(2, dims, NULL);   
    aspace_id = H5Screate_simple(1, dims + 1, NULL);   

    // Select hyper slab
    start[0] = rank;
    start[1] = 0;
    count[0] = 1;
    count[1] = N;
    H5Sselect_hyperslab(dspace_id, H5S_SELECT_SET, start, NULL, count, NULL);

    // Create dataset and attribute
    for (i = 0; i < NT; i++){
        // Dataset
        sprintf(name, "Dataset%d", i);
        dset_ids[i]  = H5Dcreate(file_id, name, types[i], dspace_id, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);  

        // Attribute
        sprintf(name, "Attribute%d", i);
        att_ids[i]  = H5Acreate(file_id, name, types[i], aspace_id, H5P_DEFAULT, H5P_DEFAULT);
    }

    
    dxpl_id = H5Pcreate (H5P_DATASET_XFER);
    H5Pset_dxpl_mpio(dxpl_id, H5FD_MPIO_COLLECTIVE);

    // Write
    for (i = 0; i < NT; i++){
        for(j = 0; j < N; j++){
            buf[j] = rank + 1 + j;
        }
        H5Dwrite(dset_ids[i], H5T_STD_I32BE, aspace_id, dspace_id, dxpl_id, buf);  
        H5Fflush(file_id, H5F_SCOPE_GLOBAL);
        for(j = 0; j < N; j++){
            buf[j] = np + 1 + j;
        }
        H5Awrite(att_ids[i], H5T_STD_I32BE, buf);
    }

    // Read
    for (i = 0; i < NT; i++){
        for(j = 0; j < N; j++){
            buf[j] = 0;
        }
        H5Dread(dset_ids[i], H5T_STD_I32BE, aspace_id, dspace_id, dxpl_id, buf);  
        H5Fflush(file_id, H5F_SCOPE_GLOBAL);
        for(j = 0; j < N; j++){
            if (buf[j] != rank + 1 + j){
                printf("Rank %d: Error. Expect buf[%d] = %d, but got %d\n", rank, j, rank + 1 + j, buf[j]);
            }
        }

        for(j = 0; j < N; j++){
            buf[j] = 0;
        }
        H5Aread(att_ids[i], H5T_STD_I32BE, buf);
        for(j = 0; j < N; j++){
            if (buf[j] != np + 1 + j){
                printf("Rank %d: Error. Expect buf[%d] = %d, but got %d\n", rank, j, np + 1 + j, buf[j]);
            }
        }
    }

    for (i = 0; i < NT; i++){
        H5Aclose(att_ids[i]);
        H5Dclose(dset_ids[i]);  
    }

    H5Sclose(dspace_id);
    H5Sclose(aspace_id);
    H5Fclose(file_id);
    
    H5VLclose(pnc_vol_id);
    H5Pclose(fapl_id);
    H5Pclose(dxpl_id);

    MPI_Finalize();

    return 0;  
}  


