GCXX = g++ 
#CXX = /opt/intel/bin/icc 
CXXFLAGS = -m64 -fopenmp -O3 -march=native -msse4.2 -mavx -g3 -gdwarf-2 -Wall
#CXXFLAGS = -m64 -fopenmp -O0 -march=native -g

LDFLAGS = -lgomp

FILELIST = anchor5.cpp anyoption.cpp clueIterator.cpp clusterize.cpp fClueIterator.cpp grid.cpp grids.cpp incrSolver.cpp main.cpp minimizer.cpp neighbourGrid.cpp options.cpp patterns.cpp patminlex.cpp rowminlex.cpp similarPuzzles.cpp solver.cpp subcanon.cpp tables.cpp templates.cpp t_128.cpp unav12.cpp uset.cpp 
TARGET = gridchecker

all: $(PFILES)
	$(GCXX) $(CXXFLAGS) -o gridchecker $(FILELIST)

profiling:
	@echo 'Building target $(TARGET) using Gnu C++ Profile Generate settings'
	$(GCXX) $(CXXFLAGS) -fprofile-generate -o $(TARGET) $(FILELIST)
	@echo 'Done'

release:
	@echo 'Building target $(TARGET) using Gnu C++ Profile Use settings'
	$(GCXX) $(CXXFLAGS) -fprofile-use -o $(TARGET) $(FILELIST)
	@echo 'Done'

clean:
	rm -f *.o *.gcda *.dyn pgopti.*
