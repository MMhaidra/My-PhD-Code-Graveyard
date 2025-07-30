# M.Mhaidra 2019 #
##################

from ROOT import TFile, gStyle, gPad
from array import array
import sys

def setup_style():
    """Configure ROOT plotting style"""
    gStyle.SetOptStat(0)
    gStyle.SetPalette(55)  # Using rainbow color palette (55)
    
def load_histogram(filepath):
    """Load the 3D histogram from ROOT file"""
    try:
        infile = TFile(filepath, "READ")
        hist = infile.Get("histMedianMetric")
        if not hist:
            raise RuntimeError("Could not find histogram 'histMedianMetric'")
        return infile, hist
    except Exception as e:
        print(f"Error loading file: {e}")
        sys.exit(1)

def process_slice(hist, slice_index):
    """Process a single Z slice of the 3D histogram"""
    templ = hist.Project3D("yx")
    templ.Reset()
    
    # Fill template histogram with values from the z-slice
    for x in range(hist.GetNbinsX()):
        for y in range(hist.GetNbinsY()):
            val = hist.GetBinContent(x, y, slice_index)
            # Flip Y axis for better visualization
            templ.SetBinContent(x, templ.GetNbinsY() - y, val)
    
    return templ

def plot_slice(template, slice_index, total_slices):
    """Plot a single slice and save to PDF"""
    template.GetZaxis().SetRangeUser(11.256, 15.)
    template.Smooth(1, "k3a")
    template.Draw("COLZ0")
    
    # Set margins for better visibility
    gPad.SetRightMargin(0.15)
    gPad.SetLeftMargin(0.15)
    gPad.Update()
    
    # Handle PDF creation with proper opening/closing
    output_name = "1test_1L_medianCut_slices.pdf"
    if slice_index == 0:
        gPad.Print(f"{output_name}(")  # Open PDF
    elif slice_index == total_slices - 1:
        gPad.Print(f"{output_name})")  # Close PDF
    else:
        gPad.Print(output_name)

def main():
    # Initialize ROOT style
    setup_style()
    
    # Load input file and histogram
    infile, hist = load_histogram("/home/mmhaidra/SliceMethod/largedrum_1L_dense_MedianCut_3cmVoxel.discriminator.root")
    
    # Process each Z slice
    n_slices = hist.GetNbinsZ()
    for i in range(n_slices):
        template = process_slice(hist, i)
        plot_slice(template, i, n_slices)
    
    infile.Close()

if __name__ == "__main__":
    main()
