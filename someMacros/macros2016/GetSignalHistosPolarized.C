// Usage:
// > root -b doAll.C

// C++
#include <iostream>
#include <vector>
#include <map>

// ROOT
#include "TBenchmark.h"
#include "TChain.h"
#include "TDirectory.h"
#include "TFile.h"
#include "TROOT.h"
#include "TTreeCache.h"
#include "TLorentzVector.h"
#include "TH3D.h"
#include "TH2F.h"

// CMS3
#include "CMS3.cc"

using namespace std;
using namespace tas;


float dRbetweenVectors(LorentzVector& vec1,LorentzVector& vec2 ){                                                                                                              
  float dphi = std::min(::fabs(vec1.Phi() - vec2.Phi()), 2 * M_PI - fabs(vec1.Phi() - vec2.Phi()));
  float deta = vec1.Eta() - vec2.Eta();

  return sqrt(dphi*dphi + deta*deta);
}

int ScanChain( TChain* chain, bool fast = true, int nEvents = -1, string skimFilePrefix = "test") {

  int T2tb_BRselection = -1;//1: T2tt, 2: mixed, 3: T2bW, -1: default
  int LR = +1;//-1: left-handed, +1: right-handed

  int countercheck = 1;
  //load PUweights
  TFile *fPU = new TFile("puWeights.root","READ");
  TH1D *puWeight     = (TH1D*)fPU->Get("puWeight");
  TH1D *puWeightUp   = (TH1D*)fPU->Get("puWeightUp");
  TH1D *puWeightDown = (TH1D*)fPU->Get("puWeightDown");

  TFile *fxsec = new TFile("xsec_stop_13TeV.root","READ");
  TH1D *hxsec     = (TH1D*)fxsec->Get("stop");

  TFile *f_el_SF       = new TFile("lepsf/kinematicBinSFele.root", "read");
  TFile *f_mu_SF_id    = new TFile("lepsf/TnP_MuonID_NUM_MediumID_DENOM_generalTracks_VAR_map_pt_eta.root", "read");
  TFile *f_mu_SF_iso   = new TFile("lepsf/TnP_MuonID_NUM_MiniIsoTight_DENOM_LooseID_VAR_map_pt_eta.root");
  TFile *f_mu_SF_veto_id  = new TFile("lepsf/TnP_MuonID_NUM_LooseID_DENOM_generalTracks_VAR_map_pt_eta.root", "read");
  TFile *f_mu_SF_veto_iso = new TFile("lepsf/TnP_MuonID_NUM_MiniIsoTight_DENOM_LooseID_VAR_map_pt_eta.root");
  //TFile *f_mu_SF_veto_iso = new TFile("lepsf/TnP_MuonID_NUM_MiniIsoLoose_DENOM_LooseID_VAR_map_pt_eta.root");
  //TFile *f_vetoLep_eff = new TFile("lepsf/lepeff__ttbar_powheg_pythia8_25ns.root", "read");
  TFile *f_vetoLep_eff = new TFile("lepsf/lepeff__ttbar_powheg_pythia8_25ns__SRcuts.root", "read");  
  TH2D *h_el_SF_id_temp      = (TH2D*)f_el_SF->Get("CutBasedMedium");
  TH2D *h_el_SF_iso_temp     = (TH2D*)f_el_SF->Get("MiniIso0p1_vs_AbsEta");
  TH2D *h_el_SF_veto_id_temp  = (TH2D*)f_el_SF->Get("CutBasedVeto");
  TH2D *h_el_SF_veto_iso_temp = (TH2D*)f_el_SF->Get("MiniIso0p4_vs_AbsEta");
  TH2D *h_mu_SF_id_temp      = (TH2D*)f_mu_SF_id->Get("pt_abseta_PLOT_pair_probeMultiplicity_bin0_&_tag_combRelIsoPF04dBeta_bin0_&_tag_pt_bin0_&_tag_IsoMu20_pass");
  TH2D *h_mu_SF_iso_temp     = (TH2D*)f_mu_SF_iso->Get("pt_abseta_PLOT_pair_probeMultiplicity_bin0_&_tag_combRelIsoPF04dBeta_bin0_&_tag_pt_bin0_&_PF_pass_&_tag_IsoMu20_pass");
  TH2D *h_mu_SF_veto_id_temp  = (TH2D*)f_mu_SF_veto_id->Get("pt_abseta_PLOT_pair_probeMultiplicity_bin0_&_tag_combRelIsoPF04dBeta_bin0_&_tag_pt_bin0_&_tag_IsoMu20_pass");
  TH2D *h_mu_SF_veto_iso_temp = (TH2D*)f_mu_SF_veto_iso->Get("pt_abseta_PLOT_pair_probeMultiplicity_bin0_&_tag_combRelIsoPF04dBeta_bin0_&_tag_pt_bin0_&_PF_pass_&_tag_IsoMu20_pass");
  //TH2D *h_el_vetoLepEff_temp = (TH2D*)f_vetoLep_eff->Get("h2_lepEff_vetoSel_Eff_el");
  //TH2D *h_mu_vetoLepEff_temp = (TH2D*)f_vetoLep_eff->Get("h2_lepEff_vetoSel_Eff_mu");
  TH2D *h_el_vetoLepEff_temp = (TH2D*)f_vetoLep_eff->Get("h2_lepEff_vetoSel_rebin_Eff_el");
  TH2D *h_mu_vetoLepEff_temp = (TH2D*)f_vetoLep_eff->Get("h2_lepEff_vetoSel_rebin_Eff_mu");
  TH2D *h_el_SF_id  = (TH2D*)h_el_SF_id_temp->Clone("h_el_SF_id");
  TH2D *h_el_SF_iso = (TH2D*)h_el_SF_iso_temp->Clone("h_el_SF_iso");
  TH2D *h_mu_SF_id  = (TH2D*)h_mu_SF_id_temp->Clone("h_mu_SF_id");
  TH2D *h_mu_SF_iso = (TH2D*)h_mu_SF_iso_temp->Clone("h_mu_SF_iso");
  TH2D *h_el_SF_veto_id  = (TH2D*)h_el_SF_veto_id_temp->Clone("h_el_SF_veto_id");
  TH2D *h_el_SF_veto_iso = (TH2D*)h_el_SF_veto_iso_temp->Clone("h_el_SF_veto_iso");
  TH2D *h_mu_SF_veto_id  = (TH2D*)h_mu_SF_veto_id_temp->Clone("h_mu_SF_veto_id");
  TH2D *h_mu_SF_veto_iso = (TH2D*)h_mu_SF_veto_iso_temp->Clone("h_mu_SF_veto_iso");
  //This is are the important ones
  TH2D *h_el_vetoLepEff = (TH2D*)h_el_vetoLepEff_temp->Clone("h_el_vetoLepEff");
  TH2D *h_mu_vetoLepEff = (TH2D*)h_mu_vetoLepEff_temp->Clone("h_mu_vetoLepEff");
  TH2D *h_el_SF = (TH2D*)h_el_SF_id->Clone("h_el_SF");
  h_el_SF->Multiply(h_el_SF_iso);
  TH2D *h_el_SF_veto = (TH2D*)h_el_SF_veto_id->Clone("h_el_SF_veto");
  TH2D *h_mu_SF = (TH2D*)h_mu_SF_id->Clone("h_mu_SF");
  h_mu_SF->Multiply(h_mu_SF_iso);
  TH2D *h_mu_SF_veto = (TH2D*)h_mu_SF_veto_id->Clone("h_mu_SF_veto");
  h_mu_SF_veto->Multiply(h_mu_SF_veto_iso);
  TFile *f_el_FS_ID       = new TFile("lepsf/sf_el_mediumCB.root", "read");
  TFile *f_el_FS_Iso      = new TFile("lepsf/sf_el_mini01.root", "read");
  TFile *f_mu_FS_ID       = new TFile("lepsf/sf_mu_mediumID.root", "read");
  TFile *f_mu_FS_Iso      = new TFile("lepsf/sf_mu_mini02.root", "read");
  TH2D *h_el_FS_ID_temp  = (TH2D*)f_el_FS_ID ->Get("histo2D");
  //TH2D *h_el_FS_ID       = (TH2D*)h_el_FS_ID_temp ->Clone("h_el_FS_ID");
  TH2D *h_el_FS          = (TH2D*)h_el_FS_ID_temp ->Clone("h_el_FS");
  TH2D *h_el_FS_Iso_temp = (TH2D*)f_el_FS_Iso->Get("histo2D");
  //TH2D *h_el_FS_Iso      = (TH2D*)h_el_FS_Iso_temp->Clone("h_el_FS_Iso");
  h_el_FS->Multiply(h_el_FS_Iso_temp);
  TH2D *h_mu_FS_ID_temp  = (TH2D*)f_mu_FS_ID ->Get("histo2D");
  //TH2D *h_mu_FS_ID       = (TH2D*)h_mu_FS_ID_temp ->Clone("h_mu_FS_ID");
  TH2D *h_mu_FS          = (TH2D*)h_mu_FS_ID_temp ->Clone("h_mu_FS");
  TH2D *h_mu_FS_Iso_temp = (TH2D*)f_mu_FS_Iso->Get("histo2D");
  //TH2D *h_mu_FS_Iso      = (TH2D*)h_mu_FS_Iso_temp->Clone("h_mu_FS_ID");
  h_mu_FS->Multiply(h_mu_FS_Iso_temp);

  
  // Benchmark
  TBenchmark *bmark = new TBenchmark();
  bmark->Start("benchmark");

  // Example Histograms
  TDirectory *rootdir = gDirectory->GetDirectory("Rint:");


  map<string, TH3D*> histos;//use D histos as weights can vary a lot among the signal
  vector<string> histonames; histonames.clear();
  map<string, TH3D*> histos2;//use D histos as weights can vary a lot among the signal
  vector<string> histonames2; histonames2.clear();
  //  vector<int> hbins; hbins.clear();
  //  vector<float> hlow; hlow.clear();
  //  vector<float> hup; hup.clear();

  //lumi, trigger, stats done
  histonames.push_back("SRyield");
  histonames.push_back("SR_Bup_HF");//done
  histonames.push_back("SR_Bdown_HF");
  histonames.push_back("SR_Bup_LF");//done
  histonames.push_back("SR_Bdown_LF");
  histonames.push_back("SR_JESup");
  histonames.push_back("SR_JESdown");
  histonames.push_back("SR_muRFup");
  histonames.push_back("SR_muRFdown");
  //histonames.push_back("SR_PDFup");
  //histonames.push_back("SR_PDFdown");
  histonames.push_back("SR_ISRup");//done preliminary
  histonames.push_back("SR_ISRdown");
  histonames.push_back("SR_PUup");//done preliminary
  histonames.push_back("SR_PUdown");
  histonames.push_back("SR_LepEffup");//done - I guess we need no renormalization - no fastsim in, no vetoSF
  histonames.push_back("SR_LepEffdown");
  histonames.push_back("SR_LepEffFSup");//done - I guess we need no renormalization - no fastsim in, no vetoSF
  histonames.push_back("SR_LepEffFSdown");
  histonames.push_back("SR_Xsecup");//done
  histonames.push_back("SR_Xsecdown");
  histonames.push_back("CR1l_sigcontamination");//scaled to signalreg yield
  histonames.push_back("CR2l_sigcontamination");//scaled to signalreg yield
  
  histonames2.push_back("eventsum");
  histonames2.push_back("rawweightsum");
  histonames2.push_back("totweightsum");
  histonames2.push_back("ISRsum");
  histonames2.push_back("BSFsum");
  histonames2.push_back("PUweightsum");
  histonames2.push_back("xsecsum");
  histonames2.push_back("nevtsum");
  histonames2.push_back("lepsum");
  histonames2.push_back("lepSFsum");
  histonames2.push_back("WLsum");
  histonames2.push_back("WRsum");
  
  for(unsigned int i = 0; i<histonames.size(); ++i){
    string mapname = histonames[i];
    if(histos.count(mapname) == 0 ) histos[mapname] = new TH3D(mapname.c_str(), "", 37,99,1024, 19,-1,474, 9, 0.5,9.5);
    //mStop 100-1000, mLSP 0-450, SR 1-12, 9200 bins, SR 0 is non-SR - in case it it needed!!
      histos[mapname]->Sumw2(); histos[mapname]->SetDirectory(rootdir);
    }
  for(unsigned int i = 0; i<histonames2.size(); ++i){
    string mapname = histonames2[i];
    if(histos2.count(mapname) == 0 ) histos2[mapname] = new TH3D(mapname.c_str(), "", 37,99,1024, 19,-1,474, 9, 0.5,9.5);
    //mStop 100-1000, mLSP 0-450, SR 1-12, 9200 bins, SR 0 is non-SR - in case it it needed!!
      histos2[mapname]->Sumw2(); histos2[mapname]->SetDirectory(rootdir);
    }
  
  
  // Loop over events to Analyze
  unsigned int nEventsTotal = 0;
  unsigned int nEventsChain = chain->GetEntries();
  if( nEvents >= 0 ) nEventsChain = nEvents;
  TObjArray *listOfFiles = chain->GetListOfFiles();
  TIter fileIter(listOfFiles);
  TFile *currentFile = 0;

  //get the reweighting histograms
  TIter fileIterFirst(listOfFiles);
  TFile *currentFileFirst = 0;
  TH3D* counterhistSig;
  TH2F* histNEvts;
  bool thisisfirst = true;
  // File Loop for adding correct histograms
  while ( (currentFileFirst = (TFile*)fileIterFirst.Next()) ) {
    TFile *file = new TFile( currentFileFirst->GetTitle() );
    file->cd();
    if(thisisfirst){
      counterhistSig = (TH3D*)file->Get("h_counterSMS");
      counterhistSig->SetDirectory(0); 
      histNEvts = (TH2F*)file->Get("histNEvts");
      histNEvts->SetDirectory(0);
      thisisfirst = false;
    } else {
      TH3D *tempcounterhistSig = (TH3D*)file->Get("h_counterSMS");
      tempcounterhistSig->SetDirectory(0); 
      TH2F *temphistNEvts = (TH2F*)file->Get("histNEvts");
      temphistNEvts->SetDirectory(0);
      counterhistSig->Add(tempcounterhistSig);
      histNEvts->Add(temphistNEvts);
      tempcounterhistSig->Delete();
      temphistNEvts->Delete();
    }
    file->Close();
    delete file;
  }

  // File Loop
  while ( (currentFile = (TFile*)fileIter.Next()) ) {

    // Get File Content
    TFile *file = new TFile( currentFile->GetTitle() );
    TTree *tree = (TTree*)file->Get("t");
    if(fast) TTreeCache::SetLearnEntries(10);
    if(fast) tree->SetCacheSize(128*1024*1024);
    cms3.Init(tree);
    
    // Loop over Events in current file
    if( nEventsTotal >= nEventsChain ) continue;
    unsigned int nEventsTree = tree->GetEntriesFast();
    for( unsigned int event = 0; event < nEventsTree; ++event) {
 
      // Get Event Content
      if( nEventsTotal >= nEventsChain ) continue;
      if(fast) tree->LoadTree(event);
      cms3.GetEntry(event);
      ++nEventsTotal;
    
      // Progress
      CMS3::progress( nEventsTotal, nEventsChain );

      // Analysis Code

      float mStop = mass_stop();
      float mLSP = mass_lsp();
      float mCharg = mass_chargino();
      //float xVal = mass_lsp();
      int Nevts = histNEvts->GetBinContent(histNEvts->FindBin(mStop,mLSP));
      double nevts = double(Nevts);
      //float weight = cms3.scale1fb()*2.11;
      double PUweight     = puWeight    ->GetBinContent(puWeight    ->FindBin(pu_ntrue() ) );
      double PUweightUp   = puWeightUp  ->GetBinContent(puWeightUp  ->FindBin(pu_ntrue() ) );
      double PUweightDown = puWeightDown->GetBinContent(puWeightDown->FindBin(pu_ntrue() ) );
      PUweightUp = 1; PUweightDown = PUweight; PUweight = 1; //now PU syst is applying vs not applying
     double ISRnorm = counterhistSig->GetBinContent(counterhistSig->FindBin(mStop,mLSP,19));
      double ISRnormup = counterhistSig->GetBinContent(counterhistSig->FindBin(mStop,mLSP,20));
      double ISRnormdown = counterhistSig->GetBinContent(counterhistSig->FindBin(mStop,mLSP,21));
      double ISRweight = weight_ISR();
      double BSFnorm = counterhistSig->GetBinContent(counterhistSig->FindBin(mStop,mLSP,14));
      double BSFnormHup = counterhistSig->GetBinContent(counterhistSig->FindBin(mStop,mLSP,15));
      double BSFnormLup = counterhistSig->GetBinContent(counterhistSig->FindBin(mStop,mLSP,16));
      double BSFnormHdown = counterhistSig->GetBinContent(counterhistSig->FindBin(mStop,mLSP,17));
      double BSFnormLdown = counterhistSig->GetBinContent(counterhistSig->FindBin(mStop,mLSP,18));
      double BSFweight = weight_btagsf();
      double muRFnorm = counterhistSig->GetBinContent(counterhistSig->FindBin(mStop,mLSP,1));
      double muRFnormup = counterhistSig->GetBinContent(counterhistSig->FindBin(mStop,mLSP,5));
      double muRFnormdown = counterhistSig->GetBinContent(counterhistSig->FindBin(mStop,mLSP,9));
      if(ISRnorm>0) ISRweight*=nevts/ISRnorm;
      if(ISRnorm<=0||ISRnormup<=0||ISRnormdown<=0){ ISRnormdown=1.; ISRnormup=1.; ISRnorm=1.;}
      if(ISRweight!=weight_ISR()) cout << "ISRw " << ISRweight << " wISR " << weight_ISR() << " nevts " << nevts << " ISRn " << ISRnorm << endl;
      if(BSFnorm>0) BSFweight *=nevts/BSFnorm;
      if(BSFnorm<=0||BSFnormHup<=0||BSFnormLup<=0||BSFnormHdown<=0||BSFnormLdown<=0){
	BSFnorm=1; BSFnormHup=1; BSFnormLup=1; BSFnormHdown=1; BSFnormLdown=1;
      }
      if(muRFnorm<=0||muRFnormup<=0||muRFnormdown<=0){ muRFnormdown=1; muRFnormup=1; muRFnorm=1; }
      //lepSF is done below
      double xsection = hxsec->GetBinContent(hxsec->FindBin(mStop));
      double xsectionerr = hxsec->GetBinError(hxsec->FindBin(mStop));
      //double rawweight = xsec()*2260./nevts;
      //double weight = xsec()*2260./nevts*PUweight*ISRweight*BSFweight;//xsec given in pb
      double rawweight = xsection*2260./nevts;
      double weight = xsection*2260./nevts*PUweight*ISRweight*BSFweight;//xsec given in pb
      //did put ISRweight which should be ==1
      if(ISRweight!=1) cout << "ISRw " << ISRweight << endl;
      if(event==0) cout << "weight " << weight << " nEvents " << nEventsTree << " filename " << currentFile->GetTitle() << endl;

      int NSLeps = 0;
      int NAddVetoLeps = 0;
      if(lep1_is_mu()){
	if(lep1_pt()>20&&fabs(lep1_eta())<2.4) {++NSLeps;}
      } else if (lep1_is_el()){
	if(lep1_pt()>20&&fabs(lep1_eta())<1.4442) {++NSLeps; }
      } if(lep2_is_mu()){
	if(lep2_pt()>20&&fabs(lep2_eta())<2.4) {++NSLeps;}
      } else if (lep2_is_el()){
	if(lep2_pt()>20&&fabs(lep2_eta())<1.4442) {++NSLeps; }
      }
      if(lep2_is_mu()){
	if(lep2_pt()>10&&fabs(lep2_eta())<2.4) {++NAddVetoLeps;}
      } else if (lep2_is_el()){
	if(lep2_pt()>10&&fabs(lep2_eta())<2.4) {++NAddVetoLeps; }
      }
      if(NSLeps<1) continue;//temp
      float lepSF_pt_cutoff = 100.0;
      float lepSF_pt_min    = 10.0;
      double lepSF    = 1.0;
      double lepSF_Up = 1.0;
      double lepSF_Dn = 1.0;
      float lepSF_FS_pt_cutoff = 200.0;
      double lepSF_FS    = 1.0;
      double lepSF_FS_Up = 1.0;
      double lepSF_FS_Dn = 1.0;	
      if(lep1_is_el()){
	int binX = h_el_SF->GetXaxis()->FindBin( std::min(lepSF_pt_cutoff, (float)lep1_p4().Pt()) );
	int binY = h_el_SF->GetYaxis()->FindBin( fabs(lep1_p4().Eta()) );
	lepSF    = h_el_SF->GetBinContent( binX, binY );
	lepSF_Up = lepSF + h_el_SF->GetBinError( binX, binY );
	lepSF_Dn = lepSF - h_el_SF->GetBinError( binX, binY );
	int bin = h_el_FS->FindBin(  std::min(lepSF_FS_pt_cutoff, (float)lep1_p4().Pt()), fabs(lep1_p4().Eta()) );
	lepSF_FS = h_el_FS->GetBinContent(bin);
	lepSF_FS_Up = lepSF_FS + h_el_FS->GetBinError(bin);
	lepSF_FS_Dn = lepSF_FS + h_el_FS->GetBinError(bin);
      }
      if(lep1_is_mu()){
	int binX = h_mu_SF->GetXaxis()->FindBin( std::min(lepSF_pt_cutoff, (float)lep1_p4().Pt()) );
	int binY = h_mu_SF->GetYaxis()->FindBin( fabs(lep1_p4().Eta()) );
	lepSF    = h_mu_SF->GetBinContent( binX, binY );
	lepSF_Up = lepSF + h_mu_SF->GetBinError( binX, binY );
	lepSF_Dn = lepSF - h_mu_SF->GetBinError( binX, binY );
	int bin = h_mu_FS->FindBin(  std::min(lepSF_FS_pt_cutoff, (float)lep1_p4().Pt()), fabs(lep1_p4().Eta()) );
	lepSF_FS = h_mu_FS->GetBinContent(bin);
	lepSF_FS_Up = lepSF_FS + h_mu_FS->GetBinError(bin);
	lepSF_FS_Dn = lepSF_FS + h_mu_FS->GetBinError(bin);
      }
      weight *= (lepSF*lepSF_FS);

      
      
      if(nvtxs()<0)               continue;
      if(ngoodleps()<1)           continue;//accomodate 2l-CR
      if(nvetoleps()<1)           continue;//accomodate 2l-CR
    //if(!PassTrackVeto_v3())     continue;//accomodate 2l-CR
    //if(!PassTauVeto())          continue;//accomodate 2l-CR
      if(ngoodjets()<2)           continue;
      if(ngoodbtags()<0)          continue;//accomodate 1l-CR
      if(pfmet()<250)             continue;
      if(mt_met_lep()<150)        continue;
      if(mindphi_met_j1_j2()<0.8) continue;


      int SR = -1;
      int compressedSR = -1;
      if(ngoodleps()==1&&nvetoleps()==1&&PassTrackVeto_v3()&&PassTauVeto()&&ngoodbtags()>=1) { //basis for SR 1l, >=1b
	if(ngoodjets()>=4){
	  if(MT2W()<=200){
	    if(pfmet()>325) SR = 6;
	    else SR = 5;
	  } else { //high MT2W
	    if(pfmet()>450) SR = 9;
	    else if(pfmet()>350) SR = 8;
	    else SR = 7;
	  }
	} else if(ngoodjets()==3 && MT2W()>200) {
	  if(pfmet()>350) SR = 4;
	  else SR = 3;
	} else if(ngoodjets()==2 && topnessMod()>6.4) {
	  if(pfmet()>350) SR = 2;
	  else SR = 1;
	}
	//compressed region (jets are sorted by pt
	//if(ngoodjets()>=5&&ak4pfjets_passMEDbtag()[0]==false&&ak4pfjets_pt()[0]>200.){
	//  if(MT2W()<=200) compressedSR = 11;
	//  else compressedSR = 12;
	//}
      }

      //CR-1l
      int CR1l = -1;
      if(ngoodleps()==1&&nvetoleps()==1&&PassTrackVeto_v3()&&PassTauVeto()&&ngoodbtags()==0&&ngoodjets()==2&&topnessMod()>6.4){
	CR1l = 1;
      } else if(ngoodleps()==1&&nvetoleps()==1&&PassTrackVeto_v3()&&PassTauVeto()&&ngoodbtags()==0&&ngoodjets()>=3&&MT2W()>200){
	if(ngoodjets()==3){
	  CR1l = 2;
	} else {
	  CR1l = 3;
	}
      }
      //CR1l  1 --> SR  6
      //CR1l  2 --> SR  3-5
      float CR1l_1_1 = 0.743*0.072;
      float CR1l_1_2 = 0.258*0.078;
      float CR1l_2_3 = 0.69*0.0901;
      float CR1l_2_4 = 0.312*0.191;
      float CR1l_3_7 = 0.58*0.13;
      float CR1l_3_8 = 0.241*0.186;
      float CR1l_3_9 = 0.177*0.253;
      /*
      float CR1l_1_1 = 0.76*0.07;
      float CR1l_1_2 = 0.26*0.08;
      float CR1l_2_3 = 0.68*0.09;
      float CR1l_2_4 = 0.32*0.19;
      float CR1l_3_7 = 0.57*0.14;
      float CR1l_3_8 = 0.24*0.19;
      float CR1l_3_9 = 0.18*0.27;
      */

      //CR2l = -1;
      int lepind = -1;
      if(ngoodleps()>2) lepind = 5;
      else if(ngoodleps()==2) lepind = 4;//exactly two leptons,CR4
      else if(ngoodleps()==1&&nvetoleps()>=2&&lep2_pt()>=10) lepind = 3;//one lepton, but more than 1 add. loose,1l,>2l
      else if(ngoodleps()==1&&nvetoleps()==1&&(!PassTrackVeto_v3()||!PassTauVeto())) lepind = 1;//exactly one lepton, but do not pass track/tau veto - i.e. one additional track or tau, CR6
      int CR2l = -1;
      if((lepind==4||lepind==3||lepind==1)&&ngoodjets()==2&&ngoodbtags()>=1&&topnessMod()>6.4){
	CR2l = 1;
      }
      if((lepind==4||lepind==3||lepind==1)&&ngoodjets()>=3&&ngoodbtags()>=1){
	if(MT2W()<=200) CR2l = 2;
	else CR2l = 3;
      }
      float CR2l_1_1 = 0.49*0.88;
      float CR2l_1_2 = 0.49*0.18;
      float CR2l_2_5 = 0.70*0.49;
      float CR2l_2_6 = 0.70*0.19;
      float CR2l_3_3 = 0.50*0.33;
      float CR2l_3_4 = 0.50*0.10;
      float CR2l_3_7 = 0.50*0.38;
      float CR2l_3_8 = 0.50*0.12;
      float CR2l_3_9 = 0.50*0.08;

      if(SR==(-1)&&CR1l==(-1)&&CR2l==(-1)&&compressedSR==(-1)) continue;
      // T2tb_BRselection = -1;
      if(T2tb_BRselection>0){
	int tLSP = 0;
	int bCharg = 0;
	for(unsigned int i = 0; i<gensusy_id().size(); ++i){
	  if(abs(gensusy_id()[i])!=1000022) continue;
	  if(gensusy_status()[i]!=1) continue;
	  if(abs(gensusy_motherid()[i])==1000024) ++bCharg;
	  if(abs(gensusy_motherid()[i])==1000006) ++tLSP;
	}
	if((tLSP+bCharg)!=2) cout << "This should not happen, have " << tLSP << " stop decays to tLSP, and " << bCharg << " stop decays to bChargino" << endl;
	if(T2tb_BRselection==1){
	  if(tLSP!=2) continue;
	}
	if(T2tb_BRselection==2){
	  if(tLSP!=1) continue;
	  if(bCharg!=1) continue;
	}
	if(T2tb_BRselection==3){
	  if(bCharg!=2) continue;
	}
      }
      //++countercheck;

      double weight_pol_L = 1.0;
      double weight_pol_R = 1.0;
      if(mStop-mLSP>=170){
	LorentzVector tl1, tl2;
	//LorentzVector tW1, tW2;
	LorentzVector tt1, tt2;
	LorentzVector tst1, tst2;
	for(unsigned int i = 0; i<gensusy_id().size(); ++i){
	  if(gensusy_status()[i]!=1) continue;
	  //cout << gensusy_status()[i] << " " << gensusy_id()[i] << " motherid/status " << gensusy_motherstatus()[i] << " " << gensusy_motherid()[i] << " grandmotherid/status " << gensusy_gmotherstatus()[i] << " " << gensusy_gmotherid()[i] << endl;
	  //cout << gensusy_p4()[i].Pt() << " " <<  gensusy_id()[i] << " "  << gensusy_motherp4()[i].Pt() << " " <<  gensusy_motherid()[i] << endl;
	  if((gensusy_id()[i])==1000022&&(gensusy_motherid()[i])==1000006) { /*cout << "am here " << endl;*/ tst1 = gensusy_motherp4()[i]; }//mother of LSP - safer
	  if((gensusy_id()[i])==1000022&&(gensusy_motherid()[i])==-1000006) { /*cout << "am there " << endl;*/  tst2 = gensusy_motherp4()[i]; }
	  if(countercheck%10000==0) cout << "this is my stop " << gensusy_motherid()[i] << " " << gensusy_motherp4()[i].Pt() << endl;
	}
	for(unsigned int i = 0; i<genleps_id().size(); ++i){
	  if(genleps_status()[i] != 23 && genleps_status()[i] != 22 && genleps_status()[i] != 1) continue;
	  if(abs(genleps_motherid()[i])!=24) continue;
	  if((genleps_gmotherid()[i])==6&&genleps_gmotherid()[i]*genleps_id()[i]<0) { tt1 = genleps_gmotherp4()[i]; tl1 = genleps_p4()[i]; }
	  if((genleps_gmotherid()[i])==-6&&genleps_gmotherid()[i]*genleps_id()[i]<0) { tt2 = genleps_gmotherp4()[i]; tl2 = genleps_p4()[i]; }
	  if(countercheck%10000==0&&abs(genleps_gmotherid()[i])==6&&genleps_gmotherid()[i]*genleps_id()[i]<0) cout << "this is my top " << genleps_gmotherid()[i] << " " <<  genleps_gmotherp4()[i].Pt() << " and lepton " << genleps_id()[i] << " " << genleps_p4()[i].Pt() << endl;
	}
	for(unsigned int i = 0; i<gennus_id().size(); ++i){
	  if(gennus_status()[i] != 23 && gennus_status()[i] != 22 && gennus_status()[i] != 1) continue;
	  if(abs(gennus_motherid()[i])!=24) continue;
	  if((gennus_gmotherid()[i])==6&&gennus_gmotherid()[i]*gennus_id()[i]<0) { tt1 = gennus_gmotherp4()[i]; tl1 = gennus_p4()[i]; }
	  if((gennus_gmotherid()[i])==-6&&gennus_gmotherid()[i]*gennus_id()[i]<0) { tt2 = gennus_gmotherp4()[i]; tl2 = gennus_p4()[i]; }
	  if(countercheck%10000==0&&abs(gennus_gmotherid()[i])==6&&gennus_gmotherid()[i]*gennus_id()[i]<0) cout << "this is my top " << gennus_gmotherid()[i] << " " <<  gennus_gmotherp4()[i].Pt() << " and neutrino " << gennus_id()[i] << " " << gennus_p4()[i].Pt() << endl;

	}
      	for(unsigned int i = 0; i<genqs_id().size(); ++i){
	  if(genqs_status()[i] != 23 && genqs_status()[i] != 22 && genqs_status()[i] != 1) continue;
	  if(abs(genqs_motherid()[i])!=24) continue;
	  if((genqs_gmotherid()[i])==6&&genqs_gmotherid()[i]*genqs_id()[i]<0) { tt1 = genqs_gmotherp4()[i]; tl1 = genqs_p4()[i]; }
	  if((genqs_gmotherid()[i])==-6&&genqs_gmotherid()[i]*genqs_id()[i]<0) { tt2 = genqs_gmotherp4()[i]; tl2 = genqs_p4()[i]; }
	  if(countercheck%10000==0&&abs(genqs_gmotherid()[i])==6&&genqs_gmotherid()[i]*genqs_id()[i]<0) cout << "this is my top " << genqs_gmotherid()[i] << " " <<  genqs_gmotherp4()[i].Pt() << " and quark " << genqs_id()[i] << " " << genqs_p4()[i].Pt() << endl;

	}

	TLorentzVector st1; st1.SetPxPyPzE(tst1.Px(),tst1.Py(),tst1.Pz(),tst1.E());
	TLorentzVector st2; st2.SetPxPyPzE(tst2.Px(),tst2.Py(),tst2.Pz(),tst2.E());
	TVector3 bV1(-st1.Px()/st1.Energy(),-st1.Py()/st1.Energy(),-st1.Pz()/st1.Energy());
	TVector3 bV2(-st2.Px()/st2.Energy(),-st2.Py()/st2.Energy(),-st2.Pz()/st2.Energy());
	TLorentzVector t1; t1.SetPxPyPzE(tt1.Px(),tt1.Py(),tt1.Pz(),tt1.E());
	TLorentzVector t2; t2.SetPxPyPzE(tt2.Px(),tt2.Py(),tt2.Pz(),tt2.E());
	TLorentzVector l1; l1.SetPxPyPzE(tl1.Px(),tl1.Py(),tl1.Pz(),tl1.E());
	TLorentzVector l2; l2.SetPxPyPzE(tl2.Px(),tl2.Py(),tl2.Pz(),tl2.E());
	t1.Boost(bV1);
	t2.Boost(bV2);
	l1.Boost(bV1);
	l2.Boost(bV2);
	if(countercheck%10000==0){
	  cout << "Boost Pt " <<  bV1.Pt() << " " << bV2.Pt() << endl;
	  cout << "Check " << t1.P() << " " << l1.P() << " "  << t2.P() << " " << l2.P() << endl;
	  cout << " stops " << st1.Px() << " " << st1.Py() << " " << st1.Pz() << " " << st1.E() << " " <<  st2.Px() << " " << st2.Py() << " " << st2.Pz() << " " << st2.E() << endl;
	  cout << "Tops before boost " << tt1.Px() << " " << tt1.Py() << " " << tt1.Pz() << " " << tt1.E() << " " <<  tt2.Px() << " " << tt2.Py() << " " << tt2.Pz() << " " << tt2.E() << endl;
	  cout << "Tops after boost " << t1.Px() << " " << t1.Py() << " " << t1.Pz() << " " << t1.E() << " " <<  t2.Px() << " " << t2.Py() << " " << t2.Pz() << " " << t2.E() << endl;
	}
	//cout << t1.P() << " " << l1.P()  << t2.P() << " " << l2.P() << endl;
	if(t1.P()>0&&l1.P()>0){
	  double costh = (t1.Px()*l1.Px()+t1.Py()*l1.Py()+t1.Pz()*l1.Pz())/t1.P()/l1.P();
	  double weight_L = (t1.Energy()+t1.P())*(1-costh);
	  double weight_R = (t1.Energy()-t1.P())*(1+costh);
	  if(countercheck%10000==0) cout << "1 costh wL wR " << costh << " " << weight_L << " " << weight_R << endl;
	  weight_pol_L *= 2*weight_L/(weight_R+weight_L);
	  weight_pol_R *= 2*weight_R/(weight_R+weight_L);
	}
	if(t2.P()>0&&l2.P()>0){
	  double costh = (t2.Px()*l2.Px()+t2.Py()*l2.Py()+t2.Pz()*l2.Pz())/t2.P()/l2.P();
	  double weight_L = (t2.Energy()+t2.P())*(1-costh);
	  double weight_R = (t2.Energy()-t2.P())*(1+costh);
	  if(countercheck%10000==0) cout << "2 costh wL wR  " << costh << " " << weight_L << " " << weight_R << endl;
	  weight_pol_L *= 2*weight_L/(weight_R+weight_L);
	  weight_pol_R *= 2*weight_R/(weight_R+weight_L);
	}
      }
      if(LR==-1) weight *= weight_pol_L;
      if(LR==+1) weight *= weight_pol_R;
      if(countercheck%10000==0){
	cout << "This is it " << weight_pol_L << " " << weight_pol_R << endl;
      }
      
      //implement some sanity checks
      if(CR1l!=(-1)&&CR2l!=(-1)) cout << "WTF CR1l " << CR1l << " CR2l " << CR2l << endl;
      if(SR!=(-1)&&CR1l!=(-1)) cout << "WTF SR " << SR << " CR1l " << CR1l << endl;
      if(SR!=(-1)&&CR2l!=(-1)) cout << "WTF SR " << SR << " CR2l " << CR2l << endl;

      //ISR reweighting, get stop pair using last copy:
      double ISRup = weight_ISRup()/weight_ISR()*ISRnorm/ISRnormup;
      double ISRdown = weight_ISRdown()/weight_ISR()*ISRnorm/ISRnormdown;
      //double XSup = (xsec()+xsec_uncert())/xsec();
      //double XSdown = (xsec()-xsec_uncert())/xsec();
      double XSup = (xsection+xsectionerr)/xsection;
      double XSdown = (xsection-xsectionerr)/xsection;
      double PUup = PUweightUp/PUweight;
      double PUdown = PUweightDown/PUweight;
      double lEffup = lepSF_Up/lepSF;
      double lEffdown = lepSF_Dn/lepSF;
      double lEffFSup = lepSF_FS_Up/lepSF_FS;
      double lEffFSdown = lepSF_FS_Dn/lepSF_FS;
      double BSFHup = weight_btagsf_heavy_UP()/weight_btagsf()*BSFnorm/BSFnormHup;
      double BSFLup = weight_btagsf_light_UP()/weight_btagsf()*BSFnorm/BSFnormHup;
      double BSFHdown = weight_btagsf_heavy_DN()/weight_btagsf()*BSFnorm/BSFnormHup;
      double BSFLdown = weight_btagsf_light_DN()/weight_btagsf()*BSFnorm/BSFnormHup;
      double muRFup = genweights().at(4)/genweights().at(0)*muRFnorm/muRFnormup;
      double muRFdown = genweights().at(8)/genweights().at(0)*muRFnorm/muRFnormdown;

      if(CR1l>0){
	if(ngoodleps()!=1) cout << __LINE__ << " " << ngoodleps() << endl;
	if(NSLeps!=1) cout << __LINE__ << " " << NSLeps << endl;
	if(nvetoleps()!=1) cout << __LINE__ << " " << nvetoleps() << endl;
	if(ngoodbtags()>=1) cout << __LINE__ << " " << ngoodbtags() << endl;
	//signal contamination in 0b control region, do correlations later during datacard making
	if(CR1l==1){
	  histos["CR1l_sigcontamination"]->Fill(mStop,mLSP,1,weight*CR1l_1_1);
	  histos["CR1l_sigcontamination"]->Fill(mStop,mLSP,2,weight*CR1l_1_2);
	} else if(CR1l==2){
	  histos["CR1l_sigcontamination"]->Fill(mStop,mLSP,3,weight*CR1l_2_3);
	  histos["CR1l_sigcontamination"]->Fill(mStop,mLSP,4,weight*CR1l_2_4);
	} else if(CR1l==3){
	  histos["CR1l_sigcontamination"]->Fill(mStop,mLSP,7,weight*CR1l_3_7);
	  histos["CR1l_sigcontamination"]->Fill(mStop,mLSP,8,weight*CR1l_3_8);
	  histos["CR1l_sigcontamination"]->Fill(mStop,mLSP,9,weight*CR1l_3_9);
	}
      } else if(CR2l>0){
	//if(nvetoleps()<=1||(nvetoleps()==1&&(!PassTrackVeto_v3()||!PassTauVeto()))) cout << __LINE__ << " " << nvetoleps() << " " << PassTrackVeto_v3() << " " << PassTauVeto() << endl;
	if(ngoodbtags()<1) cout << __LINE__ << " " << ngoodbtags() << endl;
	//signal contamination in 2l control region, do correlations later during datacard making
	if(CR2l==1){
	  histos["CR2l_sigcontamination"]->Fill(mStop,mLSP,1,weight*CR2l_1_1);
	  histos["CR2l_sigcontamination"]->Fill(mStop,mLSP,2,weight*CR2l_1_2);
	} else if(CR2l==2){
	  histos["CR2l_sigcontamination"]->Fill(mStop,mLSP,5,weight*CR2l_2_5);
	  histos["CR2l_sigcontamination"]->Fill(mStop,mLSP,6,weight*CR2l_2_6);
	} else if(CR2l==3){
	  histos["CR2l_sigcontamination"]->Fill(mStop,mLSP,7,weight*CR2l_3_7);
	  histos["CR2l_sigcontamination"]->Fill(mStop,mLSP,8,weight*CR2l_3_8);
	  histos["CR2l_sigcontamination"]->Fill(mStop,mLSP,9,weight*CR2l_3_9);
	  histos["CR2l_sigcontamination"]->Fill(mStop,mLSP,3,weight*CR2l_3_3);
	  histos["CR2l_sigcontamination"]->Fill(mStop,mLSP,4,weight*CR2l_3_4);

	}
      } else if(SR>0){
	if(ngoodleps()!=1) cout << __LINE__ << " " << ngoodleps() << endl;
	if(NSLeps!=1) cout << __LINE__ << " " << NSLeps << endl;
	if(nvetoleps()!=1) cout << __LINE__ << " " << nvetoleps() << endl;
	if(!PassTrackVeto_v3())  cout << __LINE__ << endl;
	if(!PassTauVeto())  cout << __LINE__ << endl;
	//if(SR<=6&&ngoodjets()<3) cout << __LINE__ << " " << ngoodjets() << endl;
	if(ngoodbtags()<1) cout << __LINE__ << " " << ngoodbtags() << endl;
	
	histos2["eventsum"]->Fill(mStop,mLSP,SR,1.);
	histos2["rawweightsum"]->Fill(mStop,mLSP,SR,rawweight);
	histos2["totweightsum"]->Fill(mStop,mLSP,SR,weight);
	histos2["ISRsum"]->Fill(mStop,mLSP,SR,ISRweight);
	histos2["BSFsum"]->Fill(mStop,mLSP,SR,BSFweight);
	histos2["PUweightsum"]->Fill(mStop,mLSP,SR,PUweight);
	histos2["xsecsum"]->Fill(mStop,mLSP,SR,xsection);
	histos2["nevtsum"]->Fill(mStop,mLSP,SR,nevts);
	histos2["lepsum"]->Fill(mStop,mLSP,SR,lepSF);
	histos2["lepSFsum"]->Fill(mStop,mLSP,SR,lepSF_FS);
	histos2["WLsum"]->Fill(mStop,mLSP,SR,weight_pol_L);
	histos2["WRsum"]->Fill(mStop,mLSP,SR,weight_pol_R);
	
	//finally - do signal regions!
	histos["SRyield"]->Fill(mStop,mLSP,SR,weight);
	histos["SR_ISRup"]->Fill(mStop,mLSP,SR,weight*ISRup);
	histos["SR_ISRdown"]->Fill(mStop,mLSP,SR,weight*ISRdown);
	histos["SR_Xsecup"]->Fill(mStop,mLSP,SR,weight*XSup);
	histos["SR_Xsecdown"]->Fill(mStop,mLSP,SR,weight*XSdown);
	histos["SR_PUup"]->Fill(mStop,mLSP,SR,weight*PUup);
	histos["SR_PUdown"]->Fill(mStop,mLSP,SR,weight*PUdown);
	histos["SR_Bup_HF"]->Fill(mStop,mLSP,SR,weight*BSFHup);
	histos["SR_Bup_LF"]->Fill(mStop,mLSP,SR,weight*BSFLup);
	histos["SR_Bdown_HF"]->Fill(mStop,mLSP,SR,weight*BSFHdown);
	histos["SR_Bdown_LF"]->Fill(mStop,mLSP,SR,weight*BSFLdown);
	histos["SR_LepEffup"]->Fill(mStop,mLSP,SR,weight*lEffup);
	histos["SR_LepEffdown"]->Fill(mStop,mLSP,SR,weight*lEffdown);
	histos["SR_LepEffFSup"]->Fill(mStop,mLSP,SR,weight*lEffFSup);
	histos["SR_LepEffFSdown"]->Fill(mStop,mLSP,SR,weight*lEffFSdown);
	histos["SR_muRFup"]->Fill(mStop,mLSP,SR,weight*muRFup);
	histos["SR_muRFdown"]->Fill(mStop,mLSP,SR,weight*muRFdown);
      }
      if(compressedSR>0){
	if(compressedSR<=6) cout << __LINE__ << " " << compressedSR << endl;
	//compressedSR is defined to not overlap with SR - can use same histogram!
	histos["SRyield"]->Fill(mStop,mLSP,compressedSR,weight);
	histos["SR_ISRup"]->Fill(mStop,mLSP,compressedSR,weight*ISRup);
	histos["SR_ISRdown"]->Fill(mStop,mLSP,compressedSR,weight*ISRdown);
	histos["SR_Xsecup"]->Fill(mStop,mLSP,compressedSR,weight*XSup);
	histos["SR_Xsecdown"]->Fill(mStop,mLSP,compressedSR,weight*XSdown);
	histos["SR_PUup"]->Fill(mStop,mLSP,compressedSR,weight*PUup);
	histos["SR_PUdown"]->Fill(mStop,mLSP,compressedSR,weight*PUdown);
	histos["SR_Bup_HF"]->Fill(mStop,mLSP,compressedSR,weight*BSFHup);
	histos["SR_Bup_LF"]->Fill(mStop,mLSP,compressedSR,weight*BSFLup);
	histos["SR_Bdown_HF"]->Fill(mStop,mLSP,compressedSR,weight*BSFHdown);
	histos["SR_Bdown_LF"]->Fill(mStop,mLSP,compressedSR,weight*BSFLdown);
	histos["SR_LepEffup"]->Fill(mStop,mLSP,compressedSR,weight*lEffup);
	histos["SR_LepEffdown"]->Fill(mStop,mLSP,compressedSR,weight*lEffdown);
	histos["SR_LepEffFSup"]->Fill(mStop,mLSP,compressedSR,weight*lEffFSup);
	histos["SR_LepEffFSdown"]->Fill(mStop,mLSP,compressedSR,weight*lEffFSdown);
	histos["SR_muRFup"]->Fill(mStop,mLSP,compressedSR,weight*muRFup);
	histos["SR_muRFdown"]->Fill(mStop,mLSP,compressedSR,weight*muRFdown);
      }

    }//event loop
  
    // Clean Up
    delete tree;
    file->Close();
    delete file;
  }//file loop
  if ( nEventsChain != nEventsTotal ) {
    cout << Form( "ERROR: number of events from files (%d) is not equal to total number of events (%d)", nEventsChain, nEventsTotal ) << endl;
  }
  
  // Example Histograms
  // samplehisto->Draw();

  /*
  for(map<string,TH3D*>::iterator h=histos.begin(); h!=histos.end();++h){
    //add overflow
    //h->second->SetBinContent(h->second->GetNbinsX(), h->second->GetBinContent(h->second->GetNbinsX() )+ h->second->GetBinContent(h->second->GetNbinsX()+1) );
    //h->second->SetBinError(h->second->GetNbinsX(), sqrt(pow(h->second->GetBinError(h->second->GetNbinsX() ),2)+pow(h->second->GetBinError(h->second->GetNbinsX()+1),2) ) );
    //add underfloe
    //h->second->SetBinContent(1, h->second->GetBinContent(1)+ h->second->GetBinContent(0) );
    //h->second->SetBinError(1, sqrt(pow(h->second->GetBinError(1),2)+pow(h->second->GetBinError(0),2) ) );
  }
  */
  string filename = "rootfiles/signalyields/Histos_"+skimFilePrefix+".root";
  if(T2tb_BRselection==1) filename = "rootfiles/signalyields/Histos_"+skimFilePrefix+"_tLSP.root";
  if(T2tb_BRselection==2) filename = "rootfiles/signalyields/Histos_"+skimFilePrefix+"_mixed.root";
  if(T2tb_BRselection==3) filename = "rootfiles/signalyields/Histos_"+skimFilePrefix+"_bCharg.root";
  if(LR==-1) filename = "rootfiles/signalyields/Histos_"+skimFilePrefix+"_lefthanded.root";
  if(LR==+1) filename = "rootfiles/signalyields/Histos_"+skimFilePrefix+"_righthanded.root";
  TFile *f = new TFile(filename.c_str(),"RECREATE");
  f->cd();
  for(map<string,TH3D*>::iterator h=    histos.begin(); h!=    histos.end();++h) h->second->Write();
  f->Close();
  cout << "Saved histos in " << f->GetName() << endl;
  string filename2 = "rootfiles/signalyields/CheckHistos_"+skimFilePrefix+".root";
  if(T2tb_BRselection==1) filename2 = "rootfiles/signalyields/CheckHistos_"+skimFilePrefix+"_tLSP.root";
  if(T2tb_BRselection==2) filename2 = "rootfiles/signalyields/CheckHistos_"+skimFilePrefix+"_mixed.root";
  if(T2tb_BRselection==3) filename2 = "rootfiles/signalyields/CheckHistos_"+skimFilePrefix+"_bCharg.root";
  if(LR==-1) filename2 = "rootfiles/signalyields/CheckHistos_"+skimFilePrefix+"_lefthanded.root";
  if(LR==+1) filename2 = "rootfiles/signalyields/CheckHistos_"+skimFilePrefix+"_righthanded.root";
  TFile *f2 = new TFile(filename2.c_str(),"RECREATE");
  f2->cd();
  for(map<string,TH3D*>::iterator h=    histos2.begin(); h!=    histos2.end();++h) h->second->Write();
  f2->Close();
  cout << "Saved histos in " << f2->GetName() << endl;
  
  f_el_SF->Close();
  f_mu_SF_id->Close();
  f_mu_SF_iso->Close();
  f_mu_SF_veto_id->Close();
  f_mu_SF_veto_iso->Close();
  f_vetoLep_eff->Close();
  f_el_FS_ID->Close();
  f_el_FS_Iso->Close();
  f_mu_FS_ID->Close();
  f_mu_FS_Iso->Close();
  // return
  bmark->Stop("benchmark");
  cout << endl;
  cout << nEventsTotal << " Events Processed" << endl;
  cout << "------------------------------" << endl;
  cout << "CPU  Time:	" << Form( "%.01f", bmark->GetCpuTime("benchmark")  ) << endl;
  cout << "Real Time:	" << Form( "%.01f", bmark->GetRealTime("benchmark") ) << endl;
  cout << endl;
  delete bmark;
  delete fPU;//delete PU file
  delete f_el_SF;
  delete f_mu_SF_id;
  delete f_mu_SF_iso;
  delete f_mu_SF_veto_id;
  delete f_mu_SF_veto_iso;
  delete f_vetoLep_eff;
  delete f_el_FS_ID;
  delete f_el_FS_Iso;
  delete f_mu_FS_ID;
  delete f_mu_FS_Iso;
  return 0;
}
