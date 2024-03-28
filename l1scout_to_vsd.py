import ROOT
import sys
from DataFormats.FWLite import Events, Handle
from math import *

events = Events ([
	'/eos/cms/store/data/Run2023C/L1ScoutUGMTCALO/RAW/v1/000/368/636/00000/run368636_ls0400.root'
])

# VSD setup
ROOT.gSystem.Load("libVsdDict.so")

Vfile = ROOT.TFile("l1scout.root", "RECREATE")
Vtree = ROOT.TTree("VSD", "L1 Scouting")

pcv = ROOT.std.vector('VsdCandidate')()
Vtree.Branch("PinkCands", pcv)

gjv = ROOT.std.vector('VsdJet')()
Vtree.Branch("GreenJets", gjv)

tmv = ROOT.std.vector('VsdMET')()
Vtree.Branch("TestMETs", tmv)

umv = ROOT.std.vector('VsdMuon')()
Vtree.Branch("UMuon", umv)

eiv = ROOT.std.vector('VsdEventInfo')()
Vtree.Branch("EventInfo", eiv)

# Scouting data
muonHandle, muonLabel = Handle("OrbitCollection<l1ScoutingRun3::Muon"),"GmtUnpacker"
jetHandle, jetLabel = Handle("OrbitCollection<l1ScoutingRun3::Jet"),"CaloUnpacker"

# Functions to convert from scouting hw values to physical quantities

def ugmtPt(hwPt):
	return 0.5 * (hwPt - 1)

def ugmtEta(hwEta):
	return 0.0870 / 8 * hwEta

def ugmtPhi(hwPhi):
	phi = 2. * pi / 576. * hwPhi
	if phi > pi:
		phi -= 2. * pi
	return phi

def demuxEt(hwEt):
	return 0.5 * hwEt

def demuxEta(hwEta):
	return 0.0435 * hwEta

def demuxPhi(hwPhi):
	phi = 0.0435 * hwPhi
	if phi > pi:
		phi -= 2. * pi
	return phi

def save_event(bx, muonCollection, jetCollection):
	# Save Muons
	umv.clear()
	nMuons = muonCollection.getBxSize(bx)
	for i in range(nMuons):
		muon = muonCollection.getBxObject(bx, i)
		vsd_muon = ROOT.VsdMuon(ugmtPt(muon.hwPt()), 
								ugmtEta(muon.hwEta()), 
								ugmtPhi(muon.hwPhi()), 
								muon.hwCharge())
		vsd_muon.name = f"Muon_{i}"
		umv.push_back(vsd_muon)
		
	# Save Jets
	gjv.clear()
	nJets = jetCollection.getBxSize(bx)
	for i in range(nJets):
		jet = jetCollection.getBxObject(bx, i)
		vsd_jet = ROOT.VsdJet(demuxEt(jet.hwEt()), 
							  demuxEta(jet.hwEta()), 
							  demuxPhi(muon.hwPhi()), 
							  0, 0.3, 0.3)
		vsd_jet.name = f"Jet_{i}"
		gjv.push_back(vsd_jet)

		
	ei = ROOT.VsdEventInfo(event.eventAuxiliary().run(),
						   event.eventAuxiliary().luminosityBlock(),
						   event.eventAuxiliary().event() * 10000 + bx)
	eiv.push_back(ei);
	
	Vtree.Fill()

# loop over events
for event in events:
	# if event.eventAuxiliary().event() != 69340364: continue

	event.getByLabel(muonLabel, muonHandle)
	muonCollection = muonHandle.product()

	event.getByLabel(jetLabel, jetHandle)
	jetCollection = jetHandle.product()

	filledBXs = muonCollection.getFilledBxs()
	for bx in filledBXs:
		nMuons = muonCollection.getBxSize(bx)
		nJets = jetCollection.getBxSize(bx)

		# select interesting events
		keep_event = False

		if nMuons > 0:
			for i in range(nMuons):
				muon = muonCollection.getBxObject(bx, i)
				p4 = ROOT.ROOT.Math.PtEtaPhiMVector(ugmtPt(muon.hwPt()), 
													ugmtEta(muon.hwEta()), 
													ugmtPhi(muon.hwPhi()), 
													0.10566)
				if p4.pt() > 6: 
					keep_event = True
					break
		
		if nJets > 0:
			for i in range(nJets):
				jet = jetCollection.getBxObject(bx, i)
				if demuxEt(jet.hwEt()) > 50: 
					keep_event = True
					break

		if keep_event:
			save_event(bx, muonCollection, jetCollection)
	break

Vfile.cd()
Vtree.Write()
Vfile.Close()

			
# Local Variables:
# indent-tabs-mode: 1
# tab-width: 4
# python-indent: 4
# End:
