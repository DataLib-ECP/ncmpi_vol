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
H5_DLL herr_t H5VL_ncmpi_attr_optional(void *obj, hid_t dxpl_id, void **req, va_list arguments);
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
    char tmp[1024];
    char *ppath;
    H5VL_ncmpi_attr_t *attp;
    H5VL_ncmpi_dataset_t *dp;
    H5VL_ncmpi_group_t *gp;
    H5VL_ncmpi_file_t *fp;

    /* Check arguments */
    if((loc_params->obj_type != H5I_FILE) && (loc_params->obj_type != H5I_GROUP) && (loc_params->obj_type != H5I_DATASET))   RET_ERRN("container not a file or group or dataset")
    if(loc_params->type != H5VL_OBJECT_BY_SELF) RET_ERRN("loc_params->type is not H5VL_OBJECT_BY_SELF")
    if(H5I_DATATYPE != H5Iget_type(type_id))    RET_ERRN("invalid datatype ID")
    if(H5I_DATASPACE != H5Iget_type(space_id))   RET_ERRN("invalid dataspace ID")

    if (loc_params->obj_type == H5I_FILE){
        fp = (H5VL_ncmpi_file_t*)obj;
        gp = NULL;
        dp = NULL;
        ppath = NULL;
        varid = NC_GLOBAL;
    }
    else if (loc_params->obj_type == H5I_GROUP){
        gp = (H5VL_ncmpi_group_t*)obj;
        fp = gp->fp;
        dp = NULL;
        ppath = gp->path;
        varid = NC_GLOBAL;
    }
    else{
        dp = (H5VL_ncmpi_dataset_t*)obj;
        fp = dp->fp;
        gp = dp->gp;
        ppath = NULL;
        varid = dp->varid;
    }

    // Convert to NC type
    type = h5t_to_nc_type(type_id);
    if (type == NC_NAT) RET_ERRN("only native type is supported")

    // Enter define mode
    err = enter_define_mode(fp); CHECK_ERRN

    attp = (H5VL_ncmpi_attr_t*)malloc(sizeof(H5VL_ncmpi_attr_t));
    attp->objtype = H5I_ATTR;
    attp->acpl_id = acpl_id;
    attp->aapl_id = aapl_id;
    attp->dxpl_id = dxpl_id;
    attp->varid = varid;
    attp->type = type;
    attp->fp = fp;
    attp->gp = gp;
    attp->dp = dp;
    if (ppath == NULL){
        attp->path = (char*)malloc(strlen(attr_name) + 1);
        sprintf(attp->path, "%s", attr_name);
        attp->name = attp->path;
    }
    else{
        attp->path = (char*)malloc(strlen(ppath) + strlen(attr_name) + 2);
        sprintf(attp->path, "%s_%s", ppath, attr_name);
        attp->name = attp->path + strlen(ppath) + 1;
    }

    ndim = H5Sget_simple_extent_ndims(space_id);
    if (ndim < 0)   RET_ERRN("ndim < 0")

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
    free(attp->path);
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
    char tmp[1024];
    char *ppath;
    H5VL_ncmpi_attr_t *attp;
    H5VL_ncmpi_dataset_t *dp;
    H5VL_ncmpi_group_t *gp;
    H5VL_ncmpi_file_t *fp;

    /* Check arguments */
    if((loc_params->obj_type != H5I_FILE) && (loc_params->obj_type != H5I_GROUP) && (loc_params->obj_type != H5I_DATASET))   RET_ERRN("container not a file or group or dataset")
    if(loc_params->type != H5VL_OBJECT_BY_SELF) RET_ERRN("loc_params->type is not H5VL_OBJECT_BY_SELF")

    if (loc_params->obj_type == H5I_FILE){
        fp = (H5VL_ncmpi_file_t*)obj;
        gp = NULL;
        dp = NULL;
        ppath = NULL;
        varid = NC_GLOBAL;
    }
    else if (loc_params->obj_type == H5I_GROUP){
        gp = (H5VL_ncmpi_group_t*)obj;
        fp = gp->fp;
        dp = NULL;
        ppath = gp->path;
        varid = NC_GLOBAL;
    }
    else{
        dp = (H5VL_ncmpi_dataset_t*)dp;
        fp = dp->fp;
        gp = dp->gp;
        ppath = NULL;
        varid = dp->varid;
    }

    attp = (H5VL_ncmpi_attr_t*)malloc(sizeof(H5VL_ncmpi_attr_t));
    attp->objtype = H5I_ATTR;
    attp->acpl_id = -1;
    attp->aapl_id = aapl_id;
    attp->dxpl_id = dxpl_id;
    attp->varid = varid;
    attp->fp = fp;
    attp->gp = gp;
    attp->dp = dp;
    if (ppath == NULL){
        attp->path = (char*)malloc(strlen(attr_name) + 1);
        sprintf(attp->path, "%s", attr_name);
        attp->name = attp->path;
    }
    else{
        attp->path = (char*)malloc(strlen(ppath) + strlen(attr_name) + 2);
        sprintf(attp->path, "%s_%s", ppath, attr_name);
        attp->name = attp->path + strlen(ppath) + 1;
    }

    err = ncmpi_inq_attid(fp->ncid, varid, attp->path, &(attp->attid)); CHECK_ERRJ
    err = ncmpi_inq_attlen(fp->ncid, varid, attp->path, &(attp->size)); CHECK_ERRJ
    err = ncmpi_inq_atttype(fp->ncid, varid, attp->path, &(attp->type)); CHECK_ERRJ

    return (void *)attp;

