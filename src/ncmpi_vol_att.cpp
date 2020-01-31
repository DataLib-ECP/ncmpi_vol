#include "ncmpi_vol.h"
#include "pnetcdf.h"

/********************* */
/* Function prototypes */
/********************* */

H5_DLL void *H5VL_ncmpi_attr_create(void *obj, const H5VL_loc_params_t *loc_params, const char *attr_name, hid_t type_id, hid_t space_id, hid_t acpl_id, hid_t aapl_id, hid_t dxpl_id, void **req);
void *H5VL_ncmpi_attr_open(void *obj, const H5VL_loc_params_t *loc_params, const char *attr_name, hid_t aapl_id, hid_t dxpl_id, void **req);
H5_DLL herr_t H5VL_ncmpi_attr_read(void *attr, hid_t dtype_id, void *buf, hid_t dxpl_id, void **req);
H5_DLL herr_t H5VL_ncmpi_attr_write(void *attr, hid_t dtype_id, const void *buf, hid_t dxpl_id, void **req);
H5_DLL herr_t H5VL_ncmpi_attr_get(void *obj, H5VL_attr_get_t get_type, hid_t dxpl_id, void **req, va_list arguments);
H5_DLL herr_t H5VL_ncmpi_attr_specific(void *obj, const H5VL_loc_params_t *loc_params, H5VL_attr_specific_t specific_type, hid_t dxpl_id, void **req, va_list arguments);
H5_DLL herr_t H5VL_ncmpi_attr_optional(void *obj, H5VL_file_optional_t opt_type, hid_t dxpl_id, void **req, va_list arguments);
H5_DLL herr_t H5VL_ncmpi_attr_close(void *attr, hid_t dxpl_id, void **req);

const H5VL_attr_class_t H5VL_ncmpi_attr_g{
    H5VL_ncmpi_attr_create,                /* create       */
    H5VL_ncmpi_attr_open,                  /* open         */
    H5VL_ncmpi_attr_read,                  /* read         */
    H5VL_ncmpi_attr_write,                 /* write        */
    H5VL_ncmpi_attr_get,                   /* get          */
    H5VL_ncmpi_attr_specific,              /* specific     */
    H5VL_ncmpi_attr_optional,              /* optional     */
    H5VL_ncmpi_attr_close                  /* close        */
};

/*-------------------------------------------------------------------------
 * Function:    H5VL_ncmpi_attr_create
 *
 * Purpose:     Handles the attribute create callback
 *
 * Return:      Success:    attribute pointer
 *              Failure:    NULL
 *
 *-------------------------------------------------------------------------
 */
