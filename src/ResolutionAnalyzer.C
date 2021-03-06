#ifndef ResolutionAnalyzer_cxx
#define ResolutionAnalyzer_cxx

#include "ResolutionAnalyzer.h"

void ResolutionAnalyzer::ExecuteEvent(){
  RemoveInvalidHit();
  Line* track=NULL;
  bool golden=IsGolden();
  track=GetTrack("TDC");
  TVector3 s1=track->PointWithZ(-270);
  TVector3 s2=track->PointWithZ(270);
  double theta=TMath::Pi()/2-fabs(track->Theta()-TMath::Pi()/2);
  FillHist("scintillator1",s1.X(),s1.Y(),1,600,-300,300,600,-300,300);
  FillHist("scintillator2",s2.X(),s2.Y(),1,600,-300,300,600,-300,300);
  FillHist("theta",theta,1,100,0,5);
  FillHist("phi",track->Phi(),1,200,-5,5);
  FillHist("costheta",cos(theta),1,100,0,1);
  if(golden){
    FillHist("golden/scintillator1",s1.X(),s1.Y(),1,600,-300,300,600,-300,300);
    FillHist("golden/scintillator2",s2.X(),s2.Y(),1,600,-300,300,600,-300,300);
    FillHist("golden/theta",theta,1,200,-5,5);
    FillHist("golden/phi",track->Phi(),1,200,-5,5);
    FillHist("golden/costheta",cos(theta),1,100,0,1);
  }
  for(int i=0;i<NWIRES;i++){
    track=GetTrack("TDC_no_"+GetWireName(i)(0,2));
    if(track){
      if(GetTDC(i)->size()){
	double maxdriftlength=6;
	double scale=1;
	if(!GetWireName(i).BeginsWith("x")) scale=8.5/6;
	
	double z=GetWire(i)->Z();
	TVector3 track_point=track->PointWithZ(z);	
	double x0=track_point.X();
	double y0=track_point.Y();
	
	TVector3 wire_point=GetWire(i)->PointWithX(x0);
	double y1=wire_point.Y()+GetDriftLength(i,GetTDC(i)->at(0))*maxdriftlength*scale;
	double y2=wire_point.Y()-GetDriftLength(i,GetTDC(i)->at(0))*maxdriftlength*scale;
	if(fabs(y1-y0)<fabs(y2-y0)) FillHist("TDC/"+GetWireName(i),(y1-y0)/scale,1,100,-20,20);
	else FillHist("TDC/"+GetWireName(i),(y2-y0)/scale,1,100,-20,20);
	if(golden){
	  if(fabs(y1-y0)<fabs(y2-y0)) FillHist("golden/TDC/"+GetWireName(i),(y1-y0)/scale,1,100,-20,20);
	  else FillHist("golden/TDC/"+GetWireName(i),(y2-y0)/scale,1,100,-20,20);
	}
	FillHist("WireOnly_refTDC/"+GetWireName(i),(wire_point.Y()-y0)/scale,1,100,-20,20);
	if(golden){
	  FillHist("golden/WireOnly_refTDC/"+GetWireName(i),(wire_point.Y()-y0)/scale,1,100,-20,20);
	}
      }
    }
    track=GetTrack("WireOnly_no_"+GetWireName(i)(0,2));
    if(track){
      if(GetTDC(i)->size()){
	double scale=1;
	if(!GetWireName(i).BeginsWith("x")) scale=8.5/6;
	
	double z=GetWire(i)->Z();
	TVector3 track_point=track->PointWithZ(z);	
	double x0=track_point.X();
	double y0=track_point.Y();
	
	TVector3 wire_point=GetWire(i)->PointWithX(x0);
	double y=wire_point.Y();
	FillHist("WireOnly/"+GetWireName(i),(y-y0)/scale,1,100,-20,20);
	if(golden){
	  FillHist("golden/WireOnly/"+GetWireName(i),(y-y0)/scale,1,100,-20,20);
	}
      }
    }
  }
}
Line* ResolutionAnalyzer::ReconstructTrack(TString algorithm){
  Line* track=NULL;
  if(algorithm.Contains(TRegexp("_no_[xuv][0-9]"))){
    TString layer=algorithm(TRegexp("[xuv][0-9]$"));
    vector<vector<int>> temp;
    for(int i=0;i<16;i++){
      int iw=GetWireNumber(layer+"_0")+i;
      temp.push_back(*GetTDC(iw));
      GetTDC(iw)->clear();
    }
    track=AnalyzerCore::ReconstructTrack(Replace(algorithm,"_no_"+layer,""));
    for(int i=0;i<16;i++){
      int iw=GetWireNumber(layer+"_0")+i;
      *GetTDC(iw)=temp[i];
    }
  }else track=AnalyzerCore::ReconstructTrack(algorithm);
  return track;
}
    

bool ResolutionAnalyzer::IsGolden() const {
  vector<TString> layers={"x3","u3","v3","x4","u4","v4"};
  for(int i=0;i<NWIRES;i++){
    if(GetTDC(i)->size()>2){
      return false;
    }
  }
  int goodlayer=0;
  for(const auto& layer:layers){
    if(GetTDCCount(layer)>2||GetTDCCount(layer)<1) continue;
    if(GetTDCCount(layer)==2){
      vector<int> wires;
      for(int i=0;i<16;i++)
	if(GetTDC(layer+Form("_%d",i))->size())
	  wires.push_back(i);
      if(abs(wires[0]-wires[1])!=1){
	continue;	
      }
    }
    goodlayer++;
  }   
  if(goodlayer>=5) return true;
  else return false;
}
void ResolutionAnalyzer::RemoveInvalidHit(){
  for(int i=0;i<NWIRES;i++){
    vector<int> tdc;
    for(int t:*GetTDC(i)){
      if(t>900&&t<2000) tdc.push_back(t);
    }
    *GetTDC(i)=tdc;
  }
}
#endif
