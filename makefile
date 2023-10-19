# run this command to use C++20: module load gcc-11.2.0
# run the command below to load shared libraries:
# export LD_LIBRARY_PATH=/opt/boost-1.80.0/lib:$LD_LIBRARY_PATH

# Use the BOOST_ROOT environment variable or a default value
BOOST_ROOT ?= /opt/boost-1.80.0

INCLUDE = -I$(BOOST_ROOT) -I$(BOOST_ROOT)/include
LIB = -L$(BOOST_ROOT)/lib
BOOST = -lboost_thread

all: test

test: test.cc message.hpp kvstore.hpp kvserver.cc kvclient.cc
	g++ -std=c++20 -pthread $(INCLUDE) $(LIB) -o test test.cc $(BOOST)

clean:
	rm -f test
