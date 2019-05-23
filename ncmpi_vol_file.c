#include "ncmpi_vol.h"
#include "pnetcdf.h"

const H5VL_file_class_t H5VL_ncmpi_file_g{
    H5VL_ncmpi_file_create,                       /* create */
    NULL,                         /* open */
    NULL,                          /* get */
    NULL,                     /* specific */
    NULL,                     /* optional */
    H5VL_ncmpi_file_close                         /* close */
};

/*-------------------------------------------------------------------------
 * Function:    H5VL_ncmpi_file_create
 *
 * Purpose:     Creates a container using this connector
 *
 * Return:      Success:    Pointer to a file object
 *              Failure:    NULL
 *
 *-------------------------------------------------------------------------
 */
void *H5VL_ncmpi_file_create(const char *name, unsigned flags, hid_t fcpl_id, hid_t fapl_id, hid_t dxpl_id, void **req) {
    int err;
    int ncid;
    H5VL_ncmpi_t *file;

    printf("ncmpi_vol_create\n");

    err = ncmpi_create(MPI_COMM_WORLD, name, NC_64BIT_DATA, MPI_INFO_NULL, &ncid);

    if (err != NC_NOERR){
        return NULL;
    }

    file = (H5VL_ncmpi_t*)malloc(sizeof(H5VL_ncmpi_t));
    file->ncid = ncid;

    return((void *)file);
} /* end H5VL_ncmpi_file_create() */

/*-------------------------------------------------------------------------
 * Function:    H5VL_ncmpi_file_close
 *
 * Purpose:     Closes a file.
 *
 * Return:  Success:    0
 *      Failure:    -1, file not closed.
 *
 *-------------------------------------------------------------------------
 */
herr_t H5VL_ncmpi_file_close(void *file, hid_t dxpl_id, void **req) {
    int err;
    H5VL_ncmpi_t *ncvlp = (H5VL_ncmpi_t *)file;
    herr_t ret_value;

    err = ncmpi_close(ncvlp->ncid);

    free(ncvlp);

    if (err != NC_NOERR){
        return -1;
    }

    return 0;
} /* end H5VL_ncmpi_file_close() */