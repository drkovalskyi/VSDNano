#include "VSDBase.h"

#include "TROOT.h"
#include "ROOT/REveDataCollection.hxx"
#include "ROOT/REveDataSimpleProxyBuilderTemplate.hxx"
#include "ROOT/REveManager.hxx"
#include "ROOT/REveScalableStraightLineSet.hxx"
#include "ROOT/REveViewContext.hxx"
#include <ROOT/REveGeoShape.hxx>
#include <ROOT/REveJetCone.hxx>
#include <ROOT/REvePointSet.hxx>
#include <ROOT/REveProjectionBases.hxx>
#include <ROOT/REveProjectionManager.hxx>
#include <ROOT/REveScene.hxx>
#include <ROOT/REveTableProxyBuilder.hxx>
#include <ROOT/REveTableInfo.hxx>
#include <ROOT/REveTrack.hxx>
#include <ROOT/REveTrackPropagator.hxx>
#include <ROOT/REveViewer.hxx>
#include <ROOT/REveViewContext.hxx>
#include "ROOT/REveStraightLineSet.hxx"
#include "ROOT/REveTrans.hxx"
#include "ROOT/REveGeoShape.hxx"
#include "ROOT/REveBox.hxx"
#include "TGeoBBox.h"
#include "TGeoTube.h"

using namespace ROOT::Experimental;

//====================================================================================
REveGeoShape *getShape(const char *name,
                       TGeoBBox *shape,
                       Color_t color)
{
   REveGeoShape *egs = new REveGeoShape(name);
   TColor *c = gROOT->GetColor(color);
   Float_t rgba[4] = {1, 0, 0, 1};
   if (c)
   {
      rgba[0] = c->GetRed();
      rgba[1] = c->GetGreen();
      rgba[2] = c->GetBlue();
   }
   egs->SetMainColorRGB(rgba[0], rgba[1], rgba[2]);
   egs->SetShape(shape);
   return egs;
}

float EtaToTheta(float eta)
{
   using namespace TMath;

   if (eta < 0)
      return Pi() - 2 * ATan(Exp(-Abs(eta)));
   else
      return 2 * ATan(Exp(-Abs(eta)));
}

void addRhoZEnergyProjection(REveDataProxyBuilderBase *pb, REveElement *container,
                             double r_ecal, double z_ecal,
                             double theta_min, double theta_max,
                             double phi)
{
   REveGeoManagerHolder gmgr(REveGeoShape::GetGeoManager());
   double z1 = r_ecal / tan(theta_min);
   if (z1 > z_ecal)
      z1 = z_ecal;
   if (z1 < -z_ecal)
      z1 = -z_ecal;
   double z2 = r_ecal / tan(theta_max);
   if (z2 > z_ecal)
      z2 = z_ecal;
   if (z2 < -z_ecal)
      z2 = -z_ecal;
   double r1 = z_ecal * fabs(tan(theta_min));
   if (r1 > r_ecal)
      r1 = r_ecal;
   if (phi < 0)
      r1 = -r1;
   double r2 = z_ecal * fabs(tan(theta_max));
   if (r2 > r_ecal)
      r2 = r_ecal;
   if (phi < 0)
      r2 = -r2;

   if (fabs(r2 - r1) > 1)
   {
      TGeoBBox *sc_box = new TGeoBBox(0., fabs(r2 - r1) / 2, 1);
      REveGeoShape *element = new REveGeoShape("r-segment");
      element->SetShape(sc_box);
      REveTrans &t = element->RefMainTrans();
      t(1, 4) = 0;
      t(2, 4) = (r2 + r1) / 2;
      t(3, 4) = fabs(z2) > fabs(z1) ? z2 : z1;
      pb->SetupAddElement(element, container);
   }
   if (fabs(z2 - z1) > 1)
   {
      TGeoBBox *sc_box = new TGeoBBox(0., 1, (z2 - z1) / 2);
      REveGeoShape *element = new REveGeoShape("z-segment");
      element->SetShape(sc_box);
      REveTrans &t = element->RefMainTrans();
      t(1, 4) = 0;
      t(2, 4) = fabs(r2) > fabs(r1) ? r2 : r1;
      t(3, 4) = (z2 + z1) / 2;
      pb->SetupAddElement(element, container);
   }
}


//====================================================================================




class VertexProxyBuilder : public REveDataSimpleProxyBuilderTemplate<VSDVertex>
{
public:

   using REveDataSimpleProxyBuilderTemplate<VSDVertex>::BuildItem;
   virtual void BuildItem(const VSDVertex &iData, int iIndex, REveElement *iItemHolder, const REveViewContext *vc) override
   {
    /*
      reco::Vertex::Error e = iData.error();

      auto item = dynamic_cast<FWWebEventItem *>(Collection());
      auto context = fireworks::Context::getInstance();

      if (item->getConfig()->value<bool>("Draw Ellipse"))
      {
         TMatrixDSym xxx(3);
         for (int i = 0; i < 3; i++)
            for (int j = 0; j < 3; j++)
            {
               // printf("Read error [%d,%d] %g\n", i, j, e(i,j));
               xxx(i, j) = e(i, j);
            }
         //xxx.Print();

         TMatrixDEigen eig(xxx);
         TVectorD xxxEig(eig.GetEigenValues());
         //  xxxEig.Print();
         xxxEig = xxxEig.Sqrt();

         TMatrixD vecEig = eig.GetEigenVectors();
         // vecEig.Print();

         float scale = item->getConfig()->value<long>("Scale Ellipse");

         REveVector v[3];
         for (int i = 0; i < 3; ++i)
         {
            v[i].Set(vecEig(0, i), vecEig(1, i), vecEig(2, i));
            v[i] *= xxxEig(i) * scale;
         }
         REveEllipsoid *ell = new REveEllipsoid("VertexError");
         ell->RefMainTrans().SetPos(iData.x(), iData.y(), iData.z());
         ell->SetLineWidth(2);
         ell->SetBaseVectors(v[0], v[1], v[2]);
         ell->Outline();
         SetupAddElement(ell, iItemHolder);
      }*/

      // vertex position
      //
      auto ps = new REvePointSet("vertex pnt");
      ps->SetMainColor(kGreen + 10);
      ps->SetNextPoint(iData.x(), iData.y(), iData.z());
      ps->SetMarkerStyle(4);
      ps->SetMarkerSize(4);
      SetupAddElement(ps, iItemHolder );

/*
      // tracks
      if (item->getConfig()->value<bool>("Draw Tracks"))
      {
         for(reco::Vertex::trackRef_iterator it = iData.tracks_begin() ;
             it != iData.tracks_end()  ; ++it)
         {
            float w = iData.trackWeight(*it);
            if (w < 0.5) continue;
      
            const reco::Track & track = *it->get();
            REveRecTrack t;
            t.fBeta = 1.;
            t.fV = REveVector(track.vx(), track.vy(), track.vz());
            t.fP = REveVector(track.px(), track.py(), track.pz());
            t.fSign = track.charge();
            REveTrack* trk = new REveTrack(&t, context->getTrackPropagator());
            trk->MakeTrack(); 
            SetupAddElement(trk, iItemHolder);
         }
      }
      if (item->getConfig()->value<bool>("Draw Pseudo Track")) {
         REveRecTrack t;
         t.fBeta = 1.;
         t.fV = REveVector(iData.x(), iData.y(), iData.z());
         t.fP = REveVector(-iData.p4().px(), -iData.p4().py(), -iData.p4().pz());
         t.fSign = 1;
         REveTrack* trk = new REveTrack(&t, context->getTrackPropagator());
         trk->SetLineStyle(7);
         trk->MakeTrack();
         SetupAddElement(trk, iItemHolder);
      }
   */
   }
};


//====================================================================================

class METProxyBuilder : public REveDataSimpleProxyBuilderTemplate<VSDMET>
{
public:
   virtual bool HaveSingleProduct() const override { return false; }