void *H5VL_ncmpi_attr_create(   void *obj, const H5VL_loc_params_t *loc_params, const char *attr_name,
                                hid_t type_id, hid_t space_id, hid_t acpl_id, hid_t aapl_id,
                                hid_t dxpl_id, void **req) {
    int err;
    int i;
    int ndim;
    hsize_t *dims;
    int varid;
    nc_type type;
    char *buf = NULL;
    char path[PNC_VOL_MAX_NAME];
    char *ppath;
    H5VL_ncmpi_attr_t *attp;
    H5VL_ncmpi_file_t *fp;

    /* Check arguments */
    if((loc_params->obj_type != H5I_FILE) && (loc_params->obj_type != H5I_GROUP) && (loc_params->obj_type != H5I_DATASET))   RET_ERRN("container not a file or group or dataset")
    if(loc_params->type != H5VL_OBJECT_BY_SELF) RET_ERRN("loc_params->type is not H5VL_OBJECT_BY_SELF")
    //if(H5I_DATATYPE != H5Iget_type(type_id))    RET_ERRN("invalid datatype ID")
    if(H5I_DATASPACE != H5Iget_type(space_id))   RET_ERRN("invalid dataspace ID")

    if (loc_params->obj_type == H5I_FILE){
        fp = (H5VL_ncmpi_file_t*)obj;
        varid = NC_GLOBAL;
        ppath = NULL;
    }
    else if (loc_params->obj_type == H5I_GROUP){
        fp = ((H5VL_ncmpi_group_t*)obj)->fp;
        ppath = ((H5VL_ncmpi_group_t*)obj)->path;
        varid = NC_GLOBAL;
    }
    else{
        fp = ((H5VL_ncmpi_dataset_t*)obj)->fp;
        varid = ((H5VL_ncmpi_dataset_t*)obj)->varid;
        ppath = NULL;
    }

    // Convert to NC type
    //type = h5t_to_nc_type(type_id);
    type = (nc_type)type_id;
    if (type == NC_NAT) RET_ERRN("only native type is supported")

    // Enter define mode
    err = enter_define_mode(fp); CHECK_ERRN

    attp = (H5VL_ncmpi_attr_t*)malloc(sizeof(H5VL_ncmpi_attr_t));
    attp->objtype = H5I_ATTR;
    attp->acpl_id = acpl_id;
    attp->aapl_id = aapl_id;
    attp->dxpl_id = dxpl_id;
    attp->type = type;
    attp->varid = varid;
    attp->fp = fp;
    if (ppath == NULL){
        sprintf(attp->path, "%s", attr_name);
        attp->name = attp->path;
    }
    else{
        sprintf(attp->path, "%s_%s", ppath, attr_name);
        attp->name = attp->path + strlen(ppath) + 1;
    }

    ndim = H5Sget_simple_extent_ndims(space_id);
    if (ndim < 0)   RET_ERRN("ndim < 0")
    if (ndim > 1)   RET_ERRN("Attribute dimension > 1 is not supported")

    dims = (hsize_t*)malloc(sizeof(hsize_t) * ndim);
    H5Sget_simple_extent_dims(space_id, dims, NULL);
    
    attp->size = 1;
    for(i = 0; i < ndim; i++){
        attp->size *= dims[i];
    }

    buf = (char*)malloc(attp->size * nc_type_size(type));
    memset(buf, 0, attp->size * nc_type_size(type));
    err = ncmpi_put_att(fp->ncid, varid, attp->path, type, attp->size, buf); CHECK_ERRJ
    free(buf);
    buf = NULL;

    err = ncmpi_inq_attid(fp->ncid, varid, attp->path, &(attp->attid)); CHECK_ERRJ

    return (void *)attp;

errout:
    free(attp);
    if (buf != NULL){
        free(buf);
    }

    return NULL;
} /* end H5VL_ncmpi_attr_create() */


/*-------------------------------------------------------------------------
 * Function:    H5VL_ncmpi_attr_open
 *
 * Purpose:     Handles the attribute open callback
 *
 * Return:      Success:    attribute pointer
 *              Failure:    NULL
 *
 *-------------------------------------------------------------------------
 */
