CC =g++
LDFLAGS =-lboost_system -lboost_filesystem -lSDL2 -lSDL2_image -lGLEW -lGL
EXE =ex
CFLAGS =-std=c++11 -Wall -g
SRC =$(wildcard *.cpp) $(wildcard src/*/*.cpp)
OBJ =$(addprefix obj/,$(notdir $(SRC:.cpp=.o)))  

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