   using REveDataSimpleProxyBuilderTemplate<VSDMET>::BuildItemViewType;
   virtual void BuildItemViewType(const VSDMET &met, int /*idx*/, ROOT::Experimental::REveElement *iItemHolder,
                                  const std::string &viewType, const REveViewContext *context) override
   {
      using namespace TMath;
      double phi = met.phi();
      double theta = EtaToTheta(met.eta());
      //double theta = 0.f;
      double size = 1.f;

      REveScalableStraightLineSet *marker = new REveScalableStraightLineSet("MET marker");
      marker->SetLineWidth(2);
      marker->SetAlwaysSecSelect(false);

      SetupAddElement(marker, iItemHolder);

      float offr = 5;
      float r_ecal = context->GetMaxR() + offr;
      float z_ecal = context->GetMaxZ() + offr;
      float energyScale = 5.f;

      if (viewType.compare(0, 3, "Rho") == 0)
      {
         // body
         double r0;
         if (TMath::Abs(met.eta()) < abs(atan(r_ecal / z_ecal)))
         {
            r0 = r_ecal / sin(theta);
         }
         else
         {
            r0 = z_ecal / fabs(cos(theta));
         }
         marker->SetScaleCenter(0., Sign(r0 * sin(theta), phi), r0 * cos(theta));
         double r1 = r0 + 1;
         marker->AddLine(0., Sign(r0 * sin(theta), phi), r0 * cos(theta),
                         0., Sign(r1 * sin(theta), phi), r1 * cos(theta));
         // arrow pointer
         double r2 = r1 - 0.1;
         double dy = 0.05 * size;
         marker->AddLine(0., Sign(r2 * sin(theta) + dy * cos(theta), phi), r2 * cos(theta) - dy * sin(theta),
                         0., Sign(r1 * sin(theta), phi), r1 * cos(theta));
         dy = -dy;
         marker->AddLine(0., Sign(r2 * sin(theta) + dy * cos(theta), phi), r2 * cos(theta) - dy * sin(theta),
                         0., Sign(r1 * sin(theta), phi), r1 * cos(theta));

         // segment
         addRhoZEnergyProjection(this, iItemHolder, r_ecal - 1, z_ecal - 1,
                                 theta - 0.04, theta + 0.04,
                                 phi);
      }
      else
      {
         // body
         double r0 = r_ecal;
         double r1 = r0 + 1;
         marker->SetScaleCenter(r0 * cos(phi), r0 * sin(phi), 0);
         marker->AddLine(r0 * cos(phi), r0 * sin(phi), 0,
                         r1 * cos(phi), r1 * sin(phi), 0);

         // arrow pointer, xy  rotate offset point ..
         double r2 = r1 - 0.1;
         double dy = 0.05 * size;

         marker->AddLine(r2 * cos(phi) - dy * sin(phi), r2 * sin(phi) + dy * cos(phi), 0,
                         r1 * cos(phi), r1 * sin(phi), 0);
         dy = -dy;
         marker->AddLine(r2 * cos(phi) - dy * sin(phi), r2 * sin(phi) + dy * cos(phi), 0,
                         r1 * cos(phi), r1 * sin(phi), 0);

         // segment
         double min_phi = phi - M_PI / 36 / 2;
         double max_phi = phi + M_PI / 36 / 2;
         REveGeoManagerHolder gmgr(REveGeoShape::GetGeoManager());
         REveGeoShape *element = getShape("spread", new TGeoTubeSeg(r0 - 2, r0, 1, min_phi * 180 / M_PI, max_phi * 180 / M_PI), 0);
         element->SetPickable(kTRUE);
         SetupAddElement(element, iItemHolder);
      }
      // float value = met.et();
      float value = met.pt();
      marker->SetScale(energyScale * value);
   }
};

//====================================================================================

class CandidateProxyBuilder : public REveDataSimpleProxyBuilderTemplate<VSDCandidate>
{
   using REveDataSimpleProxyBuilderTemplate<VSDCandidate>::BuildItem;
   void BuildItem(const VSDCandidate &el, int /*idx*/, REveElement *iItemHolder, const REveViewContext *context) override
   {
      VSDCandidate &cand = (VSDCandidate &)(el); // amt need a const
      // int pdg = 11 * cand.charge();

      float theta = EtaToTheta(cand.eta());
      float pz = cand.pt() / TMath::Tan(theta);
      float px = cand.pt() * TMath::Cos(cand.phi());
      float py = cand.pt() * TMath::Sin(cand.phi());

      REveRecTrack t;
      t.fBeta = 1.;
      t.fV = REveVector(); // iData.vx(), iData.vy(), iData.vz());
      t.fP = REveVector(px, py, pz);
      t.fSign = cand.m_charge;
      REveTrack *track = new REveTrack(&t, context->GetPropagator());

      // printf("==============  BUILD track %s (pt=%f, eta=%f) \n", iItemHolder->GetCName(), p.Pt(), p.Eta());
      // auto track = new REveTrack((TParticle *)(x), 1, context->GetPropagator());
      track->MakeTrack();
      SetupAddElement(track, iItemHolder, true);
      // track->SetName(Form("element %s id=%d", iItemHolder->GetCName(), track->GetElementId()));
   }
};

//====================================================================================

class MuonProxyBuilder : public REveDataSimpleProxyBuilderTemplate<VSDMuon>
{
private:
   REveTrackPropagator *muonPropagator_g = nullptr;

public:
   void initMuonPropagator()
   {
      if (muonPropagator_g)
         return;
      // AMT this is ugly ... introduce a global contenxt
      muonPropagator_g = new REveTrackPropagator();
      muonPropagator_g->SetMagFieldObj(new REveMagFieldDuo(350, -3.5, 2.0));
      muonPropagator_g->SetMaxR(850);
      muonPropagator_g->SetMaxZ(1100);
      muonPropagator_g->SetMaxOrbs(6);
      muonPropagator_g->IncRefCount();
   }

