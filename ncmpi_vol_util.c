#include "ncmpi_vol.h"

MPI_Datatype h5t_to_mpi_type(hid_t type_id) {
    if (type_id == H5T_NATIVE_CHAR) return MPI_CHAR;
    else if (type_id == H5T_NATIVE_SHORT) return MPI_SHORT;
    else if (type_id == H5T_NATIVE_INT) return MPI_INT;
    else if (type_id == H5T_NATIVE_LONG) return MPI_LONG;
    else if (type_id == H5T_NATIVE_LLONG) return MPI_LONG_LONG;
    else if (type_id == H5T_NATIVE_UCHAR) return MPI_UNSIGNED_CHAR;
    else if (type_id == H5T_NATIVE_USHORT) return MPI_UNSIGNED_SHORT;
    else if (type_id == H5T_NATIVE_UINT) return MPI_UNSIGNED;
    else if (type_id == H5T_NATIVE_ULONG) return MPI_UNSIGNED_LONG;
    else if (type_id == H5T_NATIVE_ULLONG) return MPI_UNSIGNED_LONG_LONG;
    else if (type_id == H5T_NATIVE_FLOAT) return MPI_FLOAT;
    else if (type_id == H5T_NATIVE_DOUBLE) return MPI_DOUBLE;
    else if (type_id == H5T_NATIVE_LDOUBLE) return MPI_DOUBLE;
    return MPI_DATATYPE_NULL;
}

nc_type h5t_to_nc_type(hid_t type_id) {
    if (type_id == H5T_NATIVE_CHAR) return NC_CHAR;
    else if (type_id == H5T_NATIVE_SHORT) return NC_SHORT;
    else if (type_id == H5T_NATIVE_INT) return NC_INT;
    else if (type_id == H5T_NATIVE_LONG) return NC_INT;
    else if (type_id == H5T_NATIVE_LLONG) return NC_INT64;
    else if (type_id == H5T_NATIVE_UCHAR) return NC_CHAR;
    else if (type_id == H5T_NATIVE_USHORT) return NC_USHORT;
    else if (type_id == H5T_NATIVE_UINT) return NC_UINT;
    else if (type_id == H5T_NATIVE_ULONG) return NC_UINT;
    else if (type_id == H5T_NATIVE_ULLONG) return NC_UINT64;
    else if (type_id == H5T_NATIVE_FLOAT) return NC_FLOAT;
    else if (type_id == H5T_NATIVE_DOUBLE) return NC_DOUBLE;
    else if (type_id == H5T_NATIVE_LDOUBLE) return NC_DOUBLE;
    return NC_NAT;
}