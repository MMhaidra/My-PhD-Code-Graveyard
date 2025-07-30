# M.Mhaidra 09/2019
from ROOT import *
import sys
infile1 = TFile(sys.argv[1],"READ")
outname = sys.argv[2]
infile1.ls()
hist1 = infile1.Get("h3_diff")
c1 = TCanvas("c1","c1",1000,800)
c1.Divide(2,2)


for i in range(hist1.GetNbinsX()):
    for j in range(hist1.GetNbinsY()):
        for k in range(hist1.GetNbinsZ()):
            hist1.SetBinContent(i,j,k, hist1.GetBinContent(i,j,k))



hist1.GetXaxis().SetRangeUser(-600,600)
hist1.GetYaxis().SetRangeUser(-600,600)
hist1.GetZaxis().SetRangeUser(-600,600)
# XY Slice
c1.cd(1)
templ1 = hist1.Project3D("yx");
#templ1.Reset()
templ1.GetXaxis().SetRangeUser(-600,600)
templ1.GetXaxis().SetTitle("X (mm)")
templ1.GetYaxis().SetRangeUser(-600,600)
templ1.GetYaxis().SetTitle("Y (mm)")
templ1.GetZaxis().SetTitle("Discriminator Difference")
gStyle.SetOptStat(0)
templ1.SetTitle("XY Projection")
templ1.GetZaxis().SetRangeUser(-2.,6.)
gStyle.SetOptTitle(1)
gStyle.SetPalette(55) #107
#templ1.Draw("COLZ0")
templ1.Smooth(1,"k3a")
templ1.Draw("CONTZ")
gPad.SetRightMargin(0.15)
gPad.Update()



# XY Slice projection onto X axis
c1.cd(2)
templ2 = templ1.ProjectionX("x",-480,480,"o");
templ2.GetXaxis().SetRangeUser(-500,500)
templ2.GetXaxis().SetTitle("X (mm)")
templ2.GetYaxis().SetTitle("Average Discriminator Difference")
gStyle.SetOptStat(0)
j = templ1.GetNbinsY()/2
for i in range(hist1.GetNbinsX()):

        val = templ1.GetBinContent(i+1,j+1)
        templ2.SetBinContent(i+1, val)

templ2.SetTitle("XY Slice projection onto X axis")
gStyle.SetOptTitle(1)
templ2.ClearUnderflowAndOverflow()
templ2.SetFillColorAlpha(kBlue, 0.35)
templ2.Draw()
gPad.SetRightMargin(0.15)
gPad.Update()




#plot the canvas
c1.Update()
c1.SaveAs(outname)
