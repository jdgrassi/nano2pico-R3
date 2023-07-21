//----------------------------------------------------------------------------
// utilities - Various functions used accross the code
//----------------------------------------------------------------------------

#include "utilities.hpp"

#include <cmath>

#include <deque>
#include <iostream>
#include <fstream>
#include <string>
#include <stdexcept>
#include <iomanip>   // setw

#include <libgen.h>

#include "TCollection.h"
#include "TFile.h"
#include "TGraph.h"
#include "TH1D.h"
#include "TList.h"
#include "TString.h"
#include "TSystemDirectory.h"
#include "TSystemFile.h"
#include "TSystem.h"
#include "TTree.h"
#include "TChain.h"
#include "TRegexp.h"
#include "TLorentzVector.h"

#include "nano_tree.hpp"

using namespace std;

long double DeltaPhi(long double phi1, long double phi2){
  long double dphi = fmod(fabs(phi2-phi1), 2.L*PI);
  return dphi>PI ? 2.L*PI-dphi : dphi;
}

long double SignedDeltaPhi(long double phi1, long double phi2){
  long double dphi = fmod(phi2-phi1, 2.L*PI);
  if(dphi>PI){
    return dphi-2.L*PI;
  }else if(dphi<-PI){
    return dphi+2.L*PI;
  }else{
    return dphi;
  }
}

float dR(float eta1, float eta2, float phi1, float phi2) {
  return AddInQuadrature(eta1-eta2, DeltaPhi(phi1,phi2));
}

double cosThetaJeff(TLorentzVector lminus, TLorentzVector lplus, TLorentzVector photon) {
  // Calculates the angle between the Zs spin and the lepton in the Zs rest frame
  TLorentzVector ll = lminus + lplus;
  lminus.Boost(-ll.BoostVector());
  photon.Boost(-ll.BoostVector());
  TVector3 l(lminus.Vect()), p(photon.Vect());
  double costj = l*p/(l.Mag()*p.Mag());
  return costj;
}

TString roundNumber(double num, int decimals, double denom){
  if(denom==0) return " - ";
  double neg = 1; if(num*denom<0) neg = -1;
  num /= neg*denom; num += 0.5*pow(10.,-decimals);
  long num_int = static_cast<long>(num);
  long num_dec = static_cast<long>((1+num-num_int)*pow(10.,decimals));
  TString s_dec = ""; s_dec += num_dec; s_dec.Remove(0,1);
  TString result="";
  if(neg<0) result+="-";
  result+= num_int;
  if(decimals>0) {
    result+="."; result+=s_dec;
  }

  TString afterdot = result;
  afterdot.Remove(0,afterdot.First(".")+1);
  for(int i=0; i<decimals-afterdot.Length(); i++)
    result += "0";
  return result;
}

TString addCommas(double num){
  TString result(""); result += num;
  int posdot(result.First('.'));
  if(posdot==-1) posdot = result.Length();
  for(int ind(posdot-3); ind > 0; ind -= 3)
    result.Insert(ind, ",");
  return result;
}

long double AddInQuadrature(long double x, long double y){
  if(fabs(y)>fabs(x)){
    const long double temp = y;
    y=x;
    x=temp;
  }
  if(x==0.) return y;
  const long double rat=y/x;
  return fabs(x)*sqrt(1.0L+rat*rat);
}

long double GetMass(long double e, long double px, long double py, long double pz){
  px/=e; py/=e; pz/=e;
  return fabs(e)*sqrt(1.0L-px*px-py*py-pz*pz);
}

long double GetMT(long double m1, long double pt1, long double phi1,
                  long double m2, long double pt2, long double phi2){
  return sqrt(m1*m1+m2*m2+2.L*(sqrt((m1*m1+pt1*pt1)*(m2*m2+pt2*pt2))-pt1*pt2*cos(phi2-phi1)));
}

long double GetMT(long double pt1, long double phi1,
                  long double pt2, long double phi2){
  //Faster calculation in massless case
  return sqrt(2.L*pt1*pt2*(1.L-cos(phi2-phi1)));
}

bool Contains(const string& text, const string& pattern){
  return text.find(pattern) != string::npos;
}

