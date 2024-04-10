import ROOT
import json

ROOT.gSystem.Load("libVsdDict.so")

Vfile = ROOT.TFile("UserVsd.root", "RECREATE")
Vtree = ROOT.TTree("VSD", "Custom plain VSD tree")

pcv = ROOT.std.vector('VsdCandidate')()
candBr = Vtree.Branch("PinkCands", pcv)
candCfg = {
   "filter" : "i.pt() > 1",
   "color" : ROOT.kViolet,
   "purpose" : "Candidate"
}
candBr.SetTitle(json.dumps(candCfg))

# resue cand vectir for ECAL
ecalBr = Vtree.Branch("ChargedCands", pcv)
ecalCfg = {
   "color" : ROOT.kRed,
   "filter" : "i.charge() != 0",
   "purpose" : "CaloTower"
}
ecalBr.SetTitle(json.dumps(ecalCfg))
# resue cand vectir for HCAL
hcalBr = Vtree.Branch("NeutralCands", pcv)
hcalCfg = {
   "color" : ROOT.kBlue,
   "filter" : "i.charge() > 0",
   "purpose" : "CaloTower"
}
hcalBr.SetTitle(json.dumps(hcalCfg))



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

vertv = ROOT.std.vector('VsdVertex')()
vertBr = Vtree.Branch("ErrVertex", vertv)

for i in range(10):

    pcv.clear()
    gjv.clear()
    tmv.clear()
    umv.clear()
    eiv.clear() # could be reused since it is a single object per event
    vertv.clear()

    for j in range(10 + ROOT.gRandom.Integer(11)):
        cnd = ROOT.VsdCandidate(
            ROOT.gRandom.Uniform(0.1, 20),
            ROOT.gRandom.Uniform(-2.5, 2.5),
            ROOT.gRandom.Uniform(-ROOT.TMath.Pi(), ROOT.TMath.Pi()),
            (1 if ROOT.gRandom.Rndm() > 0.5 else -1))
        cnd.name = f"Candidate_{j}"
        cnd.setPos(ROOT.gRandom.Uniform(0.1, 20),ROOT.gRandom.Uniform(0.1, 20), ROOT.gRandom.Uniform(0.1, 20))
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
        jet.setPos(ROOT.gRandom.Uniform(0.1, 20),ROOT.gRandom.Uniform(0.1, 20), ROOT.gRandom.Uniform(0.1, 20))
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
        muon.setPos(ROOT.gRandom.Uniform(0.1, 20),ROOT.gRandom.Uniform(0.1, 20), ROOT.gRandom.Uniform(0.1, 20))
        umv.push_back(muon)
    
    ei = ROOT.VsdEventInfo(333, 7777, i+1000)
    eiv.push_back(ei);

    for j in range(4):
        vert = ROOT.VsdVertex(
            ROOT.gRandom.Uniform(0.1, 0.10),
            ROOT.gRandom.Uniform(-0.5, 0.5),
            ROOT.gRandom.Uniform(-2.5, 2.5))
        vert.name = f"Vertex_{j}"
        vert.setErr(0, 0, ROOT.gRandom.Uniform(5 * 0.001, 2* 0.001))
        vert.setErr(1, 1, ROOT.gRandom.Uniform(5 * 0.001, 2* 0.001))
        vert.setErr(2, 2, ROOT.gRandom.Uniform(5 * 0.001, 2* 0.001))

        # vert.setErr(0, 1, ROOT.gRandom.Uniform(1.5 * 0.001, 2* 0.001))
       # vert.setErr(1, 0, vert.getErr(0,1))
       # vert.setErr(0, 2, ROOT.gRandom.Uniform(1.5 * 0.001, 2* 0.001))
        #vert.setErr(2, 0, vert.getErr(0,2))
        #vert.setErr(1, 2, ROOT.gRandom.Uniform(1.5 * 0.001, 2* 0.001))
        #vert.setErr(2, 1, vert.getErr(1,2))
        vertv.push_back(vert)

    ei = ROOT.VsdEventInfo(333, 7777, i+1000)
    eiv.push_back(ei);

    Vtree.Fill()

# Save the TTree to a file and close it
Vtree.Write()
Vfile.Close()
