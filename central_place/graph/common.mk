GCC=mpic++

CXXFLAGS=-std=c++11 -O3
IFLAGS=
LFLAGS=-lstdc++ -Wl,-Bstatic -lboost_serialization -lboost_mpi -Wl,-Bdynamic -lm -L/usr/local/lib

DIR=objs

SOURCES=$(wildcard *.cpp)
OBJS=$(patsubst %.cpp, $(DIR)/%.o, $(SOURCES))

compile: $(TARGET)

.PHONY: compile lib clean

$(TARGET) : $(OBJS)

$(DIR)/%.o : %.cpp
	@mkdir -p $(DIR)
	$(GCC) $(CXXFLAGS) $(IFLAGS) $(LFLAGS) -c -o $@ $<

debug: CXXFLAGS=-std=c++11 -g -O0
debug: compile 

lib :
	ar rcs $(DIR)/libgraph.a $(DIR)/*.o

clean :
	rm -rf $(DIR)/*.o $(TARGET)
	rm -rf *.o
