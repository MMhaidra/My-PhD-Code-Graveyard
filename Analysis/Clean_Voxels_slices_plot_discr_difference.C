// M.Mhaidra 10/09/2019


#include <algorithm>
#include <iterator>
#include <vector>
#include <math.h>
#include <numeric>
#include <iomanip>
#include "TCanvas.h"
#include "TTree.h"
#include "TH3D.h"
#include "TH3F.h"
#include "TH2D.h"
#include "TH1D.h"
#include "TFile.h"
#include "TStyle.h"
#include "TRandom.h"
#include "TLine.h"
#include "TPaveText.h"
#include "TLegend.h"

struct HistogramData {
    TH3F* hist;
    const char* filepath;
    const char* histname;
};

// Helper functions
TH3F* LoadHistogram(const char* filepath, const char* histname) {
    TFile* file = new TFile(filepath, "READ");
    if (!file) return nullptr;
    
    TH3F* hist = (TH3F*)file->Get(histname);
    if (hist) hist->SetDirectory(0);
    
    file->Close();
    delete file;
    return hist;
}

void SaveHistogram(TH3F* hist, const char* filename) {
    TFile* outFile = new TFile(filename, "RECREATE");
    hist->Write();
    outFile->Close();
    delete outFile;
}

void ComputeDifference(TH3F* hist1, TH3F* hist2, TH3F* diffHist) {
    double NbinsXCenter = diffHist->GetNbinsX()/2;
    double NbinsYCenter = diffHist->GetNbinsY()/2;
    double NbinsZCenter = diffHist->GetNbinsZ()/2;
    
    diffHist->Reset();
    
    for (int i = hist1->GetXaxis()->FindBin(-440); i < hist1->GetXaxis()->FindBin(440); i++) {
        for (int j = hist1->GetYaxis()->FindBin(-300); j < hist1->GetYaxis()->FindBin(300); j++) {
            for (int k = hist1->GetZaxis()->FindBin(-300); k < hist1->GetZaxis()->FindBin(300); k++) {
                if (hist1->GetBinContent(i,j,k) == 0. || hist2->GetBinContent(i,j,k) == 0.) continue;
                
                diffHist->SetBinContent(i,j,k, hist1->GetBinContent(i,j,k) - hist2->GetBinContent(i,j,k));
            }
        }
    }
}

void Clean_Voxels_slices_plot_discr_difference() {
    // Input files configuration
    HistogramData data1 = {
        nullptr,
        "/home/mmhaidra/SliceMethod/Chance_largedrum_bub_tracks35_discr_STE3/largedrum_bub_STE3_OnlyBitumen_Isotest_0.25L_ForVoxelsCleaning_tracks35.discriminator.root",
        "histBS"
    };
    
    HistogramData data2 = {
        nullptr,
        "/home/mmhaidra/SliceMethod/Chance_largedrum_bub_tracks35_discr_STE3/largedrum_bub_STE3_OnlyBitumen_ForVoxelsCleaning_tracks35.discriminator.root",
        "histBS"
    };

    // Load histograms
    data1.hist = LoadHistogram(data1.filepath, data1.histname);
    data2.hist = LoadHistogram(data2.filepath, data2.histname);
    
    if (!data1.hist || !data2.hist) {
        std::cerr << "Failed to load histograms" << std::endl;
        return;
    }

    // Create difference histogram
    TH3F* h3_diff = (TH3F*)data1.hist->Clone("h3_diff");
    ComputeDifference(data1.hist, data2.hist, h3_diff);

    // Save results
    SaveHistogram(h3_diff, "Isotest_0.25L_STE3_35tracks_Without_Bkg.root");

    // Cleanup
    delete data1.hist;
    delete data2.hist;
    delete h3_diff;
}

int main(int argc, char** argv) {
    Clean_Voxels_slices_plot_discr_difference();
    return 0;
}
