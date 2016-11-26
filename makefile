CXX = g++ 
#CXX = /opt/intel/bin/icc 
CXXFLAGS = -m64 -fopenmp -O3 -march=native -g
#CXXFLAGS = -m64 -fopenmp -O0 -march=native -g

LDFLAGS = -lgomp

PFILES = anchor5.o anyoption.o clueIterator.o clusterize.o fClueIterator.o grid.o grids.o incrSolver.o main.o minimizer.o neighbourGrid.o options.o patterns.o patminlex.o rowminlex.o similarPuzzles.o solver.o subcanon.o tables.o templates.o t_128.o unav12.o uset.o 

all: $(PFILES)
	$(CXX) $(CXXFLAGS) -o gridchecker $(PFILES)

clean:
	rm -f *.o
