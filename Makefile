ROOT_CFLAGS := `root-config --cflags`
ROOT_LDFLAGS := -L`root-config --libdir`

sample:
	curl -O https://amraktad.web.cern.ch/amraktad/nano-CMSSW_11_0_0-RelValZTT-mcRun.root

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

mt_read: mt_test.cc VsdTree.h VsdTree.cc VsdDict.cc
	c++ -DSTANDALONE_READ_TEST ${ROOT_CFLAGS} -g -O0 -std=c++1z `root-config --libs` -o $@ mt_test.cc VsdTree.cc VsdDict.cc

mt_write: mt_test.cc VsdTree.h VsdTree.cc VsdDict.cc
	c++ -DSTANDALONE_WRITE_TEST ${ROOT_CFLAGS} -g -O0 -std=c++1z `root-config --libs` -o $@ mt_test.cc VsdTree.cc VsdDict.cc

MyVsdTree.class: MyVsdTree.h
	cpp -DFOR_VSD_CODE MyVsdTree.h | sed '/^#/d' > MyVsdTree.class

nano_write: libVsdDictAMT.so
	root.exe -e 'gSystem->Load("libVsdDictAMT.so")' rdf.C

evd: vsd-nano.root
	root.exe testevd.C


