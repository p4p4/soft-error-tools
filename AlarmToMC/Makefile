AIGERPATH = /home/pklampfl/libs/aiger-1.9.4/

CXXFILES = AlarmToMC.cpp 
CXXFLAGS = -O3 -o alarmToMC

CXXFLAGS += -I$(AIGERPATH) -std=c++11
LDFLAGS = -L$(AIGERPATH)
all:
	$(CXX) $(CXXFILES) $(CXXFLAGS) $(LDFLAGS) $(AIGERPATH)aiger.o

clean:
	rm -f alarmToMC *.o
