from ROOT import TFile, TCanvas, gStyle, gPad, kGreen
import sys

def check_arguments():
    """Validate command line arguments"""
    if len(sys.argv) < 3:
        print("Usage: python plot_bristol.py <input_file> <output_name>")
        sys.exit(1)
    return sys.argv[1], sys.argv[2]

def load_histogram(filepath):
    """Load and validate input histogram"""
    try:
        infile = TFile(filepath, "READ")
        hist = infile.Get("histMedianMetric")
        if not hist:
            raise RuntimeError("Could not find histogram 'histMedianMetric'")
        return infile, hist
    except Exception as e:
        print(f"Error loading file: {e}")
        sys.exit(1)

def setup_style():
    """Configure ROOT plotting style"""
    gStyle.SetOptStat(0)
    gStyle.SetOptTitle(1)
    gStyle.SetPalette(107)  # Color palette setting

def create_xy_slice(hist):
    """Create and configure XY slice projection"""
    templ1 = hist.Project3D("yx")
    templ1.Reset()
    
    # Configure axes
    templ1.GetXaxis().SetRangeUser(-600, 600)
    templ1.GetXaxis().SetTitle("X (mm)")
    templ1.GetYaxis().SetRangeUser(-600, 600)
    templ1.GetYaxis().SetTitle("Y (mm)")
    templ1.GetZaxis().SetTitle("Median of Log(Metric)")
    
    # Fill histogram with middle Z slice
    i = hist.GetNbinsZ()//2
    for j in range(hist.GetNbinsX()):
        for k in range(hist.GetNbinsY()):
            val = hist.GetBinContent(j+1, k+1, i+1)
            templ1.SetBinContent(j+1, k+1, val)
    
    templ1.SetTitle("XY")
    templ1.GetZaxis().SetRangeUser(11.256, 15.)
    templ1.Smooth(1, "k3a")
    return templ1

def create_x_projection(templ1):
    """Create X-axis projection from XY slice"""
    templ3 = templ1.ProjectionX("x", 80, 120, "o")
    templ3.GetXaxis().SetRangeUser(-500, 500)
    templ3.GetXaxis().SetTitle("X (mm)")
    templ3.GetYaxis().SetTitle("Average Median of Log(Metric)")
    
    # Fill projection with middle Y slice
    j = templ1.GetNbinsY()//2
    for i in range(templ1.GetNbinsX()):
        val = templ1.GetBinContent(i+1, j+1)
        templ3.SetBinContent(i+1, val)
    
    templ3.SetTitle("XY projection onto X axis")
    templ3.ClearUnderflowAndOverflow()
    templ3.SetFillColorAlpha(kGreen, 0.35)
    return templ3

def main():
    # Process command line arguments
    input_file, output_name = check_arguments()
    
    # Setup
    setup_style()
    infile, hist = load_histogram(input_file)
    
    # Create canvas
    c1 = TCanvas("c1", "c1", 1000, 800)
    c1.Divide(2, 2)
    
    # Create and draw XY slice
    c1.cd(1)
    templ1 = create_xy_slice(hist)
    templ1.Draw("COLZ0")
    gPad.SetRightMargin(0.15)
    gPad.Update()
    
    # Create and draw X projection
    c1.cd(2)
    templ3 = create_x_projection(templ1)
    templ3.Draw("CONTZ")
    gPad.SetRightMargin(0.15)
    gPad.Update()
    
    # Save output
    c1.Update()
    c1.SaveAs(output_name)
    infile.Close()

if __name__ == "__main__":
    main()
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
templ3.SetFillColorAlpha(kGreen, 0.35)
#gStyle.SetHistFillColor(3)
#gStyle.SetHistFillStyle(1)
# save the histogram
#templ3.SaveAs(outname2)

# Z Slice profile

#c1.cd(4)
#templ4 = templ1.ProfileX("px",80,120,"o");
#templ4.GetXaxis().SetRangeUser(-600,600)
#templ4.GetXaxis().SetTitle("X (mm)")
#templ4.GetYaxis().SetTitle("Average Discriminator")
#gStyle.SetOptStat(0)

#j = templ1.GetNbinsY()/2
#for i in range(hist.GetNbinsX()):

#  val = templ1.GetBinContent(i+1,j+1)
#  templ4.SetBinContent(i+1, val)

#templ4.SetTitle("X Discriminator profile")
#gStyle.SetOptTitle(1)
#templ4.Draw("CONTZ")
#gPad.SetGridx(1)
#gPad.SetGridy(1)
#gPad.SetRightMargin(0.15)
#gPad.Update()

# save the histogram
#templ4.SaveAs(outname3)


#plot the canvas
c1.Update()
c1.SaveAs(outname)
