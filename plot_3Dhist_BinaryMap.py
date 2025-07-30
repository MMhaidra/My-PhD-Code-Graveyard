# M.Mhaidra 10/2021
from ROOT import *
import sys
from pprint import pformat

infile = TFile(sys.argv[1],"READ")
outname = sys.argv[2]
infile.ls()
hist = infile.Get("h3_diff")
c1 = TCanvas("c1","c1",1000,800)
c1.Divide(2,2)

for i in range(hist.GetNbinsX()):
    for j in range(hist.GetNbinsY()):
        for k in range(hist.GetNbinsZ()):
            hist.SetBinContent(i,j,k, hist.GetBinContent(i,j,k))

# XY Slice
c1.cd(1)
templ1 = hist.Project3D("yx")
#templ1.Reset()
templ1.GetXaxis().SetRangeUser(-600,600)
templ1.GetXaxis().SetTitle("X (mm)")
templ1.GetYaxis().SetRangeUser(-600,600)
templ1.GetYaxis().SetTitle("Y (mm)")
templ1.GetZaxis().SetTitle("Discriminator")
gStyle.SetOptStat(0)

templ1.SetTitle("XY Projection")
templ1.GetZaxis().SetRangeUser(0.,4.)
gStyle.SetOptTitle(1)
gStyle.SetPalette(55) # #107
templ1.Draw("COLZ0")
templ1.Smooth(1,"k3a")
gPad.SetRightMargin(0.15)
gPad.Update()
c1.Update()

#Drawing a 3D map
c1.cd(2)
hist.GetXaxis().SetRangeUser(-600,600)
hist.GetYaxis().SetRangeUser(-600,600)
hist.GetZaxis().SetRangeUser(-600,600)
#hist.GetZaxis().SetTitle("Discriminator")
gStyle.SetOptStat(0)
hist.SetTitle("3D Map")
#hist.GetZaxis().SetRangeUser(0.,2.)
gStyle.SetOptTitle(1)
gStyle.SetCanvasPreferGL(1)
hist.Draw("BOX1")
#hist.Smooth(1,"k3a")
gPad.SetRightMargin(0.15)
gPad.Update()
c1.Update()


# ZY Slice
c1.cd(3)
templ2 = hist.Project3D("zy")
#templ1.Reset()
templ2.GetXaxis().SetRangeUser(-600,600)
templ2.GetXaxis().SetTitle("Z (mm)")
templ2.GetYaxis().SetRangeUser(-600,600)
templ2.GetYaxis().SetTitle("Y (mm)")
templ2.GetZaxis().SetTitle("Discriminator")
gStyle.SetOptStat(0)

templ2.SetTitle("YZ Projection")
templ2.GetZaxis().SetRangeUser(0.,4.)
gStyle.SetOptTitle(1)
gStyle.SetPalette(55) # #107
templ2.Draw("COLZ0")
templ2.Smooth(1,"k3a")
gPad.SetRightMargin(0.15)
gPad.Update()
c1.Update()


c1.cd(4)
templ3 = templ1.ProjectionX("x",80,120,"o")
templ3.GetXaxis().SetRangeUser(-500,500)
templ3.GetXaxis().SetTitle("X (mm)")
templ3.GetYaxis().SetTitle("Average Discriminator")
gStyle.SetOptStat(0)
j = templ1.GetNbinsY()/2
for i in range(hist.GetNbinsX()):
    val = templ1.GetBinContent(i+1,j+1)
    templ3.SetBinContent(i+1, val)

templ3.SetTitle("XY projection onto X axis")
gStyle.SetOptTitle(1)
templ3.ClearUnderflowAndOverflow()
templ3.Draw("CONTZ")
#gPad.SetGridx(1)
#gPad.SetGridy(1)
gPad.SetRightMargin(0.15)
gPad.Update()
templ3.SetFillColorAlpha(kRed, 0.35)
print("Sidebands entries 1 : ", templ3.Integral(templ3.GetXaxis().FindBin(-400), templ3.GetXaxis().FindBin(-100)) + templ3.Integral(templ3.GetXaxis().FindBin(100), templ3.GetXaxis().FindBin(400)))


#plot the canvas
c1.Update()
c1.SaveAs(outname)



"""
#XY Slice
c1.cd(1)
templ3 = hist.Project3D("xy")
templ3.Reset()
templ3.GetXaxis().SetRangeUser(-600,600)
templ3.GetXaxis().SetTitle("X (mm)")
templ3.GetYaxis().SetRangeUser(-600,600)
templ3.GetYaxis().SetTitle("Y (mm)")
templ3.GetZaxis().SetTitle("Discriminator")
gStyle.SetOptStat(0)

templ3.Reset()
i = hist.GetNbinsZ()/2
for j in range(hist.GetNbinsX()):
    for k in range(hist.GetNbinsY()):        
        val = hist.GetBinContent(j+1,k+1,i+1)
        templ3.SetBinContent(j+1,k+1,  val)


templ3.SetTitle("XY")
templ3.GetZaxis().SetRangeUser(0.,2.)
gStyle.SetOptTitle(1)
gStyle.SetPalette(55) 
templ1.Smooth(1,"k3a")
templ3.Draw("COLZ0")
gPad.SetRightMargin(0.15)
gPad.Update()
"""