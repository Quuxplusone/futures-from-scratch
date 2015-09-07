
CXXFLAGS = -W -Wall -std=c++1y

TESTS = \
  TestAsync \
  TestConcurrentQueue \
  TestFuture \
  TestPackagedTask \
  TestScheduler \
  TestWhen

all: $(TESTS)

clean:
	rm -f $(TESTS)