   using REveDataSimpleProxyBuilderTemplate<VSDMuon>::BuildItem;
   void BuildItem(const VSDMuon &muon, int /*idx*/, REveElement *iItemHolder, const REveViewContext *context) override
   {
      initMuonPropagator();
      float theta = EtaToTheta(muon.eta());
      float pz = muon.pt() / TMath::Tan(theta);
      float px = muon.pt() * TMath::Cos(muon.phi());
      float py = muon.pt() * TMath::Sin(muon.phi());

      REveRecTrack t;
      t.fBeta = 1.;
      t.fV = REveVector(); // iData.vx(), iData.vy(), iData.vz());
      t.fP = REveVector(px, py, pz);
      t.fSign = muon.m_charge;

      auto track = new REveTrack(&t, muonPropagator_g);
      track->MakeTrack();
      track->SetLineWidth(2);

      SetupAddElement(track, iItemHolder, true);
      // track->SetName(Form("element %s id=%d", iItemHolder->GetCName(), track->GetElementId()));
   }
};
//====================================================================================
class JetProxyBuilder : public REveDataSimpleProxyBuilderTemplate<VSDJet>
{
   struct Cell
   {
      float thetaMin;
      float thetaMax;
      float phiMin;
      float phiMax;
   };

   bool HaveSingleProduct() const override { return false; }

   void makeBarrelCell(Cell &cellData, float &offset, float towerH, float *pnts)
   {
      using namespace TMath;

      float r1 = offset;
      float r2 = r1 + towerH * Sin(cellData.thetaMin);
      float z1In, z1Out, z2In, z2Out;

      z1In = r1 / Tan(cellData.thetaMax);
      z1Out = r2 / Tan(cellData.thetaMax);
      z2In = r1 / Tan(cellData.thetaMin);
      z2Out = r2 / Tan(cellData.thetaMin);

      float cos1 = Cos(cellData.phiMin);
      float sin1 = Sin(cellData.phiMin);
      float cos2 = Cos(cellData.phiMax);
      float sin2 = Sin(cellData.phiMax);

      // 0
      pnts[0] = r1 * cos2;
      pnts[1] = r1 * sin2;
      pnts[2] = z1In;
      pnts += 3;
      // 1
      pnts[0] = r1 * cos1;
      pnts[1] = r1 * sin1;
      pnts[2] = z1In;
      pnts += 3;
      // 2
      pnts[0] = r1 * cos1;
      pnts[1] = r1 * sin1;
      pnts[2] = z2In;
      pnts += 3;
      // 3
      pnts[0] = r1 * cos2;
      pnts[1] = r1 * sin2;
      pnts[2] = z2In;
      pnts += 3;
      //---------------------------------------------------
      // 4
      pnts[0] = r2 * cos2;
      pnts[1] = r2 * sin2;
      pnts[2] = z1Out;
      pnts += 3;
      // 5
      pnts[0] = r2 * cos1;
      pnts[1] = r2 * sin1;
      pnts[2] = z1Out;
      pnts += 3;
      // 6
      pnts[0] = r2 * cos1;
      pnts[1] = r2 * sin1;
      pnts[2] = z2Out;
      pnts += 3;
      // 7
      pnts[0] = r2 * cos2;
      pnts[1] = r2 * sin2;
      pnts[2] = z2Out;

      offset += towerH * Sin(cellData.thetaMin);

   } // end RenderBarrelCell

