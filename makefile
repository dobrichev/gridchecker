GCXX = g++ 
ICXX = /opt/intel/bin/icc 
CLXX = clang++

GCXXFLAGS = -m64 -fopenmp -O3 -funsafe-loop-optimizations -fno-guess-branch-probability -falign-functions=16 -falign-jumps=16 -falign-loops=16 -falign-labels=16 -g -pedantic -march=native -msse4.2 -mavx -g3 -gdwarf-2 -Wall -Wextra -MMD -MP -static -fuse-ld=gold -fuse-linker-plugin -flto-partition=none -std=c++11
ICXXFLAGS = -m64 -fopenmp -O0 -march=native -g
CLXXFLAGS = -m64 -fopenmp -O3 -march=native -msse4.2 -mavx -g3 -gdwarf-2 -Wall -std=gnu++11

#LDFLAGS = -lgomp

FILELIST = anchor5.cpp anyoption.cpp catalog.cpp clueIterator.cpp clusterize.cpp fClueIterator.cpp grid.cpp grids.cpp incrSolver.cpp main.cpp minimizer.cpp neighbourGrid.cpp options.cpp patminlex.cpp patterns.cpp rowminlex.cpp similarPuzzles.cpp solver.cpp subcanon.cpp t_128.cpp tables.cpp templates.cpp unav12.cpp uset.cpp 
TARGET = gridchecker

all: $(PFILES)
	$(GCXX) $(GCXXFLAGS) -o $(TARGET) $(FILELIST)

gnu: $(PFILES)
	$(GCXX) $(GCXXFLAGS) -o $(TARGET) $(FILELIST)
	
icc: $(PFILES)
	$(ICXX) $(ICXXFLAGS) -o $(TARGET) $(FILELIST)

clang: $(PFILES)
	$(CLXX) $(CLXXFLAGS) -o $(TARGET) $(FILELIST)

clean:
	rm -f *.o *.gcda *.dyn pgopti.*