errout:
    free(attp->path);
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
    nc_type type;
    H5VL_ncmpi_attr_t *ap = (H5VL_ncmpi_attr_t*)attr;

    /* Check arguments */
    if(H5I_DATATYPE != H5Iget_type(dtype_id))    RET_ERR("invalid datatype ID")

    // Convert to NC type
    type = h5t_to_nc_type(dtype_id);
    if (type == NC_NAT) RET_ERR("only native type is supported")

    // Call PnetCDF
    if (type == NC_CHAR) err = ncmpi_get_att_text(ap->fp->ncid, ap->varid, ap->path, (char*)buf); 
    else if (type == NC_SHORT) err = ncmpi_get_att_short(ap->fp->ncid, ap->varid, ap->path, (short*)buf); 
    else if (type == NC_INT) err = ncmpi_get_att_int(ap->fp->ncid, ap->varid, ap->path, (int*)buf);
    else if (type == NC_INT64) err = ncmpi_get_att_longlong(ap->fp->ncid, ap->varid, ap->path, (long long*)buf); 
    else if (type == NC_USHORT) err = ncmpi_get_att_ushort(ap->fp->ncid, ap->varid, ap->path, (unsigned short*)buf); 
    else if (type == NC_UINT) err = ncmpi_get_att_uint(ap->fp->ncid, ap->varid, ap->path, (unsigned int*)buf); 
    else if (type == NC_UINT64) err = ncmpi_get_att_ulonglong(ap->fp->ncid, ap->varid, ap->path, (unsigned long long*)buf); 
    else if (type == NC_FLOAT) err = ncmpi_get_att_float(ap->fp->ncid, ap->varid, ap->path, (float*)buf); 
    else if (type == NC_DOUBLE) err = ncmpi_get_att_double(ap->fp->ncid, ap->varid, ap->path, (double*)buf); 
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
    type = h5t_to_nc_type(dtype_id);
    if (type == NC_NAT) RET_ERR("only native type is supported")

    // Enter define mode
    err = enter_define_mode(ap->fp); CHECK_ERR

    // Call PnetCDF
    if (type == NC_CHAR) err = ncmpi_put_att_text(ap->fp->ncid, ap->varid, ap->path, ap->size, (char*)buf); 
    else if (type == NC_SHORT) err = ncmpi_put_att_short(ap->fp->ncid, ap->varid, ap->path, ap->type, ap->size, (short*)buf); 
    else if (type == NC_INT) err = ncmpi_put_att_int(ap->fp->ncid, ap->varid, ap->path, ap->type, ap->size, (int*)buf);
    else if (type == NC_INT64) err = ncmpi_put_att_longlong(ap->fp->ncid, ap->varid, ap->path, ap->type, ap->size, (long long*)buf); 
    else if (type == NC_USHORT) err = ncmpi_put_att_ushort(ap->fp->ncid, ap->varid, ap->path, ap->type, ap->size, (unsigned short*)buf); 
    else if (type == NC_UINT) err = ncmpi_put_att_uint(ap->fp->ncid, ap->varid, ap->path, ap->type, ap->size, (unsigned int*)buf); 
    else if (type == NC_UINT64) err = ncmpi_put_att_ulonglong(ap->fp->ncid, ap->varid, ap->path, ap->type, ap->size, (unsigned long long*)buf); 
    else if (type == NC_FLOAT) err = ncmpi_put_att_float(ap->fp->ncid, ap->varid, ap->path, ap->type, ap->size, (float*)buf); 
    else if (type == NC_DOUBLE) err = ncmpi_put_att_double(ap->fp->ncid, ap->varid, ap->path, ap->type, ap->size, (double*)buf); 
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
    herr_t ret_value = 0;

    switch(get_type) {
        /* H5Aget_space */
        case H5VL_ATTR_GET_SPACE:
            {
                hid_t   *ret_id = va_arg(arguments, hid_t *);
                H5VL_ncmpi_attr_t   *attr = (H5VL_ncmpi_attr_t *)obj;
              
                break;
            }
        /* H5Aget_type */
        case H5VL_ATTR_GET_TYPE:
            {
                hid_t   *ret_id = va_arg(arguments, hid_t *);
                H5VL_ncmpi_attr_t   *attr = (H5VL_ncmpi_attr_t *)obj;
               
                break;
            }
        /* H5Aget_create_plist */
        case H5VL_ATTR_GET_ACPL:
            {
                hid_t   *ret_id = va_arg(arguments, hid_t *);
                H5VL_ncmpi_attr_t   *attr = (H5VL_ncmpi_attr_t *)obj;
            
                break;
            }
        /* H5Aget_name */
        case H5VL_ATTR_GET_NAME:
            {
                const H5VL_loc_params_t *loc_params = va_arg(arguments, const H5VL_loc_params_t *);
                size_t  buf_size = va_arg(arguments, size_t);
                char    *buf = va_arg(arguments, char *);
                ssize_t *ret_val = va_arg(arguments, ssize_t *);
                H5VL_ncmpi_attr_t   *attr = NULL;

                break;
            }
        /* H5Aget_info */
        case H5VL_ATTR_GET_INFO:
            {
                const H5VL_loc_params_t *loc_params = va_arg(arguments, const H5VL_loc_params_t *);
                H5A_info_t   *ainfo = va_arg(arguments, H5A_info_t *);
                H5VL_ncmpi_attr_t   *attr = NULL;

                break;
            }

        case H5VL_ATTR_GET_STORAGE_SIZE:
            {
                hsize_t *ret = va_arg(arguments, hsize_t *);
                H5VL_ncmpi_attr_t   *attr = (H5VL_ncmpi_attr_t *)obj;

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
    herr_t      ret_value = 0;    /* Return value */
    
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
herr_t H5VL_ncmpi_attr_optional(void *obj, hid_t dxpl_id, void **req, va_list arguments) {
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

    free(ap->path);
    free(ap);

    return 0;
} /* end H5VL_ncmpi_attr_close() */

