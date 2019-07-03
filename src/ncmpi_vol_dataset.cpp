#include "ncmpi_vol.h"
#include "pnetcdf.h"

/********************* */
/* Function prototypes */
/********************* */
void *H5VL_ncmpi_dataset_create(void *obj, const H5VL_loc_params_t *loc_params, const char *name, hid_t lcpl_id, hid_t type_id, hid_t space_id, hid_t dcpl_id, hid_t dapl_id, hid_t dxpl_id, void **req);
void *H5VL_ncmpi_dataset_open(void *obj, const H5VL_loc_params_t *loc_params, const char *name, hid_t dapl_id, hid_t dxpl_id, void **req);
herr_t H5VL_ncmpi_dataset_read(void *dset, hid_t mem_type_id, hid_t mem_space_id, hid_t file_space_id, hid_t plist_id, void *buf, void **req);
herr_t H5VL_ncmpi_dataset_write(void *dset, hid_t mem_type_id, hid_t mem_space_id, hid_t file_space_id, hid_t plist_id, const void *buf, void **req);
herr_t H5VL_ncmpi_dataset_get(void *dset, H5VL_dataset_get_t get_type, hid_t dxpl_id, void **req, va_list arguments);
herr_t H5VL_ncmpi_dataset_specific(void *obj, H5VL_dataset_specific_t specific_type, hid_t dxpl_id, void **req, va_list arguments);
herr_t H5VL_ncmpi_dataset_optional(void *obj, hid_t dxpl_id, void **req, va_list arguments);
herr_t H5VL_ncmpi_dataset_close(void *dset, hid_t dxpl_id, void **req);

const H5VL_dataset_class_t H5VL_ncmpi_dataset_g{
    H5VL_ncmpi_dataset_create,                /* create       */
    H5VL_ncmpi_dataset_open,                  /* open         */
    H5VL_ncmpi_dataset_read,                  /* read         */
    H5VL_ncmpi_dataset_write,                 /* write        */
    H5VL_ncmpi_dataset_get,                   /* get          */
    H5VL_ncmpi_dataset_specific,              /* specific     */
    H5VL_ncmpi_dataset_optional,              /* optional     */
    H5VL_ncmpi_dataset_close                  /* close        */
};

/*-------------------------------------------------------------------------
 * Function:    H5VL_ncmpi_dataset_create
 *
 * Purpose:     Handles the dataset create callback
 *
 * Return:      Success:    dataset pointer
 *              Failure:    NULL
 *
 *-------------------------------------------------------------------------
 */
