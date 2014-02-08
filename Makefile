CC=g++
FLAGS=-D_FILE_OFFSET_BITS=64 -lfuse -FUSE_USE_VERSION=26 -I/usr/include/fuse -L/usr/lib
#LFLAGS=
SOURCE=compositfs.cpp compositfs-helpers.cpp
#OBJECTS=$(SOURCE:.cpp=.o)
EXECUTABLE=Compositfs

all:$(SOURCE) $(EXECUTABLE)

$(EXECUTABLE):$(SOURCE)
	$(CC) $(SOURCE) $(FLAGS) -o $@
