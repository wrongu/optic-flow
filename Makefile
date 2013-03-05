UNAME = $(shell uname)
ifeq ($(UNAME), Linux)
HOME = /home/rlange
INCLUDE = `pkg-config --cflags $(HOME)/local/lib/pkgconfig/opencv.pc` 
LIBS = -L$(HOME)/local/lib `cat libflags.make`
LIB_PATH = $(HOME)/local/lib
CC = g++
else
INCLUDE = -I/usr/local/include/opencv
LIBS = `pkg-config --libs opencv`
LIB_PATH =
CC = g++
endif
C_FLAGS = -Wall -Wno-long-long -pedantic -g
OBJS = segmentation.o math_helpers.o features.o functors.o of.o driver.o
EXECUTABLE = _run

-include $(OBJS:.o=.d)

all:
	@make $(EXECUTABLE)

$(EXECUTABLE): LIBPATH $(OBJS)
	@echo 'building $(EXECUTABLE)'
	@$(CC) $(C_FLAGS) $(OBJS) $(LIBS) -o $(EXECUTABLE)
	@echo '..done'

%.o: %.cpp
	@echo 'Compiling $<'
	$(CC) -c $(C_FLAGS) $(INCLUDE) $< -o $@
	$(CC) -MM $(C_FLAGS) $*.cpp > $*.d
	@echo ''

clean:
	rm *.o
	rm _run

LIBPATH:
	@if test "$(LIB_PATH)" != "" && "${LD_LIBRARY_PATH}" == "" ; then \
		echo "LD_LIBRARY_PATH not set. exiting."; \
		exit 1; \
	fi	
