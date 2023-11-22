ROOT_CFLAGS := `root-config --cflags`
ROOT_LDFLAGS := -L`root-config --libdir`


clean:
	rm -f reveNtuple VsdDict.cc VsdDict_rdict.pcm libVsdDict.so mt_read mt_write libVsdNanoDict.so *.class

### VsdTree and dicts

VsdDict.cc VsdDict_rdict.pcm &: VsdBase.h Vsd_Linkdef.h
	rootcling -I. -f VsdDict.cc VsdBase.h Vsd_Linkdef.h

libVsdDict.so: VsdDict.cc
	c++ -shared -fPIC -o libVsdDict.so ${ROOT_CFLAGS} VsdDict.cc

mt_read: mt_test.cc VsdTree.h VsdTree.cc VsdDict.cc
	c++ -DSTANDALONE_READ_TEST ${ROOT_CFLAGS} -g -O0 -std=c++1z `root-config --libs` -o $@ mt_test.cc VsdTree.cc VsdDict.cc

mt_write: mt_test.cc VsdTree.h VsdTree.cc VsdDict.cc MyVsdTree.class
	c++ -DSTANDALONE_WRITE_TEST ${ROOT_CFLAGS} -g -O0 -std=c++1z `root-config --libs` -o $@ mt_test.cc VsdTree.cc VsdDict.cc

MyVsdTree.class: MyVsdTree.h
	cpp -DFOR_VSD_CODE MyVsdTree.h | sed '/^#/d' > MyVsdTree.class


### CMS Nano
sample:
	curl -O https://amraktad.web.cern.ch/amraktad/nano-CMSSW_11_0_0-RelValZTT-mcRun.root

MyVsdNanoTree.class: MyVsdNanoTree.h
	cpp -DFOR_VSD_CODE MyVsdNanoTree.h | sed '/^#/d' > MyVsdNanoTree.class

libVsdNanoDict.so: VsdDict.cc
	c++ -shared -fPIC -o libVsdNanoDict.so ${ROOT_CFLAGS} VsdDict.cc VsdTree.cc MyVsdNanoTree.h VsdRegisterBranch.h

# write nano vsd root file
vsd-nano.root: libVsdNanoDict.so MyVsdNanoTree.class
	root.exe -e 'gSystem->Load("libVsdNanoDict.so")' rdf.C

## run event display
evd: vsd-nano.root
	root.exe  -e 'gSystem->Load("libVsdNanoDict.so")' testevd.C