vector<string> Tokenize(const string& input,
                        const string& tokens){
  char* ipt(new char[input.size()+1]);
  memcpy(ipt, input.data(), input.size());
  ipt[input.size()]=static_cast<char>(0);
  char* ptr(strtok(ipt, tokens.c_str()));
  vector<string> output(0);
  while(ptr!=NULL){
    output.push_back(ptr);
    ptr=strtok(NULL, tokens.c_str());
  }
  return output;
}

void get_count_and_uncertainty(TTree& tree,
                               const string& cut,
                               double& count,
                               double& uncertainty){
  const string hist_name("temp");
  TH1D temp(hist_name.c_str(), "", 1, -1.0, 1.0);
  tree.Project(hist_name.c_str(), "0.0", cut.c_str());
  count=temp.IntegralAndError(0,2,uncertainty);
}

void AddPoint(TGraph& graph, const double x, const double y){
  graph.SetPoint(graph.GetN(), x, y);
}

string execute(const string &cmd){
  FILE *pipe = popen(cmd.c_str(), "r");
  if(!pipe) throw runtime_error("Could not open pipe.");
  const size_t buffer_size = 128;
  char buffer[buffer_size];
  string result = "";
  while(!feof(pipe)){
    if(fgets(buffer, buffer_size, pipe) != NULL) result += buffer;
  }

  pclose(pipe);
  return result;
}

TString hoursMinSec(long seconds){
  int minutes((seconds/60)%60), hours(seconds/3600);
  TString hhmmss("");
  if(hours<10) hhmmss += "0";
  hhmmss += hours; hhmmss += ":";
  if(minutes<10) hhmmss += "0";
  hhmmss += minutes; hhmmss += ":";
  if((seconds%60)<10) hhmmss += "0";
  hhmmss += seconds%60; 

  return hhmmss;
}

void ReplaceAll(string &str, const string &orig, const string &rep){
  size_t loc = 0;
  while ((loc = str.find(orig, loc)) != string::npos) {
    str.replace(loc, orig.length(), rep);
    loc += rep.length();
  }
}

string CopyReplaceAll(string str, const string &orig, const string &rep){
  ReplaceAll(str, orig, rep);
  return str;
}

void SplitFilePath(const string &path, string &dir_name, string &base_name){
  vector<char> cstr(path.c_str(), path.c_str()+path.size()+1);
  dir_name = dirname(&cstr.at(0));
  cstr = vector<char>(path.c_str(), path.c_str()+path.size()+1);
  base_name = basename(&cstr.at(0));
}

void getMETWithJEC(nano_tree & nano, int year, bool isFastsim, float & MET_pt, float & MET_phi, bool is_preUL) {
  if (isFastsim) { 
    if (year==2017 && is_preUL) {
      MET_pt = nano.METFixEE2017_T1_pt(); 
      MET_phi = nano.METFixEE2017_T1_phi();
    } else {
      MET_pt = nano.MET_T1_pt();
      MET_phi = nano.MET_T1_phi();
    }
  } else {
    if (year == 2017 && is_preUL) {
      MET_pt = nano.METFixEE2017_pt();
      MET_phi = nano.METFixEE2017_phi();
    } else {
      MET_pt = nano.MET_pt();
      MET_phi = nano.MET_phi();
    }
  }
}
void getJetWithJEC(nano_tree & nano, bool isFastsim, vector<float> & Jet_pt, vector<float> & Jet_mass) {
  Jet_pt.resize(nano.nJet());
  Jet_mass.resize(nano.nJet());
  for(int ijet(0); ijet<nano.nJet(); ++ijet){
    if (isFastsim) {
      Jet_pt[ijet] = nano.Jet_pt_nom()[ijet];
      Jet_mass[ijet] = nano.Jet_mass_nom()[ijet];
    } else {
      Jet_pt[ijet] = nano.Jet_pt()[ijet];
      Jet_mass[ijet] = nano.Jet_mass()[ijet];
    }
  }
}

void getJetId(nano_tree & nano, int year, vector<int> & Jet_jetId) {
  Jet_jetId.resize(nano.nJet());
  for(int ijet(0); ijet<nano.nJet(); ++ijet){
    if (year == 2023) Jet_jetId[ijet] = nano.Jet_jetId_11p9()[ijet];
    else Jet_jetId[ijet] = nano.Jet_jetId()[ijet];
  }
}

