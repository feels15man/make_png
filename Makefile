.PHONY: all clean blob dynamic static

CC=gcc-12
LD=ld
INCLUDES=-I freetype/include -I. -I libpng/ -I /zlib
LD_DYNAMIC= -L ../freetype/objs/.libs/ -L ../libpng/build/ -lfreetype -lpng -ldl -lm -D DYNAMIC
LD_STATIC = -static -L ../freetype/objs/.libs/ -L ../libpng/build/ -lfreetype -lpng -lm -lz

BLOB_FLAGS=-D BLOB -nostdlib -fPIE -fPIC -e _start
LD_BLOB = -L ../freetype/objs/.libs/ -L ../libpng/build/ -lfreetype -lpng 

BUILD_ARTIFACT_DIR=build/
  
APP = app/

all: blob dynamic static

blob: blob.o
	$(CC) $(APP)main.c -o $(APP)app-blob $(BLOB_FLAGS) $(INCLUDES) $(LD_BLOB)
	
blob.o:
	$(CC) $(APP)main.c -c -o $(BUILD_ARTIFACT_DIR)blob $(BLOB_FLAGS) $(INCLUDES) $(LD_BLOB)

static: static.o
	$(CC) $(APP)main.c -o $(APP)app-static $(LD_STATIC) $(INCLUDES)

static.o:
	$(CC) $(APP)main.c -c -o $(BUILD_ARTIFACT_DIR)static $(LD_STATIC) $(INCLUDES)

dynamic: dynamic.o
	$(CC) $(BUILD_ARTIFACT_DIR)dynamic.o -o $(APP)app-dynamic

dynamic.o:
	$(CC) $(APP)main.c -c -o $(BUILD_ARTIFACT_DIR)dynamic.o $(LD_DYNAMIC) $(INCLUDES)

clean:
	rm -f $(APP)app-*
	rm -f $(BUILD_ARTIFACT_DIR)*
