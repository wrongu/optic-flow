UNAME = $(shell uname)
ifeq ($(UNAME), Linux)
HOME = /home/rlange
INCLUDE = -I$(HOME)/opencv/include/opencv \
-I$(HOME)/opencv/modules/core/include \
-I$(HOME)/opencv/modules/imgproc/include \
-I$(HOME)/opencv/modules/video/include \
-I$(HOME)/opencv/modules/features2d/include \
-I$(HOME)/opencv/modules/ml/include \
-I$(HOME)/opencv/modules/highgui/include
else
INCLUDE = -I/usr/local/include/opencv
endif
CC = g++
#LIBS := -lopencv_video -lopencv_features2d  -lopencv_ml -lopencv_core -lopencv_imgproc -lopencv_highgui
C_FLAGS = -Wall -pedantic -g
OBJS = segmentation.o math_helpers.o features.o functors.o of.o driver.o
EXECUTABLE = _run

-include $(OBJS:.o=.d)

all: $(EXECUTABLE)

$(EXECUTABLE): $(OBJS)
	$(CC) `pkg-config --libs opencv` $(C_FLAGS) $(OBJS) -o $(EXECUTABLE)

%.o: %.cpp
	@echo 'Compiling $<'
	$(CC) -c $(C_FLAGS) $(INCLUDE) $< -o $@
	$(CC) -MM $(C_FLAGS) $*.cpp > $*.d
	@echo ''

clean:
	rm *.o
	rm *.d
	rm _run
