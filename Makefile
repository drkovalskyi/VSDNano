ROOT_CFLAGS := `root-config --cflags`
ROOT_LDFLAGS := -L`root-config --libdir`

all: myvsd nanovsd

myvsd: mt_write mt_read MyVsdTree.class vsd-nano.root
	@echo Now you can run "make evd"

nanovsd: libVsdNanoDict.so MyVsdNanoTree.class

clean:
	rm -f reveNtuple VsdDict.cc VsdDict_rdict.pcm libVsdDict.so
	rm -f mt_read mt_write libVsdNanoDict.so *.class

### VsdTree and dicts

VsdDict.cc VsdDict_rdict.pcm &: VsdBase.h Vsd_Linkdef.h
	rootcling -I. -f VsdDict.cc VsdBase.h Vsd_Linkdef.h

libVsdDict.so: VsdDict.cc
	c++ -shared -fPIC -o libVsdDict.so ${ROOT_CFLAGS} VsdDict.cc

mt_read: mt_test.cc VsdTree.h VsdTree.cc VsdDict.cc
	c++ -DSTANDALONE_READ_TEST ${ROOT_CFLAGS} -g -O0 -std=c++1z `root-config --libs` -o $@ mt_test.cc VsdTree.cc VsdDict.cc

mt_write: mt_test.cc VsdTree.h VsdTree.cc VsdDict.cc MyVsdTree.class
	c++ -DSTANDALONE_WRITE_TEST ${ROOT_CFLAGS} -g -O0 -std=c++1z `root-config --libs` -o $@ mt_test.cc VsdTree.cc VsdDict.cc

mt_user: mt_test.cc VsdTree.h VsdTree.cc VsdDict.cc UserVsd.root
	c++ -DUSER_VSD_READ_TEST ${ROOT_CFLAGS} -g -O0 -std=c++1z `root-config --libs` -o $@ mt_test.cc VsdTree.cc VsdDict.cc

mt_colproxy: mt_test.cc VsdTree.h VsdTree.cc VsdDict.cc UserVsd.root
	c++ -DUSER_COLPROXY_READ_TEST ${ROOT_CFLAGS} -g -O0 -std=c++1z `root-config --libs` -o $@ mt_test.cc VsdTree.cc VsdDict.cc

MyVsdTree.class: MyVsdTree.h
	cpp -DFOR_VSD_CODE MyVsdTree.h | sed '/^#/d' > MyVsdTree.class

vsd.root: mt_write
	./mt_write

mt_evd: vsd.root
	root.exe  -e 'gSystem->Load("libVsdDict.so")' testevd.C'("vsd.root")'

UserVsd.root: UserVsd.py
	python UserVsd.py

### CMS Nano

NANO_ROOT := nano-CMSSW_11_0_0-RelValZTT-mcRun.root

sample: ${NANO_ROOT}
${NANO_ROOT}:
	curl -O https://amraktad.web.cern.ch/amraktad/${NANO_ROOT}

## run event display
evd: UserVsd.root libVsdDict.so
	root.exe  -e 'gSystem->Load("libVsdDict.so")' 'evd.h("UserVsd.root")'
service:
	c++ `root-config --cflags` -fPIC  service.cc -L`root-config --libdir` `root-config --libs` -lROOTEve -lROOTWebDisplay -lGeom -lCore -lRIO -lMathCore -lRint -lNet -o service
