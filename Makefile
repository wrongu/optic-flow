UNAME = $(shell uname)
ifeq ($(UNAME), Linux)
HOME = /home/rlange
INCLUDE = `pkg-config --cflags $(HOME)/local/lib/pkgconfig/opencv.pc` 
LIBS = -L$(HOME)/local/lib `cat libflags.make`
LIB_REQ = LIBPATH
CC = gcc
#LIBS = -L$(HOME)/local/lib `pkg-config --libs $(HOME)/local/lib/pkgconfig/opencv.pc`
#LIBS = -L$(HOME)/local/lib/ 
#LIBS := -L$(HOME)/local/lib -lopencv_video -lopencv_ml -lopencv_core -lopencv_imgproc -lopencv_highgui
#LIBS = -L$(HOME)/local/lib2 `ls $(HOME)/local/lib2/ | sed 's:l.*:-l& \\:'`
else
INCLUDE = -I/usr/local/include/opencv
LIBS = `pkg-config --libs opencv`
LIB_REQ = 
CC = g++
endif
C_FLAGS = -Wall -Wno-long-long -pedantic -g
OBJS = of.o driver.o features.o functors.o segmentation.o math_helpers.o
EXECUTABLE = _run

-include $(OBJS:.o=.d)

all: $(EXECUTABLE)
.PHONY: all

$(EXECUTABLE): $(OBJS) $(LIB_REQ)
	$(CC) $(C_FLAGS) $(OBJS) $(LIBS) -o $(EXECUTABLE)

%.o: %.cpp
	@echo 'Compiling $<'
	$(CC) -c $(C_FLAGS) $(INCLUDE) $< -o $@
	$(CC) -MM $(C_FLAGS) $*.cpp > $*.d
	@echo ''

clean:
	rm *.o
	rm *.d
	rm _run

LIBPATH:
	@if test "${LD_LIBRARY_PATH}" == "" ; then \
		echo "LD_LIBRARY_PATH not set. exiting."; \
		exit 1; \
	fi	