void *H5VL_ncmpi_attr_open(void *obj, const H5VL_loc_params_t *loc_params, const char *attr_name, 
                           hid_t aapl_id, hid_t dxpl_id, void **req) {
    int err;
    int i;
    int ndim;
    hsize_t *dims;
    int varid;
    nc_type type;
    char *buf = NULL;
    char path[PNC_VOL_MAX_NAME];
    char *ppath;
    H5VL_ncmpi_file_t *fp;
    H5VL_ncmpi_attr_t *attp;

    /* Check arguments */
    if((loc_params->obj_type != H5I_FILE) && (loc_params->obj_type != H5I_GROUP) && (loc_params->obj_type != H5I_DATASET))   RET_ERRN("container not a file or group or dataset")
    //if(loc_params->type != H5VL_OBJECT_BY_SELF) RET_ERRN("loc_params->type is not H5VL_OBJECT_BY_SELF")

    if (loc_params->type == H5VL_OBJECT_BY_SELF){
        if (loc_params->obj_type == H5I_FILE){
            fp = (H5VL_ncmpi_file_t*)obj;
            varid = NC_GLOBAL;
            ppath = NULL;
        }
        else if (loc_params->obj_type == H5I_GROUP){
            fp = ((H5VL_ncmpi_group_t*)obj)->fp;
            ppath = ((H5VL_ncmpi_group_t*)obj)->path;
            varid = NC_GLOBAL;
        }
        else{
            fp = ((H5VL_ncmpi_dataset_t*)obj)->fp;
            varid = ((H5VL_ncmpi_dataset_t*)obj)->varid;
            ppath = NULL;
        }
    }
    else if (loc_params->type == H5VL_OBJECT_BY_NAME){
        if (loc_params->obj_type == H5I_FILE){
            fp = (H5VL_ncmpi_file_t*)obj;
            ppath = NULL;
            varid = NC_GLOBAL;
        }
        else if (loc_params->obj_type == H5I_GROUP){
            fp = ((H5VL_ncmpi_group_t*)obj)->fp;
            ppath = ((H5VL_ncmpi_group_t*)obj)->path;
            varid = NC_GLOBAL;
        }
        else{
            RET_ERRN("loc_params->type is not H5VL_OBJECT_BY_SELF")
        }

        // Try variable
        if (ppath == NULL){
            sprintf(path, "%s", loc_params->loc_data.loc_by_name.name);
        }
        else{
            sprintf(path, "%s_%s", ppath, loc_params->loc_data.loc_by_name.name);
        }
        err = ncmpi_inq_varid(fp->ncid, path, &varid);
        if (err != NC_NOERR){ // If not, it must be group
            varid = NC_GLOBAL;

            // Try group
            if (ppath == NULL){
                sprintf(path, "_group_%s", loc_params->loc_data.loc_by_name.name);
            }
            else{
                sprintf(path, "_group_%s_%s", ppath, loc_params->loc_data.loc_by_name.name);
            }
            err = ncmpi_inq_attid(fp->ncid, varid, path, &i);
            if (err != NC_NOERR){   // Neither, something wrong
                RET_ERRN("Specified object name not found")
            }

            if (ppath == NULL){
                sprintf(path, "%s", loc_params->loc_data.loc_by_name.name);
            }
            else{
                sprintf(path, "%s_%s", ppath, loc_params->loc_data.loc_by_name.name);
            }
            ppath = path;
        }
        else{
            ppath = NULL;
        }
    }

    attp = (H5VL_ncmpi_attr_t*)malloc(sizeof(H5VL_ncmpi_attr_t));
    attp->objtype = H5I_ATTR;
    attp->acpl_id = -1;
    attp->aapl_id = aapl_id;
    attp->dxpl_id = dxpl_id;
    attp->varid = varid;
    attp->fp->ncid = fp->ncid;
    if (ppath == NULL){
        sprintf(attp->path, "%s", attr_name);
        attp->name = attp->path;
    }
    else{
        sprintf(attp->path, "%s_%s", ppath, attr_name);
        attp->name = attp->path + strlen(ppath) + 1;
    }

    err = ncmpi_inq_attid(fp->ncid, varid, attp->path, &(attp->attid)); CHECK_ERRJ
    err = ncmpi_inq_attlen(fp->ncid, varid, attp->path, &(attp->size)); CHECK_ERRJ
    err = ncmpi_inq_atttype(fp->ncid, varid, attp->path, &(attp->type)); CHECK_ERRJ

    return (void *)attp;

errout:
    free(attp);
    if (buf != NULL){
        free(buf);
    }

    return NULL;
} /* end H5VL_ncmpi_attr_open() */


/*-------------------------------------------------------------------------
 * Function:    H5VL_ncmpi_attr_read
 *
 * Purpose:     Handles the attribute read callback
 *
 * Return:      SUCCEED/FAIL
 *
 *-------------------------------------------------------------------------
 */
herr_t H5VL_ncmpi_attr_read(void *attr, hid_t dtype_id, void *buf,
                            hid_t dxpl_id, void **req) {
    int err;
    MPI_Datatype type;
    H5VL_ncmpi_attr_t *ap = (H5VL_ncmpi_attr_t*)attr;

    /* Check arguments */
    if(H5I_DATATYPE != H5Iget_type(dtype_id))    RET_ERR("invalid datatype ID")

    // Convert to MPI type
    type = h5t_to_mpi_type(dtype_id);
    if (type == MPI_DATATYPE_NULL) RET_ERR("memory type not supported")

    // Call PnetCDF
    if (type == MPI_CHAR) err = ncmpi_get_att_text(ap->fp->ncid, ap->varid, ap->path, (char*)buf); 
    else if (type == MPI_SHORT) err = ncmpi_get_att_short(ap->fp->ncid, ap->varid, ap->path, (short*)buf); 
    else if ((type == MPI_INT) || (type == MPI_LONG)) err = ncmpi_get_att_int(ap->fp->ncid, ap->varid, ap->path, (int*)buf);
    else if (type == MPI_LONG_LONG) err = ncmpi_get_att_longlong(ap->fp->ncid, ap->varid, ap->path, (long long*)buf); 
    else if (type == MPI_UNSIGNED_SHORT) err = ncmpi_get_att_ushort(ap->fp->ncid, ap->varid, ap->path, (unsigned short*)buf); 
    else if ((type == MPI_UNSIGNED) || (type == MPI_UNSIGNED_LONG)) err = ncmpi_get_att_uint(ap->fp->ncid, ap->varid, ap->path, (unsigned int*)buf); 
    else if (type == MPI_UNSIGNED_LONG_LONG) err = ncmpi_get_att_ulonglong(ap->fp->ncid, ap->varid, ap->path, (unsigned long long*)buf); 
    else if (type == MPI_FLOAT) err = ncmpi_get_att_float(ap->fp->ncid, ap->varid, ap->path, (float*)buf); 
    else if (type == MPI_DOUBLE) err = ncmpi_get_att_double(ap->fp->ncid, ap->varid, ap->path, (double*)buf); 
    else RET_ERR("memory type not supported")
    CHECK_ERR

    return 0;
} /* end H5VL_ncmpi_attr_read() */