void* H5VL_ncmpi_dataset_create(void *obj, const H5VL_loc_params_t *loc_params,
                                const char *name, hid_t lcpl_id, hid_t type_id, hid_t space_id,
                                hid_t dcpl_id, hid_t dapl_id, hid_t  dxpl_id,
                                void  **req) {
    int err;
    int i;
    hsize_t *dims;
    MPI_Offset dlen;
    nc_type type;
    char tmp[1024];
    char *ppath;
    H5VL_ncmpi_dataset_t *varp;        /* New dataset's info */
    H5VL_ncmpi_file_t *fp;
    H5VL_ncmpi_group_t *gp;

    /* Check arguments */
    if((loc_params->obj_type != H5I_FILE) && (loc_params->obj_type != H5I_GROUP))   RET_ERRN("container not a file or group")
    if(loc_params->type != H5VL_OBJECT_BY_SELF) RET_ERRN("loc_params->type is not H5VL_OBJECT_BY_SELF")
    if(H5I_DATATYPE != H5Iget_type(type_id))    RET_ERRN("invalid datatype ID")
    if(H5I_DATASPACE != H5Iget_type(space_id))   RET_ERRN("invalid dataspace ID")

    if (loc_params->obj_type == H5I_FILE){
        fp = (H5VL_ncmpi_file_t*)obj;
        gp = NULL;
        ppath = NULL;
    }
    else{
        gp = (H5VL_ncmpi_group_t*)obj;
        fp = gp->fp;
        ppath = gp->path;
    }

    // Convert to NC type
    type = h5t_to_nc_type(type_id);
    if (type == NC_NAT) RET_ERRN("only native type is supported")

    // Enter define mode
    err = enter_define_mode(fp); CHECK_ERRN

    varp = (H5VL_ncmpi_dataset_t*)malloc(sizeof(H5VL_ncmpi_dataset_t));

    varp->objtype = H5I_DATASET;
    varp->dcpl_id = dcpl_id;
    varp->dapl_id = dapl_id;
    varp->dxpl_id = dxpl_id;
    varp->fp = fp;
    varp->gp = gp;
    if (ppath == NULL){
        varp->path = (char*)malloc(strlen(name) + 1);
        sprintf(varp->path, "%s", name);
        varp->name = varp->path;
    }
    else{
        varp->path = (char*)malloc(strlen(ppath) + strlen(name) + 2);
        sprintf(varp->path, "%s_%s", ppath, name);
        varp->name = varp->path + strlen(ppath) + 1;
    }
    
    varp->ndim = H5Sget_simple_extent_ndims(space_id);
    if (varp->ndim < 0)   RET_ERRN("ndim < 0")

    dims = (hsize_t*)malloc(sizeof(hsize_t) * varp->ndim );
    varp->ndim = H5Sget_simple_extent_dims(space_id, dims, NULL);
    if (varp->ndim < 0)   RET_ERRN("ndim < 0")

    varp->dimids = (int*)malloc(sizeof(int) * varp->ndim );
    for(i = 0; i < varp->ndim; i++){
        dlen = dims[i];
        sprintf(tmp, "%s_dim_%d", name, i);
        err = ncmpi_def_dim(fp->ncid, tmp, dlen, varp->dimids + i); CHECK_ERRN
    }

    err = ncmpi_def_var(fp->ncid, varp->path, type, varp->ndim, varp->dimids, &(varp->varid)); CHECK_ERRJ

    free(dims);

    // Back to data mode
    err = enter_data_mode(fp); CHECK_ERRN

    return (void *)varp;

errout:
    free(varp->path);
    free(varp);

    return NULL;
} /* end H5VL_ncmpi_dataset_create() */


/*-------------------------------------------------------------------------
 * Function:    H5VL_ncmpi_dataset_open
 *
 * Purpose:     Handles the dataset open callback
 *
 * Return:      Success:    dataset pointer
 *              Failure:    NULL
 *
 *-------------------------------------------------------------------------
 */
void* H5VL_ncmpi_dataset_open(void *obj, const H5VL_loc_params_t *loc_params, const char *name, 
                            hid_t dapl_id, hid_t  dxpl_id, void  **req) {
    int err;
    int i;
    hsize_t *dims;
    MPI_Offset dlen;
    nc_type type;
    char tmp[1024];
    char *ppath;
    H5VL_ncmpi_dataset_t *varp;        /* New dataset's info */
    H5VL_ncmpi_group_t *gp;
    H5VL_ncmpi_file_t *fp;

    /* Check arguments */
    if((loc_params->obj_type != H5I_FILE) && (loc_params->obj_type != H5I_GROUP))   RET_ERRN("container not a file or group")
    if(loc_params->type != H5VL_OBJECT_BY_SELF) RET_ERRN("loc_params->type is not H5VL_OBJECT_BY_SELF")

    if (loc_params->obj_type == H5I_FILE){
        fp = (H5VL_ncmpi_file_t*)obj;
        gp = NULL;
        ppath = NULL;
    }
    else{
        gp = (H5VL_ncmpi_group_t*)obj;
        fp = gp->fp;
        ppath = gp->path;
    }
    
    varp = (H5VL_ncmpi_dataset_t*)malloc(sizeof(H5VL_ncmpi_dataset_t));

    varp->objtype = H5I_DATASET;
    varp->dcpl_id = -1;
    varp->dapl_id = dapl_id;
    varp->dxpl_id = dxpl_id;
    varp->fp = fp;
    varp->gp = gp;
    if (ppath == NULL){
        varp->path = (char*)malloc(strlen(name) + 1);
        sprintf(varp->path, "%s", name);
        varp->name = varp->path;
    }
    else{
        varp->path = (char*)malloc(strlen(ppath) + strlen(name) + 2);
        sprintf(varp->path, "%s_%s", ppath, name);
        varp->name = varp->path + strlen(ppath) + 1;
    }

    err = ncmpi_inq_varid(fp->ncid, varp->path, &(varp->varid)); CHECK_ERRJ

    err = ncmpi_inq_varndims(fp->ncid, varp->varid, &(varp->ndim)); CHECK_ERRJ

    varp->dimids = (int*)malloc(sizeof(int) * varp->ndim );
    err = ncmpi_inq_vardimid(fp->ncid, varp->varid, varp->dimids);  CHECK_ERRJ

    return (void *)varp;

errout:
    free(varp->path);
    free(varp);

    return NULL;
} /* end H5VL_ncmpi_dataset_open() */


