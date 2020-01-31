#include "ncmpi_vol.h"

herr_t H5VL_ncmpi_introspect_get_conn_cls(void *obj, H5VL_get_conn_lvl_t lvl, const H5VL_class_t **conn_cls);
herr_t H5VL_ncmpi_introspect_opt_query(void *obj, H5VL_subclass_t cls, int opt_type, hbool_t *supported);

const H5VL_introspect_class_t H5VL_ncmpi_introspect_g{
    H5VL_ncmpi_introspect_get_conn_cls,                       /* create */
    H5VL_ncmpi_introspect_opt_query,                         /* open */
};

/*-------------------------------------------------------------------------
 * Function:    H5VL_ncmpi_introspect_get_conn_clss
 *
 * Purpose:     Query the connector class.
 *
 * Return:      SUCCEED / FAIL
 *
 *-------------------------------------------------------------------------
 */
herr_t H5VL_ncmpi_introspect_get_conn_cls(void *obj, H5VL_get_conn_lvl_t lvl, const H5VL_class_t **conn_cls) {
#ifdef ENABLE_PASSTHRU_LOGGING
    printf("------- PASS THROUGH VOL INTROSPECT GetConnCls\n");
#endif

    assert(conn_cls);
    *conn_cls = &H5VL_ncmpi_g;

    return 0;
} /* end H5VL_ncmpi_introspect_get_conn_cls() */


/*-------------------------------------------------------------------------
 * Function:    H5VL_ncmpi_introspect_opt_query
 *
 * Purpose:     Query if an optional operation is supported by this connector
 *
 * Return:      SUCCEED / FAIL
 *
 *-------------------------------------------------------------------------
 */
herr_t H5VL_ncmpi_introspect_opt_query(void *obj, H5VL_subclass_t cls, int opt_type, hbool_t *supported) {
    herr_t err;

#ifdef ENABLE_PASSTHRU_LOGGING
    printf("------- PASS THROUGH VOL INTROSPECT OptQuery\n");
#endif

    //ret_value = H5VLintrospect_opt_query(o->under_object, o->under_vol_id, cls, opt_type, supported);

    *supported = false;

    return err;
} /* end H5VL_ncmpi_introspect_opt_query() */