   void makeEndCapCell(Cell &cellData, float &offset, float towerH, float *pnts)
   {
      using namespace TMath;
      float z1, r1In, r1Out, z2, r2In, r2Out;

      z1 = offset;
      z2 = z1 + towerH;

      r1In = z1 * Tan(cellData.thetaMin);
      r2In = z2 * Tan(cellData.thetaMin);
      r1Out = z1 * Tan(cellData.thetaMax);
      r2Out = z2 * Tan(cellData.thetaMax);

      float cos2 = Cos(cellData.phiMin);
      float sin2 = Sin(cellData.phiMin);
      float cos1 = Cos(cellData.phiMax);
      float sin1 = Sin(cellData.phiMax);

      // 0
      pnts[0] = r1In * cos1;
      pnts[1] = r1In * sin1;
      pnts[2] = z1;
      pnts += 3;
      // 1
      pnts[0] = r1In * cos2;
      pnts[1] = r1In * sin2;
      pnts[2] = z1;
      pnts += 3;
      // 2
      pnts[0] = r2In * cos2;
      pnts[1] = r2In * sin2;
      pnts[2] = z2;
      pnts += 3;
      // 3
      pnts[0] = r2In * cos1;
      pnts[1] = r2In * sin1;
      pnts[2] = z2;
      pnts += 3;
      //---------------------------------------------------
      // 4
      pnts[0] = r1Out * cos1;
      pnts[1] = r1Out * sin1;
      pnts[2] = z1;
      pnts += 3;
      // 5
      pnts[0] = r1Out * cos2;
      pnts[1] = r1Out * sin2;
      pnts[2] = z1;
      pnts += 3;
      // 6
      pnts[0] = r2Out * cos2;
      pnts[1] = r2Out * sin2;
      pnts[2] = z2;
      pnts += 3;
      // 7
      pnts[0] = r2Out * cos1;
      pnts[1] = r2Out * sin1;
      pnts[2] = z2;

      if (z1 > 0)
         offset += towerH * Cos(cellData.thetaMin);
      else
         offset -= towerH * Cos(cellData.thetaMin);

   } // end makeEndCapCell

   using REveDataSimpleProxyBuilderTemplate<VSDJet>::BuildItemViewType;
   void BuildItemViewType(const VSDJet &dj, int idx, REveElement *iItemHolder,
                          const std::string &viewType, const REveViewContext *context) override
   {
      auto jet = new REveJetCone();
      jet->SetCylinder(context->GetMaxR() - 5, context->GetMaxZ());
      jet->AddEllipticCone(dj.eta(), dj.phi(), dj.coneR(), dj.coneR());
      SetupAddElement(jet, iItemHolder, true);
      jet->SetTitle(Form("jet [%d] pt = %f\n", idx, dj.pt()));
      // printf("make jet %d pt == %f\n", idx, dj.pt());
      //printf("make xxx jet %d pt == %f\n", idx, xxx->pt());

      static const float_t offr = 5;
      float r_ecal = context->GetMaxR() + offr;
      float z_ecal = context->GetMaxZ() + offr;
      float energyScale = 5.f;
      float transAngle = abs(atan(r_ecal / z_ecal));
      double theta = EtaToTheta(dj.eta());
      double phi = dj.phi();

      Cell cell;
      // hardcoded cell size
      float ad = 0.02;
      // thetaMin => etaMax, thetaMax => thetaMin
      cell.thetaMax = EtaToTheta(dj.eta() - ad);
      cell.thetaMin = EtaToTheta(dj.eta() + ad);
      cell.phiMin = phi - ad;
      cell.phiMax = phi + ad;
      float pnts[24];

      // an example of slices
      std::vector<float> sliceVals;
      sliceVals.push_back(dj.pt() * (1 - dj.hadFraction()));
      sliceVals.push_back(dj.pt() * dj.hadFraction());

      if (theta < transAngle || (3.14 - theta) < transAngle)
      {
         float offset = TMath::Sign(context->GetMaxZ(), dj.eta());
         for (auto &val : sliceVals)
         {
            offset += TMath::Sign(offr, dj.eta());
            makeEndCapCell(cell, offset, TMath::Sign(val * energyScale, dj.eta()), &pnts[0]);
            REveBox *reveBox = new REveBox();
            reveBox->SetVertices(pnts);
            SetupAddElement(reveBox, iItemHolder, true);
         }
      }
      else
      {
         float offset = context->GetMaxR();
         for (auto &val : sliceVals)
         {
            offset += offr;
            makeBarrelCell(cell, offset, val * energyScale, &pnts[0]);
            REveBox *reveBox = new REveBox();
            reveBox->SetVertices(pnts);
            SetupAddElement(reveBox, iItemHolder, true);
            reveBox->SetTitle(Form("jet %d", idx)); // amt this is workaround and should be unnecessary
         }
      }
   }

   using REveDataProxyBuilderBase::LocalModelChanges;

   void LocalModelChanges(int idx, REveElement *el, const REveViewContext *ctx) override
   {
      printf("LocalModelChanges jet %s ( %s )\n", el->GetCName(), el->FirstChild()->GetCName());
      REveJetCone *cone = dynamic_cast<REveJetCone *>(el->FirstChild());
      cone->SetLineColor(cone->GetMainColor());
   }
};

//==============================================================================

//==============================================================================

//==============================================================================
