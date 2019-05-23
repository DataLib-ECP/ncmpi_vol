#ifndef _ncmpi_VOL_H_
#define _ncmpi_VOL_H_

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Copyright by The HDF Group.                                               *
 * All rights reserved.                                                      *
 *                                                                           *
 * This file is part of HDF5.  The full HDF5 copyright notice, including     *
 * terms governing use, modification, and redistribution, is contained in    *
 * the COPYING file, which can be found at the root of the source code       *
 * distribution tree, or in https://support.hdfgroup.org/ftp/HDF5/releases.  *
 * If you do not have access to either file, you may request a copy from     *
 * help@hdfgroup.org.                                                        *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/*
 * Purpose: This is a "pass through" VOL connector, which forwards each
 *      VOL callback to an underlying connector.
 *
 *      It is designed as an example VOL connector for developers to
 *      use when creating new connectors, especially connectors that
 *      are outside of the HDF5 library.  As such, it should _NOT_
 *      include _any_ private HDF5 header files.  This connector should
 *      therefore only make public HDF5 API calls and use standard C /
 *              POSIX calls.
 */

/* Header files needed */
/* (Public HDF5 and standard C / POSIX only) */
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "hdf5.h"

/* Identifier for the pass-through VOL connector */
#define H5VL_NCMPI  (H5VL_ncmpi_register())
#ifdef __cplusplus
extern "C" {
#endif
H5_DLL hid_t H5VL_ncmpi_register(void);
#ifdef __cplusplus
}
#endif

/* Characteristics of the pass-through VOL connector */
#define H5VL_NCMPI_NAME        "PnetCDF"
#define H5VL_NCMPI_VALUE        1026           /* VOL connector ID */
#define H5VL_NCMPI_VERSION      1111

/************/
/* Typedefs */
/************/

/* Pass-through VOL connector info */
typedef struct H5VL_ncmpi_info_t {
    MPI_Comm comm;
} H5VL_ncmpi_info_t;

/* The pass through VOL info object */
typedef struct H5VL_ncmpi_t {
    //PNC Code starts
    int ncid;
    //PNC Code ends
} H5VL_ncmpi_t;

/********************* */
/* Function prototypes */
/********************* */

/* "Management" callbacks */
herr_t H5VL_ncmpi_init(hid_t vipl_id);
herr_t H5VL_ncmpi_term(void);
void *H5VL_ncmpi_info_copy(const void *info);
herr_t H5VL_ncmpi_info_cmp(int *cmp_value, const void *info1, const void *info2);
herr_t H5VL_ncmpi_info_free(void *info);
herr_t H5VL_ncmpi_info_to_str(const void *info, char **str);
herr_t H5VL_ncmpi_str_to_info(const char *str, void **info);
void *H5VL_ncmpi_get_object(const void *obj);
herr_t H5VL_ncmpi_get_wrap_ctx(const void *obj, void **wrap_ctx);
herr_t H5VL_ncmpi_free_wrap_ctx(void *obj);
void *H5VL_ncmpi_wrap_object(void *obj, H5I_type_t obj_type, void *wrap_ctx);
void *H5VL_ncmpi_unwrap_object(void *wrap_ctx);

/* File callbacks */
void *H5VL_ncmpi_file_create(const char *name, unsigned flags, hid_t fcpl_id, hid_t fapl_id, hid_t dxpl_id, void **req);
void *H5VL_ncmpi_file_open(const char *name, unsigned flags, hid_t fapl_id, hid_t dxpl_id, void **req);
herr_t H5VL_ncmpi_file_get(void *file, H5VL_file_get_t get_type, hid_t dxpl_id, void **req, va_list arguments);
herr_t H5VL_ncmpi_file_specific(void *file, H5VL_file_specific_t specific_type, hid_t dxpl_id, void **req, va_list arguments);
herr_t H5VL_ncmpi_file_optional(void *file, hid_t dxpl_id, void **req, va_list arguments);
herr_t H5VL_ncmpi_file_close(void *file, hid_t dxpl_id, void **req);

extern const H5VL_file_class_t H5VL_ncmpi_file_g;
extern const H5VL_class_t H5VL_ncmpi_g;

#endif