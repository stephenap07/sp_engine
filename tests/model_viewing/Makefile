CC = clang++

EXE = ex
INC = -I../../ -I/usr/include/freetype2

LDFLAGS = ../../lib/libspengine.so -lboost_system -lboost_filesystem -lGLEW -lSDL2 -lSDL2_image -L/usr/local/lib -lfreetype
CFLAGS  = -std=c++11 -O0 -Wall -Wextra -g $(INC)

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