/*-------------------------------------------------------------------------
 * Function:    H5VL_ncmpi_attr_write
 *
 * Purpose:     Handles the attribute write callback
 *
 * Return:      SUCCEED/FAIL
 *
 *-------------------------------------------------------------------------
 */
herr_t H5VL_ncmpi_attr_write(void *attr, hid_t dtype_id, const void *buf,
                                hid_t dxpl_id, void **req) {
    int err;
    nc_type type;
    H5VL_ncmpi_attr_t *ap = (H5VL_ncmpi_attr_t*)attr;

    /* Check arguments */
    if(H5I_DATATYPE != H5Iget_type(dtype_id))    RET_ERR("invalid datatype ID")

    // Convert to NC type
    type = h5t_to_mpi_type(dtype_id);
    if (type == MPI_DATATYPE_NULL) RET_ERR("memory type not supported")

    // Enter define mode
    err = enter_define_mode(ap->fp); CHECK_ERR

    // Call PnetCDF
    if (type == MPI_CHAR) err = ncmpi_put_att_text(ap->fp->ncid, ap->varid, ap->path, ap->size, (char*)buf); 
    else if (type == MPI_SHORT) err = ncmpi_put_att_short(ap->fp->ncid, ap->varid, ap->path, ap->type, ap->size, (short*)buf); 
    else if ((type == MPI_INT) || (type == MPI_LONG)) err = ncmpi_put_att_int(ap->fp->ncid, ap->varid, ap->path, ap->type, ap->size, (int*)buf);
    else if (type == MPI_LONG_LONG) err = ncmpi_put_att_longlong(ap->fp->ncid, ap->varid, ap->path, ap->type, ap->size, (long long*)buf); 
    else if (type == MPI_UNSIGNED_SHORT) err = ncmpi_put_att_ushort(ap->fp->ncid, ap->varid, ap->path, ap->type, ap->size, (unsigned short*)buf); 
    else if ((type == MPI_UNSIGNED) || (type == MPI_UNSIGNED_LONG)) err = ncmpi_put_att_uint(ap->fp->ncid, ap->varid, ap->path, ap->type, ap->size, (unsigned int*)buf); 
    else if (type == MPI_UNSIGNED_LONG_LONG) err = ncmpi_put_att_ulonglong(ap->fp->ncid, ap->varid, ap->path, ap->type, ap->size, (unsigned long long*)buf); 
    else if (type == MPI_FLOAT) err = ncmpi_put_att_float(ap->fp->ncid, ap->varid, ap->path, ap->type, ap->size, (float*)buf); 
    else if (type == MPI_DOUBLE) err = ncmpi_put_att_double(ap->fp->ncid, ap->varid, ap->path, ap->type, ap->size, (double*)buf); 
    else RET_ERR("memory type not supported")
    CHECK_ERR

    return 0;
} /* end H5VL_ncmpi_attr_write() */


/*-------------------------------------------------------------------------
 * Function:    H5VL_ncmpi_attr_get
 *
 * Purpose:     Handles the attribute get callback
 *
 * Return:      SUCCEED/FAIL
 *
 *-------------------------------------------------------------------------
 */
