# M.Mhaidra 09/2019
from ROOT import *
#import ROOT
import sys
from pprint import pformat
#from root_numpy import array2hist, hist2array
import numpy as np
from rootpy.plotting import Hist3D
import root_numpy as rnp

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


hist.GetXaxis().SetRangeUser(-600,600)
hist.GetYaxis().SetRangeUser(-600,600)
hist.GetZaxis().SetRangeUser(-600,600)
# XY Slice
c1.cd(1)
templ1 = hist.Project3D("yx");
#templ1.Reset()
templ1.GetXaxis().SetRangeUser(-600,600)
templ1.GetXaxis().SetTitle("X (mm)")
templ1.GetYaxis().SetRangeUser(-600,600)
templ1.GetYaxis().SetTitle("Y (mm)")
templ1.GetZaxis().SetTitle("Discriminator Difference")
gStyle.SetOptStat(0)
templ1.SetTitle("XY Projection")
templ1.GetZaxis().SetRangeUser(-2.,8.)
gStyle.SetOptTitle(1)
gStyle.SetPalette(55) #107
#templ1.Draw("COLZ0")
templ1.Smooth(1,"k3a")
templ1.Draw("CONTZ")
gPad.SetRightMargin(0.15)
gPad.Update()
#templ1.SaveAs(outname1)

# XY Slice projection onto X axis
c1.cd(2)
templ2 = templ1.ProjectionX("x",templ1.GetYaxis().FindBin(-380),templ1.GetYaxis().FindBin(380),"o");
#templ3 = templ1.ProjectionX("x",80,120,"o");
templ2.GetXaxis().SetRangeUser(-410,410)
templ2.GetXaxis().SetTitle("X (mm)")
templ2.GetYaxis().SetTitle("Average Discriminator Difference")
gStyle.SetOptStat(0)
j = templ1.GetNbinsY()/2
for i in range(hist.GetNbinsX()):

        val = templ1.GetBinContent(i+1,j+1)
        templ2.SetBinContent(i+1, val)

templ2.SetTitle("XY Slice projection onto X axis")
gStyle.SetOptTitle(1)
templ2.ClearUnderflowAndOverflow()
templ2.SetFillColorAlpha(kGreen, 0.55)
#templ2.GetXaxis().SetLimits(-400, 400)
templ2.Draw()
gPad.SetRightMargin(0.15)
gPad.Update()


def get_gpeaks(templ2,lrange=[1000,3800],sigma=2,opt="",thres=0.05,niter=20):
    s = TSpectrum(niter)
    templ2.GetXaxis().SetRange(lrange[0],lrange[1])
    s.Search(templ2,sigma,opt,thres)
    bufX, bufY = s.GetPositionX(), s.GetPositionY()
    pos = []
    for i in range(s.GetNPeaks()):
        pos.append([bufX[i], bufY[i]])
    pos.sort()
    return pos

print("Position of bubbles : ", get_gpeaks(templ2,lrange=[-410,410],sigma=3.8,opt="",thres=0.05,niter=20))



def get_hist3d(histname, get_overflow=False):
    hist = infile.Get("h3_diff")
    xlims = np.array(list(hist.xedges()))
    ylims = np.array(list(hist.yedges()))
    zlims = np.array(list(hist.zedges()))
    bin_values = rnp.hist2array(hist, include_overflow=get_overflow)
    return bin_values, xlims, ylims, zlims


#plot the canvas
c1.Update()
c1.SaveAs(outname)




