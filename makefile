# run this command to use C++20: module load gcc-11.2.0
# run the command below to load shared libraries:
# export LD_LIBRARY_PATH=/opt/boost-1.80.0/lib:$LD_LIBRARY_PATH
INCLUDE = -I/opt/boost-1.80.0 -I/opt/boost-1.80.0/include 
LIB = -L/opt/boost-1.80.0/lib
BOOST = -lboost_thread
kvclient: kvclient.cc
	g++ -std=c++20 -pthread $(INCLUDE) $(LIB) -okvclient kvclient.cc $(BOOST)
kvserver: kvserver.cc
	g++ -std=c++20 -pthread $(INCLUDE) $(LIB) -okvserver kvserver.cc $(BOOST)
clean:
	rm kvclient
	rm kvserver