/*-------------------------------------------------------------------------
 * Function:    H5VL_ncmpi_dataset_read
 *
 * Purpose:     Handles the dataset read callback
 *
 * Return:      SUCCEED/FAIL
 *
 *-------------------------------------------------------------------------
 */
herr_t H5VL_ncmpi_dataset_read(void *obj, hid_t mem_type_id, hid_t mem_space_id,
    hid_t file_space_id, hid_t  dxpl_id, void *buf,
    void  **req)
{
    int err;
    herr_t herr;
    int i;
    MPI_Datatype type;
    hsize_t *hstart, *hcount, *hstride, *hblock;
    MPI_Offset *start, *count, *stride;
    MPI_Offset nelems;
    H5FD_mpio_xfer_t xmode;
    H5VL_ncmpi_dataset_t *varp = (H5VL_ncmpi_dataset_t*)obj;

    err = H5Pget_dxpl_mpio(dxpl_id, &xmode); CHECK_ERR
    if (xmode == H5FD_MPIO_COLLECTIVE){
        err = enter_coll_mode(varp->fp); CHECK_ERR
    }

    // Convert to MPI type
    type = h5t_to_mpi_type(mem_type_id);
    if (type == MPI_DATATYPE_NULL) RET_ERR("only native type is supported")

    // Get start, count, stride
    hstart = (hsize_t*)malloc(sizeof(hsize_t) * varp->ndim * 4);
    hcount = hstart + varp->ndim;
    hstride = hcount + varp->ndim;
    hblock = hstride + varp->ndim;
    herr = H5Sget_regular_hyperslab(file_space_id, hstart, hstride, hcount, hblock); CHECK_HERR
    for(i = 0; i < varp->ndim; i++){
        if (hblock[i] != 1){
            free(hstart);
            RET_ERR("selection block must be 1")
        }
    }

    // Convert to netcdf start, count, stride
    start = (MPI_Offset*)malloc(sizeof(MPI_Offset) * varp->ndim * 3);
    count = start + varp->ndim;
    stride = count + varp->ndim;
    nelems = 1;
    for(i = 0; i < varp->ndim; i++){
        start[i] = hstart[i];
        count[i] = hcount[i];
        stride[i] = hstride[i];
        nelems *= count[i];
    }

    if (xmode == H5FD_MPIO_COLLECTIVE){
        err = ncmpi_get_vars_all(varp->fp->ncid, varp->varid, start, count, stride, buf, nelems, type); CHECK_ERR
    }
    else{
        err = ncmpi_get_vars(varp->fp->ncid, varp->varid, start, count, stride, buf, nelems, type); CHECK_ERR
    }

    free(hstart);
    free(start);

    if (xmode == H5FD_MPIO_COLLECTIVE){
        err = enter_indep_mode(varp->fp); CHECK_ERR
    }

    return 0;
} /* end H5VL_ncmpi_dataset_read() */


/*-------------------------------------------------------------------------
 * Function:    H5VL_ncmpi_dataset_write
 *
 * Purpose:     Handles the dataset write callback
 *
 * Return:      SUCCEED/FAIL
 *
 *-------------------------------------------------------------------------
 */
