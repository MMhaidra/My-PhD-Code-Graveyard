from ROOT import *
import sys
infile = TFile(sys.argv[1],"READ")
outname = sys.argv[2]
infile.ls()
hist = infile.Get("h3_diff")
#hist = infile.Get("histNSTest")
#hist = infile.Get("histGS")
c1 = TCanvas("c1","c1",1000,800)
c1.Divide(2,2)

for i in range(hist.GetNbinsX()):
    for j in range(hist.GetNbinsY()):
        for k in range(hist.GetNbinsZ()):
            hist.SetBinContent(i+1,j+1,k+1, hist.GetBinContent(i+1,j+1,k+1))
# XY Slice
c1.cd(1)
templ1 = hist.Project3D("yx")
templ1.Reset()
templ1.GetXaxis().SetRangeUser(-600,600)
templ1.GetXaxis().SetTitle("X (mm)")
templ1.GetYaxis().SetRangeUser(-600,600)
templ1.GetYaxis().SetTitle("Y (mm)")
templ1.GetZaxis().SetTitle("Discriminator")
gStyle.SetOptStat(0)

templ1.Reset()
i = hist.GetNbinsZ()/2
for j in range(hist.GetNbinsX()):
    for k in range(hist.GetNbinsY()):        
        
        val = hist.GetBinContent(j+1,k+1,i+1)
        templ1.SetBinContent(j+1,k+1,  val)
#if templ1.GetBinContent(j+1,k+1) == 0.0: templ1.SetBinContent(j+1,k+1,1)

templ1.SetTitle("XY")
templ1.GetZaxis().SetRangeUser(0.,2.)
#box = TBox(-50.0,-50.0,50.0,50.0)
#box.SetLineColor(kRed)
#box.SetLineStyle(7)
#box.SetFillStyle(0)
gStyle.SetOptTitle(1)
gStyle.SetPalette(55) 
#templ1.Smooth(1,"k3a")
templ1.Draw("COLZ0")
#templ1.Draw("CONTZ")
#box.Draw("SAME")
#gPad.SetGridx(1)
#gPad.SetGridy(1)
gPad.SetRightMargin(0.15)
gPad.Update()


#Drawing a 3D map
c1.cd(2)
hist.GetXaxis().SetRangeUser(-600,600)
#hist.GetXaxis().SetTitle("X (mm)")
hist.GetYaxis().SetRangeUser(-600,600)
#hist.GetYaxis().SetTitle("Y (mm)")
hist.GetZaxis().SetRangeUser(-600,600)

#hist.GetZaxis().SetTitle("Discriminator")
gStyle.SetOptStat(0)
hist.SetTitle("3D Map")
#hist.GetZaxis().SetRangeUser(0.,2.)
gStyle.SetOptTitle(1)
gStyle.SetCanvasPreferGL(1)
#gStyle.SetPalette(55) #107
hist.Draw("BOX1")
#templ1.Smooth(1,"k3a")
gPad.SetRightMargin(0.15)
gPad.Update()

#plot the canvas
c1.Update()
c1.SaveAs(outname)
