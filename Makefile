CC=mpicxx

HDF5_DIR=${HOME}/.local/hdf5_dev
PNC_DIR=${HOME}/.local

CFLAGS=-I${HDF5_DIR}/include -I${PNC_DIR}/include -o 0 -ggdb -DENABLE_LOGGING
LDFLAGS=-L${HDF5_DIR}/lib -L${PNC_DIR}/lib -lhdf5_hl -lhdf5 -lpnetcdf

SRCS = $(wildcard *.c)
OBJS = $(SRCS:.c=.o)

%.o: %.c
	$(CC) $(CFLAGS) -o $@ -c $<

all: test

test: $(OBJS)
	$(CC) ${CFLAGS} -o $@ $^ $(LDFLAGS)

.PHONY: clean
clean:
	rm -f $(OBJS) test