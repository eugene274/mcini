bool removeParticle(UParticle *part)
{
  if(part->GetPdg() != 2212) return true;
  return false;
}

void filterUnigen (TString inPath="~/desktop/analysis/mc/model_root/urqmd_12.root", TString outPath="out.root", int eventsPerFile=1000)
{
  outPath.ReplaceAll(".root","");
  TChain ch("events");
  ch.Add(inPath);
  TFile *f{nullptr};
  TTree *t{nullptr};
  int fileNumber=0;
  auto nEvents=ch.GetEntries();
  UEvent *event{nullptr};
  ch.SetBranchAddress("event", &event);
  ch.GetEntry(0);
  auto runInfo=(URun*)(ch.GetFile()->Get("run"));
  for (int e=0;e<nEvents;e++)
  {
    ch.GetEntry(e);
    cout << "\r " << e << " / " << nEvents;
    if(e % eventsPerFile == 0)
    {
      if(f)
      {
        runInfo->SetNEvents(t->GetEntries());
        runInfo->Write();
        t->Write();
        f->Close();
	fileNumber++;
      }
      f=new TFile(Form("%s_%d.root", outPath.Data(), fileNumber), "recreate");
      t=ch.CloneTree(0);
    }
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
    event->Clear();
  }
  runInfo->SetNEvents(t->GetEntries());
  runInfo->Write();
  t->Write();
  f->Close();
  cout << endl;
}
