from ROOT import *
import sys
infile = TFile(sys.argv[1],"READ")
outname = sys.argv[2]
infile.ls()
hist = infile.Get("histNS")
c1 = TCanvas("c1","c1",1000,800)
c1.Divide(2,2)

for i in range(hist.GetNbinsX()):
    for j in range(hist.GetNbinsY()):
        for k in range(hist.GetNbinsZ()):
            hist.SetBinContent(i,j,k, hist.GetBinContent(i,j,k))
# XY Slice
c1.cd(1)
templ1 = hist.Project3D("yx");
templ1.GetXaxis().SetRangeUser(-1000,1000)
templ1.GetXaxis().SetTitle("X (mm)")
templ1.GetYaxis().SetRangeUser(-1000,1000)
templ1.GetYaxis().SetTitle("Y (mm)")
templ1.GetZaxis().SetTitle("Number of Scattering")
gStyle.SetOptStat(0)
templ1.SetTitle("XY Projection")
templ1.GetZaxis().SetRangeUser(0,2500)
gStyle.SetOptTitle(1)
gStyle.SetPalette(55)
templ1.Draw("COlz0")
gPad.SetRightMargin(0.15)
gPad.SetLeftMargin(0.15)
gPad.Update()

# XY Slice projection onto X axis
c1.cd(2)
templ2 = templ1.ProjectionX("x",-1000,1000,"o");
templ2.GetXaxis().SetRangeUser(-1000,1000)
templ2.GetXaxis().SetTitle("X (mm)")
templ2.GetYaxis().SetTitle("Average number of Scattering")
gStyle.SetOptStat(0)
j = templ1.GetNbinsY()/2
for i in range(hist.GetNbinsX()):

        val = templ1.GetBinContent(i+1,j+1)
        templ2.SetBinContent(i+1, val)

templ2.SetTitle("XY projection onto X axis")
gStyle.SetOptTitle(1)
templ2.ClearUnderflowAndOverflow()
templ2.SetFillColorAlpha(kBlue, 0.35)
templ2.Draw()
gPad.SetRightMargin(0.15)
gPad.Update()

#plot the canvas
c1.Update()
c1.SaveAs(outname)
