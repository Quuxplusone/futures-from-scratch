
CXXFLAGS = -W -Wall -std=c++1y

TESTS = \
  TestConcurrentQueue

all: $(TESTS)

clean:
	rm -f $(TESTS)
