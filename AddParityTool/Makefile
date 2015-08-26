AIGERPATH = /home/pklampfl/libs/aiger-1.9.4/

CXXFILES = AddParityTool.cpp 
CXXFLAGS = -O3 -o addParityTool

CXXFLAGS += -I$(AIGERPATH) -std=c++11
LDFLAGS = -L$(AIGERPATH)
all:
	$(CXX) $(CXXFILES) $(CXXFLAGS) $(LDFLAGS) $(AIGERPATH)aiger.o

clean:
	rm -f addParityTool *.o
