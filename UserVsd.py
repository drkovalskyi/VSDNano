import ROOT
import json

ROOT.gSystem.Load("libVsdDict.so")

Vfile = ROOT.TFile("UserVsd-2.root", "RECREATE")
Vtree = ROOT.TTree("VSD", "Custom plain VSD tree")

pcv = ROOT.std.vector('VsdCandidate')()
Vtree.Branch("PinkCands", pcv)

gjv = ROOT.std.vector('VsdJet')()
Vtree.Branch("GreenJets", gjv)

tmv = ROOT.std.vector('VsdMET')()
Vtree.Branch("TestMETs", tmv)


umv = ROOT.std.vector('VsdMuon')()
mb = Vtree.Branch("UMuon", umv)
muonCfg = {
   "filter" : "i.pt() > 1",
   "color" : ROOT.kViolet
}
mb.SetTitle(json.dumps(muonCfg))



eiv = ROOT.std.vector('VsdEventInfo')()
Vtree.Branch("EventInfo", eiv)


for i in range(10):

    pcv.clear()
    gjv.clear()
    tmv.clear()
    umv.clear()
    eiv.clear() # could be reused since it is a single object per event

    for j in range(10 + ROOT.gRandom.Integer(11)):
        cnd = ROOT.VsdCandidate(
            ROOT.gRandom.Uniform(0.1, 20),
            ROOT.gRandom.Uniform(-2.5, 2.5),
            ROOT.gRandom.Uniform(-ROOT.TMath.Pi(), ROOT.TMath.Pi()),
            (1 if ROOT.gRandom.Rndm() > 0.5 else -1))
        cnd.name = f"Candidate_{j}"
        pcv.push_back(cnd)

    for j in range(3 + ROOT.gRandom.Integer(6)):
        jet = ROOT.VsdJet(
            ROOT.gRandom.Uniform(0.1, 20),
            ROOT.gRandom.Uniform(-2.5, 2.5),
            ROOT.gRandom.Uniform(-ROOT.TMath.Pi(), ROOT.TMath.Pi()),
            (1 if ROOT.gRandom.Rndm() > 0.5 else -1),
            ROOT.gRandom.Uniform(0.1, 0.9),
            ROOT.gRandom.Uniform(0.05, 1))
        jet.name = f"Jet_{j}"
        gjv.push_back(jet)

    for j in range( 1 ):
        met = ROOT.VsdMET(
            ROOT.gRandom.Uniform(0.1, 20),
            ROOT.gRandom.Uniform(-2.5, 2.5),
            ROOT.gRandom.Uniform(-ROOT.TMath.Pi(), ROOT.TMath.Pi()),
            ROOT.gRandom.Uniform(0.1, 20) +10)
        met.name = f"MET_{j}"
        tmv.push_back(met)

    for j in range(5):
        muon = ROOT.VsdMuon(
            ROOT.gRandom.Uniform(0.1, 20),
            ROOT.gRandom.Uniform(-2.5, 2.5),
            ROOT.gRandom.Uniform(-ROOT.TMath.Pi(), ROOT.TMath.Pi()),
            (1 if ROOT.gRandom.Rndm() > 0.5 else -1))
        muon.name = f"Muon_{j}"
        umv.push_back(muon)
    
    ei = ROOT.VsdEventInfo(333, 7777, i+1000)
    eiv.push_back(ei);

    Vtree.Fill()

# Save the TTree to a file and close it
Vtree.Write()
Vfile.Close()
