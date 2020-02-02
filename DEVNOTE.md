# Summary

A prototype implementation of PnetCDF VOL

## Functionality
* File
  + Create/open NetCDF file
* Dataset
  + Define NetCDF variable
  + Read/write NetCDF variable
    + Selection limited to 1 simple selection
* Attribute
  + Add attribute to variables (dataset)
  + Add attribute to file
  + Add attribute to group
* Group
  + Create group
  + Associate variable and attribute to groups

## Design
* File
  + Maps to NetCDF file
* Dataset
  + Maps to NetCDF variable
* Attribute
  + Maps to NetCDF attribute
  + Shape is not preserved
    + ADIOS attribute can be multi-dimensional while NetCDF attributes are always 1-D
    + high-dimension attributes is flattened into 1-D when storing into NetCDF format
    + The shape is not recorded in the file and is permently lost once the session close
      + The attribute will be presented as 1-D when reading from NetCDF file
    + However, the shape is cached in the attribute structure
        + Correct shape will be returned when querying before closing the object.
* Group
  + NetCDF has no concept of group
  + Simulate using path like name
    + Object B under group A will be named A_B and stored in the file (root group) directly
  + Special attribute to record group name

## Implementation detail
* Identifying object
  + Some VOL functions (especially get) pass object pointer without specifying it's type
  + The first entry of every VOL struct is a int describing the type of the object
  + The pointer is treated as int* to retrieve the type indicator
  + After the type is known, the pointer can be treated as its actual type
* Supporting Collective/Independent I/O
  + HDF (and hence VOL) has no explicite function call for switching collective and independent mode (same as define and data mode)
  + By default, HDF I/O are independent. Collective I/O is indicated by a property set on each dataset write operation
  + The VOL must call PnetCDF mode transition fnctions apropriately based on current operation
  + PnetCDF mode switch API are all collective. As a result, we cannot switch to independent mode if a independent dataset write is called when we are in collective mode
    + By the time we know it is independent, we are already in the independent function call. Some process may not participate.
  + One solution is to keep PnetCDF in independent and data mode all the time.
    + Transition to other mode only when necessary
    + Switch back right after collective or metadta operation is done
    + Mopde switching involves file sync that can hinder performance
  + We use PnetCDF's non-blocking API
    + Every H5Dwrite and H5Dread only post the I/O operation
    + The application must call H5Fflush to carry out actual I/O oepration
      + H5Fflush must be called collectively
    + The application shall not modify the data buffer before H5Fflush completes

## Future work
* Support memory space selection
  + Current implmentation assums the memory space is contiguous
* Make it a public library
* Continue update to match the newest VOL interface in HDF5 developer branch
  + VOL is not yet in stable release at the time of the writing
  + There can be ongoing change to VOL interface that makes this prototype outdated

## HDF5 limitation
* Heavy metadata operaiton
  + Need complex metadata cache machanism
  + Metadata processing takes considerable time
* No I/O aggregation
  + Dwrite and Dread has no non-blocking counterpart
* Complex, unintutive object traversal
  + The only way to iterate through ta objects is through a call back function
  + The only way to access an object directly is through it's name
* Limitation on MPI collective I/O
  + I/O must perform on same dataset
    + Low scalibility on small datasets
  + No chunk caching
    + Cache disabled on MPI I/O driver
  + No collective read on chunked dataset
    + Decompression can be repetitive
    

