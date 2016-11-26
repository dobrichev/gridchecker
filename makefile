CXX = g++ 
#CXX = /opt/intel/bin/icc 
CXXFLAGS = -m64 -fopenmp -O3 -march=native -g
#CXXFLAGS = -m64 -fopenmp -O0 -march=native -g

LDFLAGS = -lgomp

PFILES = anchor5.o anyoption.o clueIterator.o clusterize.o fClueIterator.o grid.o grids.o incrSolver.o main.o minimizer.o neighbourGrid.o options.o patterns.o patminlex.o rate.o rowminlex.o similarPuzzles.o solver.o subcanon.o tables.o templates.o t_128.o unav12.o uset.o skfr/utilities.o skfr/opsudo.o skfr/puzzle.o skfr/fsss.o skfr/t_128.o skfr/flog.o skfr/skfr.o skfr/ratingengine.o skfr/bitfields.o

all: $(PFILES)
	$(CXX) $(CXXFLAGS) -o pg $(PFILES)

clean:
	rm -f *.o
