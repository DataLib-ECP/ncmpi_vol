#include "ncmpi_vol.h"
#include "pnetcdf.h"

/********************* */
/* Function prototypes */
/********************* */

void *H5VL_ncmpi_group_create(void *obj, const H5VL_loc_params_t *loc_params, const char *name, hid_t lcpl_id, hid_t gcpl_id, hid_t gapl_id, hid_t dxpl_id, void **req);
void *H5VL_ncmpi_group_open(void *obj, const H5VL_loc_params_t *loc_params, const char *name, hid_t gapl_id, hid_t dxpl_id, void **req);
herr_t H5VL_ncmpi_group_get(void *obj, H5VL_group_get_t get_type, hid_t dxpl_id, void **req, va_list arguments);
herr_t H5VL_ncmpi_group_specific(void *obj, H5VL_group_specific_t specific_type, hid_t dxpl_id, void **req, va_list arguments);
herr_t H5VL_ncmpi_group_optional(void *obj, H5VL_file_optional_t opt_type, hid_t dxpl_id, void **req, va_list arguments);
herr_t H5VL_ncmpi_group_close(void *grp, hid_t dxpl_id, void **req);

const H5VL_group_class_t H5VL_ncmpi_group_g{
    H5VL_ncmpi_group_create,                /* create       */
    H5VL_ncmpi_group_open,                  /* open       */
    H5VL_ncmpi_group_get,                   /* get          */
    H5VL_ncmpi_group_specific,              /* specific     */
    H5VL_ncmpi_group_optional,              /* optional     */
    H5VL_ncmpi_group_close                  /* close        */
};

/*-------------------------------------------------------------------------
 * Function:    H5VL_ncmpi_group_create
 *
 * Purpose:     Handles the group create callback
 *
 * Return:      Success:    group pointer
 *              Failure:    NULL
 *
 *-------------------------------------------------------------------------
 */
void *H5VL_ncmpi_group_create(  void *obj, const H5VL_loc_params_t *loc_params, const char *name,
                                hid_t lcpl_id, hid_t gcpl_id, hid_t  gapl_id,
                                hid_t  dxpl_id, void  **req) {
    int err;
    int i;
    char tmp[PNC_VOL_MAX_NAME];
    char *ppath;
    H5VL_ncmpi_group_t *grup;
    H5VL_ncmpi_file_t *fp;

    /* Check arguments */
    if((loc_params->obj_type != H5I_FILE) && (loc_params->obj_type != H5I_GROUP))   RET_ERRN("container not a file or group")
    if(loc_params->type != H5VL_OBJECT_BY_SELF) RET_ERRN("loc_params->type is not H5VL_OBJECT_BY_SELF")

    if (loc_params->obj_type == H5I_FILE){
        fp = (H5VL_ncmpi_file_t*)obj;
        ppath = NULL;
    }
    else {
        fp = ((H5VL_ncmpi_group_t*)obj)->fp;
        ppath = ((H5VL_ncmpi_group_t*)obj)->path;
    }

    // Enter define mode
    err = enter_define_mode(fp); CHECK_ERRN

    grup = (H5VL_ncmpi_group_t*)malloc(sizeof(H5VL_ncmpi_group_t));
    grup->objtype = H5I_GROUP;
    grup->lcpl_id = lcpl_id;
    grup->gcpl_id = gcpl_id;
    grup->gapl_id = gapl_id;
    grup->dxpl_id = dxpl_id;
    grup->fp = fp;
    if (ppath == NULL){
        grup->path = (char*)malloc(strlen(name) + 1);
        sprintf(grup->path, "%s", name);
        grup->name = grup->path;
    }
    else{
        grup->path = (char*)malloc(strlen(ppath) + strlen(name) + 2);
        sprintf(grup->path, "%s_%s", ppath, name);
        grup->name = grup->path + strlen(ppath) + 1;
    }
    sprintf(tmp, "_group_%s", grup->path);
    i = 0;
    err = ncmpi_put_att(fp->ncid, NC_GLOBAL, tmp, NC_INT, 1, &i); CHECK_ERRJ

    return (void *)grup;

errout:
    free(grup->path);
    free(grup);

    return NULL;
} /* end H5VL_ncmpi_group_create() */


/*-------------------------------------------------------------------------
 * Function:    H5VL_ncmpi_group_open
 *
 * Purpose:     Handles the group open callback
 *
 * Return:      Success:    group pointer
 *              Failure:    NULL
 *
 *-------------------------------------------------------------------------
 */
