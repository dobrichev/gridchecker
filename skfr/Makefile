CXX = g++ 
CXXFLAGS = -fopenmp -O3 -march=native
#CXXFLAGS = -msse2 -O3

LDFLAGS = -lstdc++ -lgomp
#LDFLAGS = -lstdc++

skfr: utilities.o opsudo.o puzzle.o fsss.o t_128.o flog.o skfr.o ratingengine.o bitfields.o

clean:
	rm -f *.o
