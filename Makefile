CC =g++

SRC = $(wildcard *.cpp) $(wildcard src/*/*.cpp)
OBJ = $(addprefix obj/,$(notdir $(SRC:.cpp=.o)))  

LDFLAGS = -lboost_system -lboost_filesystem -lSDL2 -lSDL2_image -lGLEW -shared
CFLAGS  = -std=c++11 -Wall -fPIC -g

DYNAMIC = lib/libspengine.so
STATIC  = lib/libspengine.a

UNAME := $(shell uname)

ifeq ($(UNAME), Linux)
	LDFLAGS += -lGL
endif
ifeq ($(UNAME), Darwin)
	LDFLAGS += -framework OpenGL
endif

all: $(DYNAMIC) $(STATIC)

$(DYNAMIC): $(OBJ) | lib
	$(CC) $(OBJ) $(LDFLAGS) -o $@

$(STATIC): $(OBJ) | lib
	ar rcs $@ $(OBJ)

obj/%.o: %.cpp | obj
	$(CC) $< -c $(CFLAGS) -o $@ 

obj:
	mkdir obj

lib:
	mkdir lib

clean:
	rm $(OBJ) $(DYNAMIC)
