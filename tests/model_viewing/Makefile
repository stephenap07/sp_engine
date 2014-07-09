CC = clang++

EXE = ex
INC = ../../

LDFLAGS = ../../lib/libspengine.so -lboost_system -lboost_filesystem -lGLEW -lSDL2 -lSDL2_image
CFLAGS  = -std=c++11 -Wall -Wextra -g -I$(INC)

SRC = $(wildcard *.cpp)
OBJ = $(addprefix obj/,$(notdir $(SRC:.cpp=.o)))  

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
