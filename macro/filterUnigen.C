bool removeEvent(UEvent *event)
{
  if(event->GetB() > 4.) return true;
  return false;
}

bool removeParticle(UParticle *part)
{
  if(part->GetPdg() != 2212) return true;
  return false;
}

void filterUnigen (TString inPath="~/desktop/analysis/mc/model_root/dcmqgsm_smm_auau_pbeam12agev/dcmqgsm_1.root", TString outPath="out.root", int eventsPerFile=200)
{
  outPath.ReplaceAll(".root","");
  TChain ch("events");
  ch.Add(inPath);
  TFile *f{nullptr};
  TTree *t{nullptr};
  int fileNumber=1;
  int nAcceptedEvents=0;
  auto nEvents=ch.GetEntries();
  UEvent *event{nullptr};
  ch.SetBranchAddress("event", &event);
  auto runInfo=(URun*)(ch.GetFile()->Get("run"));
  for (int e=0;e<nEvents;e++)
  {
    cout << "\r " << e << " / " << nEvents;
    if(nAcceptedEvents == 0)
    {  
      f=new TFile(Form("%s_%d.root", outPath.Data(), fileNumber), "recreate");
      t=ch.CloneTree(0);
    }
    
    cout << "\t" << nAcceptedEvents << "\t" << eventsPerFile << "\t" << e << "\t" << nEvents << endl;    
 
    ch.GetEntry(e);
    if (removeEvent(event))
      continue;
    nAcceptedEvents++;
    int nParticles=event->GetNpa();
    for (int p=0;p<nParticles;p++)
    {
      if (removeParticle(event->GetParticle(p)))
      {
        event->RemoveAt(p);
        p--; // new indexing - see UEvent::RemoveAt(int i)
        nParticles--; // new indexing - see UEvent::RemoveAt(int i)
      }
    }
    t->Fill();
    if(nAcceptedEvents == eventsPerFile || e == nEvents-1)
    {
      runInfo->SetNEvents(nAcceptedEvents);
      runInfo->Write();
      t->Write();
      f->Close();
      nAcceptedEvents=0;
      fileNumber++;
    }
    //event->Clear();
  }
  cout << endl;
}
