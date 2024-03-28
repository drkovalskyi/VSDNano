import ROOT
import sys
from DataFormats.FWLite import Events, Handle
from math import *

events = Events ([
    '/afs/cern.ch/work/d/dmytro/projects/Run2024-Trigger-140/src/outputScoutingPF.root'
])

# VSD setup
ROOT.gSystem.Load("libVsdDict.so")

Vfile = ROOT.TFile("hltscout.root", "RECREATE")
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
muonHandle, muonLabel = Handle("std::vector<Run3ScoutingMuon>"), "hltScoutingMuonPackerVtx"
jetHandle, jetLabel = Handle("std::vector<Run3ScoutingPFJet>"), "hltScoutingPFPacker"
pfHandle, pfLabel = Handle("std::vector<Run3ScoutingParticle>"), "hltScoutingPFPacker"

# Functions to convert from scouting hw values to physical quantities

def save_event():
    # Save Muons
    umv.clear()
    for i, muon in enumerate(muonCollection):
        vsd_muon = ROOT.VsdMuon(muon.pt(), muon.eta(), muon.phi(), muon.charge())  
        vsd_muon.name = f"Muon_{i}"
        umv.push_back(vsd_muon)
		
    # Save Jets
    gjv.clear()
    for i, jet in enumerate(jetCollection):
        p4 = ROOT.ROOT.Math.PtEtaPhiMVector(jet.pt(), jet.eta(), jet.phi(), 0)
        had_energy = jet.chargedHadronEnergy() + jet.neutralHadronEnergy()
        vsd_jet = ROOT.VsdJet(jet.pt(), jet.eta(), jet.phi(), 0,
                              1.0 if had_energy > p4.E() else had_energy / p4.E(),
                              sqrt(jet.jetArea()/2/pi))
        vsd_jet.name = f"Jet_{i}"
        gjv.push_back(vsd_jet)

    # Save Tracks
    pcv.clear()
    for pf in pfCollection:
        if abs(pf.pdgId()) != 211: continue
        vsd_trk = ROOT.VsdCandidate(pf.pt(), pf.eta(), pf.phi(), 1 if pf.pdgId() > 0 else -1)
        vsd_trk.name = "Track_%u" % len(pcv)
        pcv.push_back(vsd_trk)
		
    ei = ROOT.VsdEventInfo(event.eventAuxiliary().run(),
                           event.eventAuxiliary().luminosityBlock(),
                           event.eventAuxiliary().event())
    eiv.push_back(ei);
    
    Vtree.Fill()

# loop over events
for event_count, event in enumerate(events):
    if event_count > 100: break

    event.getByLabel(muonLabel, muonHandle)
    muonCollection = muonHandle.product()

    event.getByLabel(jetLabel, jetHandle)
    jetCollection = jetHandle.product()

    event.getByLabel(pfLabel, pfHandle)
    pfCollection = pfHandle.product()

    save_event()


Vfile.cd()
Vtree.Write()
Vfile.Close()