herr_t H5VL_ncmpi_attr_get( void *obj, H5VL_attr_get_t get_type,
                            hid_t dxpl_id, void **req, va_list arguments) {
    int err;

    switch(get_type) {
        /* H5Aget_space */
        case H5VL_ATTR_GET_SPACE:
            {
                MPI_Offset dlen;
                hsize_t dim;
                hid_t *ret_id = va_arg(arguments, hid_t*);
                H5VL_ncmpi_attr_t *attp = (H5VL_ncmpi_attr_t*)obj;

                // Get att size
                err = ncmpi_inq_attlen(attp->fp->ncid, attp->varid, attp->name, &dlen); CHECK_ERR
                dim = dlen;

                // NetCDF attribute is always 1-D
                *ret_id = H5Screate_simple(1, &dim, NULL);   

                break;
            }
        /* H5Aget_type */
        case H5VL_ATTR_GET_TYPE:
            {
                hid_t   *ret_id = va_arg(arguments, hid_t *);
                H5VL_ncmpi_attr_t *attp = (H5VL_ncmpi_attr_t*)obj;

                *ret_id = nc_to_h5t_type(attp->type);

                if ((*ret_id) < 0){
                    return -1;
                }

                break;
            }
        /* H5Aget_create_plist */
        case H5VL_ATTR_GET_ACPL:
            {
                hid_t   *ret_id = va_arg(arguments, hid_t *);
                H5VL_ncmpi_attr_t *attp = (H5VL_ncmpi_attr_t*)obj;
                
                *ret_id = attp->acpl_id;
            
                break;
            }
        /* H5Aget_name */
        case H5VL_ATTR_GET_NAME:
            {
                const H5VL_loc_params_t *loc_params = va_arg(arguments, const H5VL_loc_params_t *);
                size_t  buf_size = va_arg(arguments, size_t);
                char    *buf = va_arg(arguments, char *);
                ssize_t *ret_val = va_arg(arguments, ssize_t *);
                int i;

                if(loc_params->type == H5VL_OBJECT_BY_SELF) {
                    H5VL_ncmpi_attr_t *attp = (H5VL_ncmpi_attr_t*)obj;

                    // Name can be changed, we need to update it
                    err = ncmpi_inq_attname(attp->fp->ncid, attp->varid, attp->attid, attp->path);
                    for(i = strlen(attp->path); i > -1; i--){
                        if (attp->path[i] == '_'){
                            break;
                        }
                    }
                    attp->name = attp->path + i + 1;

                    *ret_val = strlen(attp->name);

                    if (buf_size > (*ret_val)){
                        strcpy(buf, attp->name);
                    }
                }
                else if(loc_params->type == H5VL_OBJECT_BY_IDX) {
                    RET_ERR("loc_params type not supported")
                }
                else{
                    RET_ERR("loc_params type not supported")
                }

                break;
            }
        /* H5Aget_info */
        case H5VL_ATTR_GET_INFO:
            {
                MPI_Offset dlen;
                const H5VL_loc_params_t *loc_params = va_arg(arguments, const H5VL_loc_params_t *);
                H5A_info_t   *ainfo = va_arg(arguments, H5A_info_t *);
                H5VL_ncmpi_attr_t   *attr = NULL;

                if(loc_params->type == H5VL_OBJECT_BY_SELF) {
                    H5VL_ncmpi_attr_t *attp = (H5VL_ncmpi_attr_t*)obj;

                    err = ncmpi_inq_attlen(attp->fp->ncid, attp->varid, attp->name, &dlen); CHECK_ERR

                    ainfo->data_size = dlen; 
                }
                else if(loc_params->type == H5VL_OBJECT_BY_IDX) {
                    RET_ERR("loc_params type not supported")
                }
                else if(loc_params->type == H5VL_OBJECT_BY_NAME) {
                    int varid;
                    char *attr_name = va_arg(arguments, char *);
                    char *ppath;
                    char path[PNC_VOL_MAX_NAME];
                    H5VL_ncmpi_file_t *fp;

                    if (loc_params->obj_type == H5I_FILE){
                        fp = (H5VL_ncmpi_file_t*)obj;
                        ppath = NULL;
                        varid = NC_GLOBAL;
                    }
                    else if (loc_params->obj_type == H5I_GROUP){
                        fp = ((H5VL_ncmpi_group_t*)obj)->fp;
                        ppath = ((H5VL_ncmpi_group_t*)obj)->path;
                        varid = NC_GLOBAL;
                    }
                    else{
                        RET_ERR("loc_params->type is not H5VL_OBJECT_BY_SELF")
                    }

                    // Try variable
                    if (ppath == NULL){
                        sprintf(path, "%s", loc_params->loc_data.loc_by_name.name);
                    }
                    else{
                        sprintf(path, "%s_%s", ppath, loc_params->loc_data.loc_by_name.name);
                    }
                    err = ncmpi_inq_varid(fp->ncid, path, &varid);
                    if (err != NC_NOERR){ // If not, it must be group
                        int i;

                        varid = NC_GLOBAL;

                        // Try group
                        if (ppath == NULL){
                            sprintf(path, "_group_%s", loc_params->loc_data.loc_by_name.name);
                        }
                        else{
                            sprintf(path, "_group_%s_%s", ppath, loc_params->loc_data.loc_by_name.name);
                        }
                        err = ncmpi_inq_attid(fp->ncid, varid, path, &i);
                        if (err != NC_NOERR){   // Neither, something wrong
                            RET_ERR("Specified object name not found")
                        }

                        if (ppath == NULL){
                            sprintf(path, "%s_%s", loc_params->loc_data.loc_by_name.name, attr_name);
                        }
                        else{
                            sprintf(path, "%s_%s_%s", ppath, loc_params->loc_data.loc_by_name.name, attr_name);
                        }
                    }
                    else{
                        sprintf(path, "%s", attr_name);
                    }

                    err = ncmpi_inq_attlen(fp->ncid, varid, path, &dlen); CHECK_ERR

                    ainfo->data_size = dlen; 
                }
                else{
                    RET_ERR("loc_params type not supported")
                }
                
                break;
            }

        case H5VL_ATTR_GET_STORAGE_SIZE:
            {
                MPI_Offset dlen;
                hsize_t *ret = va_arg(arguments, hsize_t *);
                H5VL_ncmpi_attr_t *attp = (H5VL_ncmpi_attr_t*)obj;

                err = ncmpi_inq_attlen(attp->fp->ncid, attp->varid, attp->name, &dlen); CHECK_ERR

                *ret = dlen * nc_type_size(attp->type); 
              
                break;
            }

        default:
            RET_ERR("get_type not supported")
    } /* end switch */

    return 0;
} /* end H5VL_ncmpi_attr_get() */


