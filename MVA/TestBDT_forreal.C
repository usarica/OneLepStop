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
#include "TSystem.h"
#include "TInterpreter.h"
#include "TTree.h"
#include "TTreeCache.h"
#include "TLorentzVector.h"

#if not defined(__CINT__) || defined(__MAKECINT__)
// needs to be included when makecint runs (ACLIC)
#include "TMVA/Factory.h"
#include "TMVA/Tools.h"
#endif


using namespace std;

int TestBDT_forreal(TString sig) {

   // This loads the library
   TMVA::Tools::Instance();
   // to get access to the GUI and all tmva macros
   //TString thisdir = gSystem->DirName(gInterpreter->GetCurrentMacroName());//was not commented, but does not work anymore
   //gROOT->SetMacroPath(thisdir + ":" + gROOT->GetMacroPath());//was not commented, but cannot not work anymore
   //gROOT->ProcessLine(".L TMVAGui.C");
   TString outfileName( "rootfiles/testTMVA_"+sig+"VsTTbar2l.root" );
   TFile* outputFile = TFile::Open( outfileName, "RECREATE" );
   TMVA::Factory *factory = new TMVA::Factory( "myBDTtest", outputFile,"!V:!Silent:Color:DrawProgressBar");

  vector<TString> histonames; histonames.clear();
  map<string, float> value;
  
  histonames.push_back("MT2W");             
  //histonames.push_back("MT2_lb_b");         
  //histonames.push_back("MT2_lb_bqq");       
  //histonames.push_back("MT2_lb_b_mass");   
  //histonames.push_back("MT2_lb_bqq_mass"); 
  histonames.push_back("Mlb_lead_bdiscr");              
  //histonames.push_back("Mjjj");              
  //histonames.push_back("topness");          
  //histonames.push_back("topnessMod");          
  histonames.push_back("pfmet");              
  //histonames.push_back("ak4_HT");               
  //histonames.push_back("MET_over_sqrtHT");    
  //histonames.push_back("ak4_htratiom");          
  //histonames.push_back("dR_lep_leadb");        
  //histonames.push_back("hadronic_top_chi2");             
  //histonames.push_back("ngoodbtags");           
  histonames.push_back("ngoodjets");            
  histonames.push_back("mindphi_met_j1_j2");        
  histonames.push_back("lep1_pt");
  histonames.push_back("ak4pfjets_leadMEDbjet_p4.Pt()"); 

  for(unsigned int b = 0; b<histonames.size(); ++b){
    factory->AddVariable(histonames[b], 'F' );
  }

  TString signame;
  if(sig.Contains("T2bw")||sig.Contains("T2tb")) signame = "/nfs-7/userdata/stopRun2/CMS3_V07-04-XX/v7.4.x_v5/priv_"+sig+".root";
  else signame = "/home/users/jgwood/Stop_1Lepton/phys14Samples_cmssw74x_babyMaker/CMSSW_7_2_0/src/StopAnalysis/StopBabyMaker/"+sig+".root";
  TString bkgname1 = "/nfs-7/userdata/stopRun2/CMS3_V07-04-XX/v7.4.x_v5/TTJets_DiLept_madgraph_25ns_1.root";
  TString bkgname2 = "/nfs-7/userdata/stopRun2/CMS3_V07-04-XX/v7.4.x_v5/TTJets_DiLept_madgraph_25ns_2.root";
  cout << "signame " << signame << endl;
  TFile *inputSig = TFile::Open( signame );
  TFile *inputBkg1 = TFile::Open( bkgname1 );
  TFile *inputBkg2 = TFile::Open( bkgname2 );
  TTree *signal     = (TTree*)inputSig->Get("t");
  TTree *background1 = (TTree*)inputBkg1->Get("t");
  TTree *background2 = (TTree*)inputBkg2->Get("t");

   // global event weights per tree (see below for setting event-wise weights)
   Double_t signalWeight     = 1.0;
   Double_t backgroundWeight = 1.0;
   
   // You can add an arbitrary number of signal or background trees
   factory->AddSignalTree    ( signal,     signalWeight     );
   factory->AddBackgroundTree( background1, backgroundWeight* 5977821./(5977821.+24521141.) );
   factory->AddBackgroundTree( background2, backgroundWeight*24521141./(5977821.+24521141.) );

   //factory->SetBackgroundWeightExpression( "weight" );
   // Apply additional cuts on the signal and background samples (can be different)
   TCut mycuts = "nvtxs>=1&&nvetoleps==1&&ngoodleps==1&&PassTrackVeto_v3&&PassTauVeto&&mt_met_lep>150&&pfmet>250&&ngoodjets>=2&&ngoodbtags>=1"; // for example: TCut mycuts = "abs(var1)<0.5 && abs(var2-0.5)<1";
   TCut mycutb = "nvtxs>=1&&nvetoleps==1&&ngoodleps==1&&PassTrackVeto_v3&&PassTauVeto&&mt_met_lep>150&&pfmet>250&&ngoodjets>=2&&ngoodbtags>=1"; // for example: TCut mycutb = "abs(var1)<0.5";
   factory->PrepareTrainingAndTestTree( mycuts, mycutb,
                                        "nTrain_Signal=0:nTrain_Background=0:SplitMode=Random:NormMode=NumEvents:!V" );
   factory->BookMethod( TMVA::Types::kBDT, "BDT",
			"!H:!V:NTrees=850:MinNodeSize=2.5%:MaxDepth=3:BoostType=AdaBoost:AdaBoostBeta=0.5:UseBaggedBoost:BaggedSampleFraction=0.5:SeparationType=GiniIndex:nCuts=20" );
   // Train MVAs using the set of training events
   cout << "Train methods" << endl;
   factory->TrainAllMethods();

   // ---- Evaluate all MVAs using the set of test events
   cout << "Test methods" << endl;
   factory->TestAllMethods();

   // ----- Evaluate and compare performance of all configured MVAs
   cout << "Evaluate methods" << endl;
   factory->EvaluateAllMethods();

   // --------------------------------------------------------------

   // Save the output
   outputFile->Close();

   std::cout << "==> Wrote root file: " << outputFile->GetName() << std::endl;
   std::cout << "==> TMVAClassification is done!" << std::endl;

   delete factory;

   // Launch the GUI for the root macros
   //if (!gROOT->IsBatch()) TMVA::TMVAGui( outfileName );

  return 0;
}
