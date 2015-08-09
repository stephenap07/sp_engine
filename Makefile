CC   = clang++
SRC  = $(wildcard *.cpp) $(wildcard src/*/*.cpp)
OBJ  = $(addprefix obj/,$(notdir $(SRC:.cpp=.o)))  
DEPS = $(SRC:%.cpp=obj/%.d)

LDFLAGS = -lboost_system -lboost_filesystem -lGLEW -lSDL2 -lSDL2_image -L/usr/local/lib -lfreetype -lpthread
CFLAGS  = -std=c++1y -Wall -fPIC -g -pg -I/usr/include/freetype2 -I/usr/local/include

EXE = sp

UNAME := $(shell uname)

ifeq ($(UNAME), Linux)
	LDFLAGS += -lGL
endif
ifeq ($(UNAME), Darwin)
	LDFLAGS += -framework OpenGL
	CFLAGS += -I/usr/local/include/freetype2
endif

.PHONY: all clean run

all: $(EXE)

$(EXE): $(OBJ)
	$(CC) $(OBJ) $(CFLAGS) $(LDFLAGS) -o $@

obj/%.o: %.cpp | obj
	$(CC) -MMD -MP -c $< $(CFLAGS) -o $@ 

obj:
	mkdir obj

run:
	./$(EXE)

clean:
	rm -f $(EXE) $(OBJ) $(DEPS)

-include $(DEPS)
