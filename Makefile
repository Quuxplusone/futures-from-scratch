
CXXFLAGS = -W -Wall -std=c++1y

TESTS = \
  TestConcurrentQueue \
  TestScheduler

all: $(TESTS)

clean:
	rm -f $(TESTS)
