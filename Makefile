
CXXFLAGS = -W -Wall -std=c++1y

TESTS = \
  TestAsync \
  TestConcurrentQueue \
  TestFuture \
  TestScheduler

all: $(TESTS)

clean:
	rm -f $(TESTS)
