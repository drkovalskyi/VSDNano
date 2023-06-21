download: nano-CMSSW_11_0_0-RelValZTT-mcRun.root

nano-CMSSW_11_0_0-RelValZTT-mcRun.root:
	wget http://amraktad.web.cern.ch/amraktad/nano-CMSSW_11_0_0-RelValZTT-mcRun.root

VSDReader.h:
	root.exe makeClass.C

testShort: VSDReader.h
	root.exe TVSDProvider.C cms_nano_aod_bootstrap.C evd.C

evd: VSDReader.h download
	root.exe cms_nano_aod_bootstrap.C evd.C

clean:
	rm VSDReader.h VSDReader.C
