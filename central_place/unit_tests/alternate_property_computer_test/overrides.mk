GCC=mpic++

CXXFLAGS=-std=c++11 -O0 -g
IFLAGS=
LFLAGS=-lstdc++ -Wl,-Bstatic -lgtest -lgtest_main -lboost_serialization -lboost_graph -lboost_regex -lboost_system -lboost_filesystem -lboost_mpi -lboost_program_options -Wl,-Bdynamic -lm -L/home/natayan/boost/boost_1_53_0/stage/lib -lm -L/usr/local/lib

DIR=objs

SOURCES=$(wildcard *.cpp)
OBJS=$(patsubst %.cpp, $(DIR)/%.o, $(SOURCES))

compile: $(TARGET)

.PHONY: compile clean

$(TARGET) : $(OBJS)

$(DIR)/%.o : %.cpp
	@mkdir -p $(DIR)
	$(GCC) $(CXXFLAGS) $(IFLAGS) $(LFLAGS) -c -o $@ $<


clean :
	rm -rf $(DIR)/*.o $(TARGET)
	rm -rf *.o
