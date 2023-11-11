ROOT_CFLAGS := `root-config --cflags`
ROOT_LDFLAGS := -L`root-config --libdir`

all: reveNtuple

sample:
	curl -O https://amraktad.web.cern.ch/amraktad/nano-CMSSW_11_0_0-RelValZTT-mcRun.root

test: sample reveNtuple
	./reveNtuple nano-CMSSW_11_0_0-RelValZTT-mcRun.root examples/nano.json

reveNtuple: reveNtuple
	c++ `root-config --cflags` -fPIC  reveNtuple.cc -L`root-config --libdir` -lROOTEve -lROOTWebDisplay -lCore -lMathCore -lRIO -lRint -o reveNtuple

service:
	c++ `root-config --cflags` -fPIC  service.cc -L`root-config --libdir` -lROOTEve -lROOTWebDisplay -lCore -lRIO -lMathCore -lRint -lNet -o service


clean:
	rm reveNtuple

### VsdTree and dicts

VsdDict.cc VsdDict_rdict.pcm &: VSDBase.h Vsd_Linkdef.h
	rootcling -I. -f VsdDict.cc VSDBase.h Vsd_Linkdef.h

libVsdDict.so: VsdDict.cc
	c++ -shared -fPIC -o libVsdDict.so ${ROOT_CFLAGS} VsdDict.cc

mt_read: VsdTree.h VsdTree.cc VsdDict.cc
	c++ -DSTANDALONE_READ_TEST ${ROOT_CFLAGS} -g -O0 -std=c++1z `root-config --libs` -o $@ VsdTree.cc VsdDict.cc

mt_write: VsdTree.h VsdTree.cc VsdDict.cc
	c++ -DSTANDALONE_WRITE_TEST ${ROOT_CFLAGS} -g -O0 -std=c++1z `root-config --libs` -o $@ VsdTree.cc VsdDict.cc
