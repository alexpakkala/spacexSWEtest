CPP=clang++ -std=c++17

.PHONY: clean test

# Build and run all tests.
test: test.out ../test/*.txt
	../test.sh test.out solution.cc

# Clean outputs.
clean:
	rm -rf *.o test.out

# Build tests.
test.out: test.cc solution.cc util.h
	$(CPP) -O3 -o test.out test.cc solution.cc
