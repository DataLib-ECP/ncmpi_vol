#include "ncmpi_vol.h"
#include "pnetcdf.h"

/********************* */
/* Function prototypes */
/********************* */
void* H5VL_ncmpi_object_open(void *obj, const H5VL_loc_params_t *loc_params, H5I_type_t *opened_type, hid_t dxpl_id, void **req);
herr_t H5VL_ncmpi_object_copy(void *src_obj, const H5VL_loc_params_t *loc_params1, const char *src_name, void *dst_obj, const H5VL_loc_params_t *loc_params2, const char *dst_name, hid_t ocpypl_id, hid_t lcpl_id, hid_t dxpl_id, void **req);
herr_t H5VL_ncmpi_object_get( void *obj, const H5VL_loc_params_t *loc_params, H5VL_object_get_t get_type, hid_t dxpl_id, void **req, va_list arguments);
herr_t H5VL_ncmpi_object_specific(void *obj, const H5VL_loc_params_t *loc_params, H5VL_object_specific_t specific_type, hid_t dxpl_id, void **req, va_list arguments);
herr_t H5VL_ncmpi_object_optional(void *obj, hid_t dxpl_id, void **req, va_list arguments);

const H5VL_object_class_t H5VL_ncmpi_object_g{
    H5VL_ncmpi_object_open,                  /* open         */
    H5VL_ncmpi_object_copy,                  /* read         */
    H5VL_ncmpi_object_get,                   /* get          */
    H5VL_ncmpi_object_specific,              /* specific     */
    H5VL_ncmpi_object_optional               /* optional     */
};

extern void *H5VL_ncmpi_group_open(void *obj, const H5VL_loc_params_t *loc_params, const char *name, hid_t  gapl_id, hid_t dxpl_id, void **req);
extern void* H5VL_ncmpi_dataset_open(void *obj, const H5VL_loc_params_t *loc_params, const char *name, hid_t dapl_id, hid_t dxpl_id, void **req);
extern void *H5VL_ncmpi_group_open(void *obj, const H5VL_loc_params_t *loc_params, const char *name, hid_t  gapl_id, hid_t dxpl_id, void **req);
extern void *H5VL_ncmpi_attr_open(void *obj, const H5VL_loc_params_t *loc_params, const char *attr_name, hid_t aapl_id, hid_t dxpl_id, void **req);

/*-------------------------------------------------------------------------
 * Function:    H5VL_ncmpi_object_open
 *
 * Purpose:     Handles the object open callback
 *
 * Return:      Success:    object pointer
 *              Failure:    NULL
 *
 *-------------------------------------------------------------------------
 */
void* H5VL_ncmpi_object_open(void *obj, const H5VL_loc_params_t *loc_params, H5I_type_t *opened_type, hid_t dxpl_id, void **req) {
    int err;
    int id;
    void *ret_value = NULL;
    char tmp[PNC_VOL_MAX_NAME];
    H5VL_ncmpi_file_t *fp = (H5VL_ncmpi_file_t*)obj;

    if(loc_params->obj_type != H5I_FILE)   RET_ERRN("container not a file")

    switch(loc_params->type) {
        case H5VL_OBJECT_BY_NAME:
            {
                // Try group
                sprintf(tmp, "_group_%s", loc_params->loc_data.loc_by_name.name);
                err = ncmpi_inq_attid(fp->ncid, NC_GLOBAL, tmp, &id);
                if (err == NC_NOERR){
                    ret_value = H5VL_ncmpi_group_open(obj, loc_params, loc_params->loc_data.loc_by_name.name, H5P_DEFAULT, dxpl_id, req);
                    break;
                }

                // Try variable
                err = ncmpi_inq_varid(fp->ncid, loc_params->loc_data.loc_by_name.name, &id);
                if (err == NC_NOERR){
                    ret_value = H5VL_ncmpi_dataset_open(obj, loc_params, loc_params->loc_data.loc_by_name.name, H5P_DEFAULT, dxpl_id, req);
                    break;
                }

                // Try attribute
                err = ncmpi_inq_attid(fp->ncid, NC_GLOBAL, loc_params->loc_data.loc_by_name.name, &id);
                if (err == NC_NOERR){
                    ret_value = H5VL_ncmpi_attr_open(obj, loc_params, loc_params->loc_data.loc_by_name.name, H5P_DEFAULT, dxpl_id, req);
                    break;
                }

                break;
            }
        default:
            RET_ERRN("Unsupported loc_params->type")
    } /* end switch */

    return ret_value;
} /* end H5VL_ncmpi_object_open() */


/*-------------------------------------------------------------------------
 * Function:    H5VL_ncmpi_object_copy
 *
 * Purpose:     Handles the object copy callback
 *
 * Return:      SUCCEED/FAIL
 *
 *-------------------------------------------------------------------------
 */