void *H5VL_ncmpi_group_open(    void *obj, const H5VL_loc_params_t *loc_params, const char *name, 
                                hid_t  gapl_id, hid_t  dxpl_id, void  **req) {
    int err;
    int i;
    char tmp[PNC_VOL_MAX_NAME];
    char *ppath;
    H5VL_ncmpi_group_t *grup;
    H5VL_ncmpi_file_t *fp;

    /* Check arguments */
    if((loc_params->obj_type != H5I_FILE) && (loc_params->obj_type != H5I_GROUP))   RET_ERRN("container not a file or group")
    if(loc_params->type != H5VL_OBJECT_BY_SELF) RET_ERRN("loc_params->type is not H5VL_OBJECT_BY_SELF")

    if (loc_params->obj_type == H5I_FILE){
        fp = (H5VL_ncmpi_file_t*)obj;
        ppath = NULL;
    }
    else {
        fp = ((H5VL_ncmpi_group_t*)obj)->fp;
        ppath = ((H5VL_ncmpi_group_t*)obj)->path;
    }
    if (loc_params->type == H5VL_OBJECT_BY_NAME) { // Only group can house variable 
        // Try group
        if (ppath == NULL){
            sprintf(tmp, "_group_%s", loc_params->loc_data.loc_by_name.name);
        }
        else{
            sprintf(tmp, "_group_%s_%s", ppath, loc_params->loc_data.loc_by_name.name);
        }
        err = ncmpi_inq_attid(fp->ncid, NC_GLOBAL, tmp, &i);
        if (err != NC_NOERR){   // Neither, something wrong
            RET_ERRN("Specified object name not found")
        }

        if (ppath == NULL){
            sprintf(tmp, "%s", loc_params->loc_data.loc_by_name.name);
        }
        else{
            sprintf(tmp, "%s_%s", ppath, loc_params->loc_data.loc_by_name.name);
        }
    }

    grup = (H5VL_ncmpi_group_t*)malloc(sizeof(H5VL_ncmpi_group_t));
    grup->objtype = H5I_GROUP;
    grup->lcpl_id = -1;
    grup->gcpl_id = -1;
    grup->gapl_id = gapl_id;
    grup->dxpl_id = dxpl_id;
    grup->fp = fp;
    if (ppath == NULL){
        grup->path = (char*)malloc(strlen(name) + 1);
        sprintf(grup->path, "%s", name);
        grup->name = grup->path;
    }
    else{
        grup->path = (char*)malloc(strlen(ppath) + strlen(name) + 2);
        sprintf(grup->path, "%s_%s", ppath, name);
        grup->name = grup->path + strlen(ppath) + 1;
    }

    sprintf(tmp, "_group_%s", grup->path);
    err = ncmpi_inq_attid(fp->ncid, NC_GLOBAL, tmp, &i); CHECK_ERRJ

    return (void *)grup;

errout:
    free(grup->path);
    free(grup);

    return NULL;
} /* end H5VL_ncmpi_group_open() */


/*-------------------------------------------------------------------------
 * Function:    H5VL_ncmpi_group_get
 *
 * Purpose:     Handles the group get callback
 *
 * Return:      SUCCEED/FAIL
 *
 *-------------------------------------------------------------------------
 */
herr_t H5VL_ncmpi_group_get(    void *obj, H5VL_group_get_t get_type,
                                hid_t  dxpl_id, void  **req, va_list arguments) {
    return 0;
} /* end H5VL_ncmpi_group_get() */


/*-------------------------------------------------------------------------
 * Function:    H5VL_ncmpi_group_specific
 *
 * Purpose:     Handles the group specific callback
 *
 * Return:      SUCCEED/FAIL
 *
 *-------------------------------------------------------------------------
 */
herr_t H5VL_ncmpi_group_specific(   void *obj, H5VL_group_specific_t specific_type, 
                                    hid_t  dxpl_id, void  **req, va_list arguments) {
    return 0;
} /* end H5VL_ncmpi_group_specific() */


/*-------------------------------------------------------------------------
 * Function:    H5VL_ncmpi_group_optional
 *
 * Purpose:     Handles the group optional callback
 *
 * Return:      SUCCEED/FAIL
 *
 *-------------------------------------------------------------------------
 */
herr_t H5VL_ncmpi_group_optional(   void *obj, H5VL_file_optional_t opt_type, hid_t  dxpl_id,
                                    void  **req, va_list arguments) {
    return 0;
} /* end H5VL_ncmpi_group_optional() */


/*-------------------------------------------------------------------------
 * Function:    H5VL_ncmpi_group_close
 *
 * Purpose:     Handles the group close callback
 *
 * Return:      Success:    SUCCEED
 *              Failure:    FAIL (group will not be closed)
 *
 *-------------------------------------------------------------------------
 */
herr_t H5VL_ncmpi_group_close(void *grp, hid_t  dxpl_id, void  **req) {
    H5VL_ncmpi_group_t *gp = (H5VL_ncmpi_group_t*)grp;

    free(gp->path);
    free(gp);

    return 0;
} /* end H5VL_ncmpi_group_close() */

