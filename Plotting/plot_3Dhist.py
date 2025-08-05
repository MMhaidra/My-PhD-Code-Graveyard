# M.Mhaidra 09/2019
from ROOT import *
import sys
infile = TFile(sys.argv[1],"READ")
outname = sys.argv[2]
infile.ls()
#hist = infile.Get("h3_diff")
hist = infile.Get("histMedianMetric_3D")
c1 = TCanvas("c1","c1",1000,800)
c1.Divide(2,2)

for i in range(hist.GetNbinsX()):
    for j in range(hist.GetNbinsY()):
        for k in range(hist.GetNbinsZ()):
            hist.SetBinContent(i,j,k, hist.GetBinContent(i,j,k))

hist.GetXaxis().SetRangeUser(-600,600)
hist.GetYaxis().SetRangeUser(-600,600)
hist.GetZaxis().SetRangeUser(-600,600)
# XZ Slice
c1.cd(1)
templ2 = hist.Project3D("zy");
templ2.GetYaxis().SetRangeUser(-600,600)
templ2.GetZaxis().SetRangeUser(-600,600)
templ2.GetYaxis().SetTitle("Z (mm)")
templ2.GetXaxis().SetTitle("Y (mm)")
templ2.GetZaxis().SetTitle("Median of Log(Metric)")
gStyle.SetOptStat(0)
templ2.SetTitle("YZ Projection")
templ2.GetZaxis().SetRangeUser(11.26,14.)
gStyle.SetOptTitle(1)
gStyle.SetPalette(55)
#templ2.Smooth(1,"k3a")
templ2.Draw("CONTZ")
gPad.SetRightMargin(0.15)
gPad.Update()



hist.GetXaxis().SetRangeUser(-600,600)
hist.GetYaxis().SetRangeUser(-600,600)
hist.GetZaxis().SetRangeUser(-600,600)
# XY Slice
c1.cd(2)
templ1 = hist.Project3D("yx");
#templ1.Reset()
templ1.GetXaxis().SetRangeUser(-600,600)
templ1.GetXaxis().SetTitle("X (mm)")
templ1.GetYaxis().SetRangeUser(-600,600)
templ1.GetYaxis().SetTitle("Y (mm)")
templ1.GetZaxis().SetTitle("Median of Log(Metric)")
gStyle.SetOptStat(0)
#templ1.Reset()
#i = hist.GetNbinsZ()/2
#for j in range(hist.GetNbinsX()):
#for k in range(hist.GetNbinsY()):
# val = hist.GetBinContent(j+1,k+1,i+1)
# print val
# templ1.SetBinContent(j+1,k+1,  val)
#if templ1.GetBinContent(j,k) > 8: templ1.SetBinContent(j,k,10)
templ1.SetTitle("XY Projection")
templ1.GetZaxis().SetRangeUser(11.26,14.)
gStyle.SetOptTitle(1)
gStyle.SetPalette(55) #107
#templ1.Draw("COLZ0")
#templ1.Smooth(1,"k3a")
templ1.Draw("CONTZ")
gPad.SetRightMargin(0.15)
gPad.Update()
#templ1.SaveAs(outname1)


# XY Slice projection onto X axis
c1.cd(3)
templ4 = templ1.ProjectionX("x",-480,480,"o");
#templ3 = templ1.ProjectionX("x",80,120,"o");
templ4.GetXaxis().SetRangeUser(-500,500)
templ4.GetXaxis().SetTitle("X (mm)")
templ4.GetYaxis().SetTitle("Average Median of Log(Metric)")
gStyle.SetOptStat(0)
j = templ1.GetNbinsY()/2
for i in range(hist.GetNbinsX()):

        val = templ1.GetBinContent(i+1,j+1)
        templ4.SetBinContent(i+1, val)

templ4.SetTitle("XY Slice projection onto X axis")
gStyle.SetOptTitle(1)
templ4.ClearUnderflowAndOverflow()
templ4.SetFillColorAlpha(kBlue, 0.35)
templ4.Draw()
gPad.SetRightMargin(0.15)
gPad.Update()
#gPad.SetLogy()
#gPad.Update()



#plot the canvas
c1.Update()
c1.SaveAs(outname)