herr_t H5VL_ncmpi_object_copy(void *src_obj, const H5VL_loc_params_t *loc_params1, const char *src_name, 
                                void *dst_obj, const H5VL_loc_params_t *loc_params2, const char *dst_name, 
                                hid_t ocpypl_id, hid_t lcpl_id, hid_t dxpl_id, void **req){
    int err;
    int id;
    int vid1, vid2;
    void *ret_value = NULL;
    char spath[PNC_VOL_MAX_NAME];
    char dpath[PNC_VOL_MAX_NAME];
    char tmp[PNC_VOL_MAX_NAME];
    char tmp2[PNC_VOL_MAX_NAME];
    H5VL_ncmpi_file_t *fps = (H5VL_ncmpi_file_t*)src_obj;
    H5VL_ncmpi_file_t *fpd = (H5VL_ncmpi_file_t*)dst_obj;

    if(loc_params1->obj_type != H5I_FILE)   RET_ERR("source container not a file")
    if(loc_params2->obj_type != H5I_FILE)   RET_ERR("dest container not a file")

    switch(loc_params1->type) {
        case H5VL_OBJECT_BY_NAME:
            {
                // Try group
                sprintf(spath, "_group_%s", loc_params1->loc_data.loc_by_name.name);
                err = ncmpi_inq_attid(fps->ncid, NC_GLOBAL, spath, &id);
                if (err == NC_NOERR){
                    vid1 = NC_GLOBAL;
                    sprintf(spath, "%s_%s", loc_params1->loc_data.loc_by_name.name, src_name);
                    break;
                }

                // Try variable
                err = ncmpi_inq_varid(fps->ncid, loc_params1->loc_data.loc_by_name.name, &vid1);
                if (err == NC_NOERR){
                    sprintf(spath, "%s", src_name);
                    break;
                }
                break;
            }
        case H5VL_OBJECT_BY_SELF:
            {
                vid1 = NC_GLOBAL;
                sprintf(spath, "%s", src_name);
                break;
            }
        default:
            RET_ERR("Unsupported loc_params1->type")
    } /* end switch */

    switch(loc_params2->type) {
        case H5VL_OBJECT_BY_NAME:
            {
                // Try group
                sprintf(dpath, "_group_%s", loc_params2->loc_data.loc_by_name.name);
                err = ncmpi_inq_attid(fpd->ncid, NC_GLOBAL, dpath, &id);
                if (err == NC_NOERR){
                    vid2 = NC_GLOBAL;
                    sprintf(dpath, "%s_%s", loc_params2->loc_data.loc_by_name.name, dst_name);
                    break;
                }

                // Try variable
                err = ncmpi_inq_varid(fpd->ncid, loc_params2->loc_data.loc_by_name.name, &vid2);
                if (err == NC_NOERR){
                    sprintf(dpath, "%s", dst_name);
                    break;
                }
                break;
            }
        case H5VL_OBJECT_BY_SELF:
            {
                vid2 = NC_GLOBAL;
                sprintf(dpath, "%s", dst_name);
                break;
            }
        default:
            RET_ERR("Unsupported loc_params1->type")
    } /* end switch */

    // Try attr
    err = ncmpi_inq_attid(fps->ncid, vid1, spath, &id);
    if (err == NC_NOERR){
        err = ncmpi_copy_att (fps->ncid, vid1, spath, fpd->ncid, vid2); CHECK_ERR
        err = ncmpi_rename_att(fpd->ncid, vid2, spath, dpath);  CHECK_ERR
        return 0;
    }

    // Try group
    sprintf(tmp, "_group_%s", spath);
    err = ncmpi_inq_attid(fps->ncid, NC_GLOBAL, tmp, &id);
    if (err == NC_NOERR){
        err = ncmpi_copy_att (fps->ncid, NC_GLOBAL, tmp, fpd->ncid, NC_GLOBAL); CHECK_ERR
        sprintf(tmp2, "_group_%s", spath);
        err = ncmpi_rename_att(fpd->ncid, NC_GLOBAL, tmp, tmp2);  CHECK_ERR
        return 0;
    }

    // Try var
    err = ncmpi_inq_varid(fps->ncid, spath, &id);
    if (err == NC_NOERR){
        RET_ERR("Variable copy is not supported")
    }

    return 0;
} /* end H5VL_ncmpi_object_copy() */


/*-------------------------------------------------------------------------
 * Function:    H5VL_ncmpi_object_get
 *
 * Purpose:     Handles the object get callback
 *
 * Return:      SUCCEED/FAIL
 *
 *-------------------------------------------------------------------------
 */
herr_t H5VL_ncmpi_object_get( void *obj, const H5VL_loc_params_t *loc_params, H5VL_object_get_t get_type, 
                                hid_t dxpl_id, void **req, va_list arguments) {
    return 0;
} /* end H5VL_ncmpi_object_get() */


/*-------------------------------------------------------------------------
 * Function:    H5VL_ncmpi_object_specific
 *
 * Purpose:     Handles the object specific callback
 *
 * Return:      SUCCEED/FAIL
 *
 *-------------------------------------------------------------------------
 */
herr_t H5VL_ncmpi_object_specific(void *obj, const H5VL_loc_params_t *loc_params, H5VL_object_specific_t specific_type, 
                                    hid_t dxpl_id, void **req, va_list arguments) {
    return 0;
} /* end H5VL_ncmpi_object_specific() */


/*-------------------------------------------------------------------------
 * Function:    H5VL_ncmpi_object_optional
 *
 * Purpose:     Handles the object optional callback
 *
 * Return:      SUCCEED/FAIL
 *
 *-------------------------------------------------------------------------
 */
herr_t H5VL_ncmpi_object_optional(void *obj, hid_t dxpl_id,
                                    void **req, va_list arguments) {
    return 0;
} /* end H5VL_ncmpi_object_optional() */

