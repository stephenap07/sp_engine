CC =g++
LDFLAGS =-lboost_system -lboost_filesystem -lSDL2 -lSDL2_image -lGLEW
EXE =ex
INC=../../
CFLAGS =-std=c++11 -Wall -g -I$(INC)
SRC =$(wildcard ../../*.cpp) $(wildcard *.cpp)
OBJ =$(addprefix obj/,$(notdir $(SRC:.cpp=.o)))  

UNAME := $(shell uname)

ifeq ($(UNAME), Linux)
	LDFLAGS += -lGL
endif
ifeq ($(UNAME), Darwin)
	LDFLAGS += -framework OpenGL
endif

all: $(EXE)

$(EXE): $(OBJ)
	$(CC) $(OBJ) $(LDFLAGS) -o $@

obj/main.o: main.cpp
	$(CC) $< -c $(CFLAGS) -o $@ 

obj/%.o: ../../%.cpp | obj
	$(CC) $< -c $(CFLAGS) -o $@ 

obj:
	mkdir obj

clean:
	rm $(OBJ) $(EXE)

run:
	./$(EXE)