/*-------------------------------------------------------------------------
 * Function:    H5VL_ncmpi_attr_specific
 *
 * Purpose:     Handles the attribute specific callback
 *
 * Return:      SUCCEED/FAIL
 *
 *-------------------------------------------------------------------------
 */
herr_t H5VL_ncmpi_attr_specific(    void *obj, const H5VL_loc_params_t *loc_params, H5VL_attr_specific_t specific_type, 
                                    hid_t dxpl_id, void **req, va_list arguments) {
    int err;
    int varid;
    char path[PNC_VOL_MAX_NAME];
    char *ppath;
    H5VL_ncmpi_attr_t *ap = NULL;
    H5VL_ncmpi_file_t *fp = NULL;

    if (loc_params->type == H5VL_OBJECT_BY_SELF){
        if (loc_params->obj_type == H5I_FILE){
            fp = (H5VL_ncmpi_file_t*)obj;
            varid = NC_GLOBAL;
            sprintf(path, "");
        }
        else if (loc_params->obj_type == H5I_GROUP){
            fp = ((H5VL_ncmpi_group_t*)obj)->fp;
            ppath = ((H5VL_ncmpi_group_t*)obj)->path;
            sprintf(path, "%s_", ppath);
        }
        else{
            fp = ((H5VL_ncmpi_dataset_t*)obj)->fp;
            varid = ((H5VL_ncmpi_dataset_t*)obj)->varid;
            sprintf(path, "");
        }
    }
    else if (loc_params->type == H5VL_OBJECT_BY_NAME){
        if (loc_params->obj_type == H5I_FILE){
            fp = (H5VL_ncmpi_file_t*)obj;
            ppath = NULL;
            varid = NC_GLOBAL;
        }
        else if (loc_params->obj_type == H5I_GROUP){
            fp = ((H5VL_ncmpi_group_t*)obj)->fp;
            ppath = ((H5VL_ncmpi_group_t*)obj)->path;
            varid = NC_GLOBAL;
        }
        else{
            RET_ERR("loc_params->type is not H5VL_OBJECT_BY_SELF")
        }

        // Try variable
        if (ppath == NULL){
            sprintf(path, "%s", loc_params->loc_data.loc_by_name.name);
        }
        else{
            sprintf(path, "%s_%s", ppath, loc_params->loc_data.loc_by_name.name);
        }
        err = ncmpi_inq_varid(fp->ncid, path, &varid);
        if (err != NC_NOERR){ // If not, it must be group
            int i;
            
            varid = NC_GLOBAL;

            // Try group
            if (ppath == NULL){
                sprintf(path, "_group_%s", loc_params->loc_data.loc_by_name.name);
            }
            else{
                sprintf(path, "_group_%s_%s", ppath, loc_params->loc_data.loc_by_name.name);
            }
            err = ncmpi_inq_attid(fp->ncid, varid, path, &i);
            if (err != NC_NOERR){   // Neither, something wrong
                RET_ERR("Specified object name not found")
            }

            if (ppath == NULL){
                sprintf(path, "%s_", loc_params->loc_data.loc_by_name.name);
            }
            else{
                sprintf(path, "%s_%s_", ppath, loc_params->loc_data.loc_by_name.name);
            }
        }
        else{
            sprintf(path, "");
        }
    }

    /* Check arguments */
    switch(specific_type) {
        case H5VL_ATTR_DELETE:
            {
                char    *attr_name = va_arg(arguments, char *);
                
                sprintf(path + strlen(path), "%s", attr_name);
                err = ncmpi_del_att(fp->ncid, varid, path); CHECK_ERR

                break;
            }
        case H5VL_ATTR_EXISTS:
            {
                const char *attr_name = va_arg(arguments, const char *);
                htri_t  *ret = va_arg(arguments, htri_t *);
                int attid;
                
                sprintf(path + strlen(path), "%s", attr_name);
                *ret = 1;
                err = ncmpi_inq_attid(fp->ncid, varid, path, &attid);
                if (err == NC_ENOTATT){
                    err = NC_NOERR;
                    *ret = 0;
                }
                CHECK_ERR

                break;
            }

        case H5VL_ATTR_ITER:
            {
               RET_ERR("specific_type not supported")

               break;
            }
        /* H5Arename/rename_by_name */
        case H5VL_ATTR_RENAME:
            {
                const char *old_name  = va_arg(arguments, const char *);
                const char *new_name  = va_arg(arguments, const char *);
                char newpath[PNC_VOL_MAX_NAME];

                sprintf(newpath, "%s", path);
                sprintf(path + strlen(path), "%s", old_name);
                sprintf(newpath + strlen(newpath), "%s", new_name);

                err = enter_define_mode(fp); CHECK_ERR
                err = ncmpi_rename_att(fp->ncid, varid, path, newpath); CHECK_ERR

                break;
            }
        default:
            RET_ERR("specific_type not supported")
    } /* end switch */

    return 0;
} /* end H5VL_ncmpi_attr_specific() */


/*-------------------------------------------------------------------------
 * Function:    H5VL_ncmpi_attr_optional
 *
 * Purpose:     Handles the attribute optional callback
 *
 * Return:      SUCCEED/FAIL
 *
 *-------------------------------------------------------------------------
 */
herr_t H5VL_ncmpi_attr_optional(void *obj, H5VL_file_optional_t opt_type, hid_t dxpl_id, void **req, va_list arguments) {
    herr_t ret_value = 0;    /* Return value */

    return 0;
} /* end H5VL_ncmpi_attr_optional() */


/*-------------------------------------------------------------------------
 * Function:    H5VL_ncmpi_attr_close
 *
 * Purpose:     Handles the attribute close callback
 *
 * Return:      Success:    SUCCEED
 *              Failure:    FAIL (attribute will not be closed)
 *
 *-------------------------------------------------------------------------
 */
herr_t H5VL_ncmpi_attr_close(void *attr, hid_t dxpl_id, void **req) {
    H5VL_ncmpi_attr_t *ap = (H5VL_ncmpi_attr_t*)attr;

    free(ap);

    return 0;
} /* end H5VL_ncmpi_attr_close() */

