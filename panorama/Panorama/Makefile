# Makefile for project 2

PROJ2=Panorama
PROJ2_OBJS=Project2.o BlendImages.o FeatureAlign.o FeatureSet.o WarpSpherical.o

IMAGELIB=ImageLib/libImage.a

CC=g++
CPPFLAGS=-Wall -O3 -I/uns/include
LIB_PATH=-L/uns/lib -L/usr/X11R6/lib
LIBS=-lfltk -lfltk_images -lpng -ljpeg -lX11 -lXext -lXi -lXmu

all: $(PROJ2)

$(IMAGELIB): 
	make -C ImageLib

$(PROJ2): $(PROJ2_OBJS) $(IMAGELIB)
	$(CC) -o $@ $(PROJ2_OBJS) $(LIB_PATH) $(LIBS) $(IMAGELIB)

clean:
	rm -f *.o *~ $(PROJ2)
