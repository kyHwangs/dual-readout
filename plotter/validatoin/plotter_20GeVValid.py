import cmsstyle as CMS
import ROOT

CMS.setCMSStyle()
ROOT.gROOT.SetBatch(ROOT.kTRUE)
ROOT.TH1.AddDirectory(False)


def main():
  TargetFile = ROOT.TFile("~/DRC/v001_FastOptPaper/STORAGE/FastSim/electron/noInc_2500mm_20GeV/summary.root", "READ")
  RefFile = ROOT.TFile("~/DRC/v001_FastOptPaper/STORAGE/FullSim/electron/noInc_2500mm_20GeV/summary.root", "READ")

  HistoList = ["ALL_Stime", "ALL_Shit", "ALL_SWave"]

  xTitle = {
    "ALL_Stime": "Time [ns]",
    "ALL_Shit": "Number of Hits",
    "ALL_SWave": "Wavelength [nm]",
  }

  for fHist in HistoList:

    fHist_target = TargetFile.Get(fHist)
    fHist_ref = RefFile.Get(fHist)

    fHist_ratio = fHist_ref.Clone("ratio")
    fHist_ratio.Divide(fHist_target)
    
    fDiCanv = CMS.cmsDiCanvas(
        f"canv_{fHist}",
        fHist_target.GetBinLowEdge(1),
        fHist_target.GetBinLowEdge(fHist_target.GetNbinsX()) + fHist_target.GetBinWidth(fHist_target.GetNbinsX()),
        0,
        1.3 * max(fHist_target.GetMaximum(), fHist_ref.GetMaximum()),
        0.8,
        1.2,
        xTitle[fHist],
        "N_{p.e.}",
        "FastSim / FullSim",
        square = True,
        extraSpace = 0.1,
        iPos = 0,
    )

    fLeg = CMS.cmsLeg(0.70, 0.89 - 0.05 * 2, 0.89, 0.89, textSize=0.03)
    fLeg.AddEntry(fHist_target, "FullSim", "lp")
    fLeg.AddEntry(fHist_ref, "FastSim", "lp")

    fDiCanv.cd(1)
    CMS.cmsDraw(fHist_target, "P", mcolor=ROOT.kBlack, msize=0.7)
    CMS.cmsDraw(fHist_ref, "P", mcolor=ROOT.kRed, msize=0.7)

    fDiCanv.cd(2)
    CMS.cmsDraw(fHist_ratio, "P", mcolor=ROOT.kRed, msize=0.7)

    CMS.SaveCanvas(fDiCanv, f"Valid_{fHist}.pdf")

  TargetFile.Close()
  RefFile.Close()

if __name__ == "__main__" :
    main()