herr_t H5VL_ncmpi_dataset_write(void *obj, hid_t mem_type_id, hid_t mem_space_id,
                                hid_t file_space_id, hid_t  dxpl_id, const void *buf,
                                void  **req) {
    int err;
    herr_t herr;
    int i;
    MPI_Datatype type;
    hsize_t *hstart, *hcount, *hstride, *hblock;
    MPI_Offset *start, *count, *stride;
    MPI_Offset nelems;
    H5FD_mpio_xfer_t xmode;
    H5VL_ncmpi_dataset_t *varp = (H5VL_ncmpi_dataset_t*)obj;

    err = H5Pget_dxpl_mpio(dxpl_id, &xmode); CHECK_ERR
    if (xmode == H5FD_MPIO_COLLECTIVE){
        err = enter_coll_mode(varp->fp); CHECK_ERR
    }

    // Convert to MPI type
    type = h5t_to_mpi_type(mem_type_id);
    if (type == MPI_DATATYPE_NULL) RET_ERR("only native type is supported")

    // Get start, count, stride
    hstart = (hsize_t*)malloc(sizeof(hsize_t) * varp->ndim * 4);
    hcount = hstart + varp->ndim;
    hstride = hcount + varp->ndim;
    hblock = hstride + varp->ndim;
    herr = H5Sget_regular_hyperslab(file_space_id, hstart, hstride, hcount, hblock); CHECK_HERR
    for(i = 0; i < varp->ndim; i++){
        if (hblock[i] != 1){
            free(hstart);
            RET_ERR("selection block must be 1")
        }
    }

    // Convert to netcdf start, count, stride
    start = (MPI_Offset*)malloc(sizeof(MPI_Offset) * varp->ndim * 3);
    count = start + varp->ndim;
    stride = count + varp->ndim;
    nelems = 1;
    for(i = 0; i < varp->ndim; i++){
        start[i] = hstart[i];
        count[i] = hcount[i];
        stride[i] = hstride[i];
        nelems *= count[i];
    }

    if (xmode == H5FD_MPIO_COLLECTIVE){
        err = ncmpi_put_vars_all(varp->fp->ncid, varp->varid, start, count, stride, buf, nelems, type); CHECK_ERR
    }
    else{
        err = ncmpi_put_vars(varp->fp->ncid, varp->varid, start, count, stride, buf, nelems, type); CHECK_ERR
    }
    
    free(hstart);
    free(start);
    
    if (xmode == H5FD_MPIO_COLLECTIVE){
        err = enter_indep_mode(varp->fp); CHECK_ERR
    }

    return 0;
} /* end H5VL_ncmpi_dataset_write() */


/*-------------------------------------------------------------------------
 * Function:    H5VL_ncmpi_dataset_get
 *
 * Purpose:     Handles the dataset get callback
 *
 * Return:      SUCCEED/FAIL
 *
 *-------------------------------------------------------------------------
 */
