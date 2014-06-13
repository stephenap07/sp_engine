CC =g++
LDFLAGS =-lboost_system -lboost_filesystem -lSDL2 -lSDL2_image -lGLEW
EXE =ex
CFLAGS =-std=c++11 -Wall -g
SRC =$(wildcard *.cpp) $(wildcard src/*/*.cpp)
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

obj/%.o: %.cpp | obj
	$(CC) $< -c $(CFLAGS) -o $@

obj:
	mkdir obj

clean:
	rm $(OBJ) $(EXE)

run:
	./$(EXE)