void getFatJet_btagDDBvL(nano_tree & nano, float nanoaod_version, vector<int> & FatJet_btagDDBvL) {
  FatJet_btagDDBvL.resize(nano.nFatJet());
  for(int ijet(0); ijet<nano.nFatJet(); ++ijet){
    if (nanoaod_version+0.01 < 9) FatJet_btagDDBvL[ijet] = nano.FatJet_btagDDBvL()[ijet];
    else FatJet_btagDDBvL[ijet] = nano.FatJet_btagDDBvLV2()[ijet];
  }
}

void getPhoton_electronIdx(nano_tree & nano, int year, vector<int> & Photon_electronIdx) {
  Photon_electronIdx.resize(nano.nPhoton());
  for(int iphoton(0); iphoton<nano.nPhoton(); ++iphoton){
    if (year == 2023) Photon_electronIdx[iphoton] = nano.Photon_electronIdx_11p9()[iphoton];
    else Photon_electronIdx[iphoton] = nano.Photon_electronIdx()[iphoton];
  }
}

void getMuon_fsrPhotonIdx(nano_tree & nano, int year, vector<int> & Muon_fsrPhotonIdx) {
  Muon_fsrPhotonIdx.resize(nano.nMuon());
  for(int imuon(0); imuon<nano.nMuon(); ++imuon){
    if (year == 2023) Muon_fsrPhotonIdx[imuon] = nano.Muon_fsrPhotonIdx_11p9()[imuon];
    else Muon_fsrPhotonIdx[imuon] = nano.Muon_fsrPhotonIdx()[imuon];
  }
}

void getElectron_photonIdx(nano_tree & nano, int year, vector<int> & Electron_photonIdx) {
  Electron_photonIdx.resize(nano.nElectron());
  //cout<<"nElectron: "<<nano.nElectron()<<endl;
  //cout<<"  "<<nano.Electron_photonIdx()[0]<<endl;
  ////cout<<"  "<<nano.Electron_photonIdx_short()[0]<<endl;
  //cout<<"nPhoton: "<<nano.nPhoton()<<endl;
  //cout<<"  "<<nano.Photon_cutBased()[0]<<endl;
  //cout<<"  "<<nano.Photon_cutBased_char()[0]<<endl;
  for(int iel(0); iel<nano.nElectron(); ++iel){
    if (year == 2023) Electron_photonIdx[iel] = nano.Electron_photonIdx_11p9()[iel];
    else Electron_photonIdx[iel] = nano.Electron_photonIdx()[iel];
  }
}

void getFsrPhoton_muonIdx(nano_tree & nano, int year, vector<int> & FsrPhoton_muonIdx) {
  FsrPhoton_muonIdx.resize(nano.nFsrPhoton());
  for(int ipart(0); ipart<nano.nFsrPhoton(); ++ipart){
    if (year == 2023) FsrPhoton_muonIdx[ipart] = nano.FsrPhoton_muonIdx_11p9()[ipart];
    else FsrPhoton_muonIdx[ipart] = nano.FsrPhoton_muonIdx()[ipart];
  }
}

void getPhoton_jetIdx(nano_tree & nano, int year, vector<int> & Photon_jetIdx) {
  Photon_jetIdx.resize(nano.nPhoton());
  for(int ipart(0); ipart<nano.nPhoton(); ++ipart){
    if (year == 2023) Photon_jetIdx[ipart] = nano.Photon_jetIdx_11p9()[ipart];
    else Photon_jetIdx[ipart] = nano.Photon_jetIdx()[ipart];
  }
}

void getPhoton_cutBased(nano_tree & nano, int year, vector<int> & Photon_cutBased) {
  Photon_cutBased.resize(nano.nPhoton());
  for(int ipart(0); ipart<nano.nPhoton(); ++ipart){
    if (year == 2023) Photon_cutBased[ipart] = nano.Photon_cutBased_11p9()[ipart];
    else Photon_cutBased[ipart] = nano.Photon_cutBased()[ipart];
  }
}
