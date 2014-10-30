CC   = g++
SRC  = $(wildcard *.cpp) $(wildcard src/*/*.cpp)
OBJ  = $(addprefix obj/,$(notdir $(SRC:.cpp=.o)))  
DEPS = $(SRC:%.cpp=obj/%.d)

LDFLAGS = -lboost_system -lboost_filesystem -lGLEW -lSDL2 -lSDL2_image -L/usr/local/lib -lfreetype -lpthread
CFLAGS  = -std=c++11 -Wall -fPIC -g -I/usr/local/include/freetype2

EXE = sp

UNAME := $(shell uname)

ifeq ($(UNAME), Linux)
	LDFLAGS += -lGL
endif
ifeq ($(UNAME), Darwin)
	LDFLAGS += -framework OpenGL
	CFLAGS += -I/usr/local/include
endif

.PHONY: all clean run

all: $(EXE)

$(EXE): $(OBJ)
	$(CC) $(OBJ) $(LDFLAGS) -o $@

obj/%.o: %.cpp | obj
	$(CC) -MMD -MP -c $< $(CFLAGS) -o $@ 

obj:
	mkdir obj

run:
	./$(EXE)

clean:
	rm $(OBJ) $(DEPS)

-include $(DEPS)
