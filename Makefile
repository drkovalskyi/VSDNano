all: reveNtuple


reveNtuple: reveNtuple
	c++ `root-config --cflags` -fPIC  reveNtuple.cc -L`root-config --libdir` -lROOTEve -lROOTWebDisplay -lCore -lMathCore -lRIO -lRint -o reveNtuple


clean:
	rm reveNtuple
