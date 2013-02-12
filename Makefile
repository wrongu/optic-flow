CC = g++
C_FLAGS = -Wall -pedantic -g -save-temps
INCLUDE = -Iinclude/ -I/usr/local/include/opencv
OBJS = segmentation.o math_helpers.o features.o functors.o
EX_PREFIX = _

all: driver

driver: $(OBJS)
	$(CC) $(C_FLAGS) $(OBJS) -o driver

%.o: %.cpp %.hpp
	@echo 'Compiling $<'
	$(CC) -c $(C_FLAGS) `pkg-config --cflags opencv` $(INCLUDE) `pkg-config --libs opencv` $< -o $@
	@echo ''

$(EX_PREFIX)%: %.cpp
	$(CC) $(C_FLAGS) `pkg-config --cflags opencv` $(INCLUDE) `pkg-config --libs opencv` $< -o $@

clean:
	rm *.o
	rm *.ii
	rm *.s

debug:
	make 'CFLAGS=$(CFLAGS) -g'