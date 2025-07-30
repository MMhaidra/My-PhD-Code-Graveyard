// M.Mhaidra 28/09/2019 //
/* This macro calculates and plots the mean of the discriminator difference
 between the matrix drum and the drum with gas inside.
 The trick is that histograms are associated with a TDirectory
 upon creation, which is most likely the open file. When the
 directory changes when a new file is opened, the pointer to
 the histogram is invalidated. TH3::SetDirectory is used to
 detach a histogram from a file, thus the reconstruction is much more quicker.*/

#include <TCanvas.h>
#include <TFile.h>
#include <TH3F.h>
#include <TH1D.h>
#include <TStyle.h>
#include <string>
#include <vector>

// Helper struct for histogram management
struct HistogramInfo {
    TH3F* hist;
    std::string name;
    double cutValue;
};

// Helper functions
TH3F* LoadHistogram(const char* filepath, const char* histname) {
    TFile f(filepath, "READ");
    TH3F* h = (TH3F*)f.Get(histname);
    if (h) h->SetDirectory(0); // Decouple from file
    return h;
}

void ComputeDifferenceHistogram(TH3F* hist1, TH3F* hist2, TH1D* diff_hist, double xRange = 190) {
    diff_hist->Reset();
    
    for (int i = hist1->GetXaxis()->FindBin(-xRange); i < hist1->GetXaxis()->FindBin(xRange); ++i) {
        for (int j = hist1->GetYaxis()->FindBin(-65); j < hist1->GetYaxis()->FindBin(65); ++j) {
            for (int k = hist1->GetZaxis()->FindBin(-300); k < hist1->GetZaxis()->FindBin(300); ++k) {
                if (hist1->GetBinContent(i,j,k) > 0 && hist2->GetBinContent(i,j,k) > 0) {
                    diff_hist->Fill(hist1->GetBinContent(i,j,k) - hist2->GetBinContent(i,j,k));
                }
            }
        }
    }
}

void SetupHistogram(TH1D* hist, int color, const char* title, const char* xtitle) {
    gStyle->SetOptStat(1111111);
    hist->SetLineColor(color);
    hist->GetYaxis()->SetTitle(title);
    hist->GetXaxis()->SetTitle(xtitle);
    hist->GetYaxis()->SetRangeUser(0,800);
}

void Allparam_difference() {
    // Load input histograms
    TH3F* hist_3D = LoadHistogram("/home/mmhaidra/SliceMethod/Chance_largedrum_bub_tracks35_discr_STE3_dense/largedrum_20L_dense_newmetrics_10cm_5cm_2cmVoxel_All.discriminator.root", "histIntegralHalfMetric");
    TH3F* hist_3D_reference = LoadHistogram("/home/mmhaidra/SliceMethod/Chance_largedrum_bub_tracks35_discr_STE3_dense/largedrum_onlybitumen_dense_newmetrics_10cm_5cm_2cmVoxel_All.discriminator.root", "histIntegralHalfMetric");

    // Create difference histograms
    TH1D* h_discr1 = new TH1D("h_discr1", "Discr", 10, -0.02, 0.02);
    TH1D* h_discr2 = new TH1D("h_discr2", "Discr", 10, -0.02, 0.02);

    // Compute differences
    ComputeDifferenceHistogram(hist_3D, hist_3D_reference, h_discr1);
    ComputeDifferenceHistogram(hist_3D_reference, hist_3D, h_discr2);

    // Setup histograms
    SetupHistogram(h_discr1, kRed, "Entries", "Half Integral Metric");
    SetupHistogram(h_discr2, kBlack, "Entries", "Half Integral Metric"); 

    // Cleanup
    delete hist_3D;
    delete hist_3D_reference;
    delete h_discr1;
    delete h_discr2;
}

int main(int argc, char** argv) {
    Allparam_difference();
    return 0;
}
