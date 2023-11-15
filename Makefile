ROOT_CFLAGS := `root-config --cflags`
ROOT_LDFLAGS := -L`root-config --libdir`

all: reveNtuple

sample:
	curl -O https://amraktad.web.cern.ch/amraktad/nano-CMSSW_11_0_0-RelValZTT-mcRun.root

test: sample reveNtuple
	./reveNtuple nano-CMSSW_11_0_0-RelValZTT-mcRun.root examples/nano.json

reveNtuple: reveNtuple.cc
	c++ `root-config --cflags` -fPIC  reveNtuple.cc -L`root-config --libdir` -lROOTEve -lROOTWebDisplay -lCore -lMathCore -lRIO -lRint -o reveNtuple

service:
	c++ `root-config --cflags` -fPIC  service.cc -L`root-config --libdir` -lROOTEve -lROOTWebDisplay -lCore -lRIO -lMathCore -lRint -lNet -o service


clean:
	rm -f reveNtuple VsdDict.cc VsdDict_rdict.pcm libVsdDict.so mt_read mt_write libVsdDictAMT.so

### VsdTree and dicts

VsdDict.cc VsdDict_rdict.pcm &: VsdBase.h Vsd_Linkdef.h
	rootcling -I. -f VsdDict.cc VsdBase.h Vsd_Linkdef.h

libVsdDict.so: VsdDict.cc
	c++ -shared -fPIC -o libVsdDict.so ${ROOT_CFLAGS} VsdDict.cc

libVsdDictAMT.so: VsdDict.cc
	c++ -shared -fPIC -o libVsdDictAMT.so ${ROOT_CFLAGS} VsdDict.cc VsdTree.cc

mt_read: VsdTree.h VsdTree.cc VsdDict.cc
	c++ -DSTANDALONE_READ_TEST ${ROOT_CFLAGS} -g -O0 -std=c++1z `root-config --libs` -o $@ VsdTree.cc VsdDict.cc

mt_write: VsdTree.h VsdTree.cc VsdDict.cc
	c++ -DSTANDALONE_WRITE_TEST ${ROOT_CFLAGS} -g -O0 -std=c++1z `root-config --libs` -o $@ VsdTree.cc VsdDict.cc

nano_write: libVsdDictAMT.so
 	root.exe -e 'gSystem->Load("libVsdDictAMT.so")' rdf.C
