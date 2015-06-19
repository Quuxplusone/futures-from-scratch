
CXXFLAGS = -W -Wall -std=c++1y

TESTS = \
  TestConcurrentQueue \
  TestFuture \
  TestScheduler

all: $(TESTS)

clean:
	rm -f $(TESTS)
