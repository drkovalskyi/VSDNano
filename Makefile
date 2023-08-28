all: reveNtuple

sample:
	curl -O https://amraktad.web.cern.ch/amraktad/nano-CMSSW_11_0_0-RelValZTT-mcRun.root

test: sample reveNtuple
	./reveNtuple nano-CMSSW_11_0_0-RelValZTT-mcRun.root examples/nano.json

reveNtuple: reveNtuple
	c++ `root-config --cflags` -fPIC  reveNtuple.cc -L`root-config --libdir` -lROOTEve -lROOTWebDisplay -lCore -lMathCore -lRIO -lRint -o reveNtuple

clean:
	rm reveNtuple