herr_t H5VL_ncmpi_dataset_get(void *obj, H5VL_dataset_get_t get_type, hid_t  dxpl_id, void  **req, va_list arguments) {
    int err;
    H5VL_ncmpi_dataset_t *varp = (H5VL_ncmpi_dataset_t*)obj;

    switch(get_type) {
        /* H5Dget_space */
        case H5VL_DATASET_GET_SPACE:
            {
                int i;
                MPI_Offset dlen;
                hsize_t *dims;
                hid_t *ret_id = va_arg(arguments, hid_t*);

                // Get dim size
                dims = (hsize_t*)malloc(sizeof(hsize_t) * varp->ndim);
                for(i = 0; i < varp->ndim; i++){
                    err = ncmpi_inq_dimlen(varp->fp->ncid, varp->dimids[i], &dlen); CHECK_ERR
                    dims[i] = dlen;
                }

                *ret_id = H5Screate_simple(varp->ndim, dims, NULL);   

                free(dims);

                break;
            }

        /* H5Dget_space_statuc */
        case H5VL_DATASET_GET_SPACE_STATUS:
            {

                break;
            }

        /* H5Dget_type */
        case H5VL_DATASET_GET_TYPE:
            {
                hid_t   *ret_id = va_arg(arguments, hid_t *);
                nc_type xtype;

                err = ncmpi_inq_vartype(varp->fp->ncid, varp->varid, &xtype); CHECK_ERR

                *ret_id = nc_to_h5t_type(xtype);

                break;
            }

        /* H5Dget_create_plist */
        case H5VL_DATASET_GET_DCPL:
            {
                hid_t   *ret_id = va_arg(arguments, hid_t *);

                *ret_id = varp->dcpl_id;

                break;
            }

        /* H5Dget_access_plist */
        case H5VL_DATASET_GET_DAPL:
            {
                hid_t   *ret_id = va_arg(arguments, hid_t *);

                *ret_id = varp->dapl_id;

                break;
            }

        /* H5Dget_storage_size */
        case H5VL_DATASET_GET_STORAGE_SIZE:
            {
                hsize_t *ret = va_arg(arguments, hsize_t *);

                break;
            }

        /* H5Dget_offset */
        case H5VL_DATASET_GET_OFFSET:
            {
                haddr_t *ret = va_arg(arguments, haddr_t *);
                MPI_Offset off;

                err = ncmpi_inq_varoffset(varp->fp->ncid, varp->varid, &off); CHECK_ERR

                *ret = off;

                break;
            }

        default:
            RET_ERR("get_type not supported")
    } /* end switch */
    
    return 0;
} /* end H5VL_ncmpi_dataset_get() */


/*-------------------------------------------------------------------------
 * Function:    H5VL_ncmpi_dataset_specific
 *
 * Purpose:     Handles the dataset specific callback
 *
 * Return:      SUCCEED/FAIL
 *
 *-------------------------------------------------------------------------
 */
herr_t H5VL_ncmpi_dataset_specific(void *obj, H5VL_dataset_specific_t specific_type, hid_t  dxpl_id, void  **req, va_list arguments) {
    int err;
    H5VL_ncmpi_dataset_t *varp = (H5VL_ncmpi_dataset_t*)obj;

    switch(specific_type) {
        /* H5Dspecific_space */
        case H5VL_DATASET_SET_EXTENT:
            {
                const hsize_t *size = va_arg(arguments, const hsize_t *); 

                RET_ERR("can not change dataset size")

                break;
            }

        case H5VL_DATASET_FLUSH:
            {
                hid_t dset_id = va_arg(arguments, hid_t);

                err = ncmpi_flush(varp->fp->ncid); CHECK_ERR

                break;
            }

        case H5VL_DATASET_REFRESH:
            {
                hid_t dset_id = va_arg(arguments, hid_t);

                break;
            }

        default:
            RET_ERR("specific_type not supported")
    } /* end switch */
    return 0;
} /* end H5VL_ncmpi_dataset_specific() */


/*-------------------------------------------------------------------------
 * Function:    H5VL_ncmpi_dataset_optional
 *
 * Purpose:     Handles the dataset optional callback
 *
 * Return:      SUCCEED/FAIL
 *
 *-------------------------------------------------------------------------
 */
herr_t H5VL_ncmpi_dataset_optional(void *obj, hid_t  dxpl_id, void  **req, va_list arguments) {

    return 0;
} /* end H5VL_ncmpi_dataset_optional() */


/*-------------------------------------------------------------------------
 * Function:    H5VL_ncmpi_dataset_close
 *
 * Purpose:     Handles the dataset close callback
 *
 * Return:      Success:    SUCCEED
 *              Failure:    FAIL (dataset will not be closed)
 *
 *-------------------------------------------------------------------------
 */
herr_t H5VL_ncmpi_dataset_close(void *dset, hid_t  dxpl_id, void  **req) {
    H5VL_ncmpi_dataset_t *varp = (H5VL_ncmpi_dataset_t*)dset;
    
    free(varp->dimids);
    free(varp->path);
    free(varp);

    return 0;
} /* end H5VL_ncmpi_dataset_close() */

