// M.Mhaidra 2020/11/30
// This macro analyzes efficiency metrics for different voxel configurations 
// in STE3 bitumen drums with hydrogen bubbles

#include <iostream>
#include <memory>
#include <vector>
#include <string>

// ROOT graphics and histograms
#include "TCanvas.h"
#include "TH1D.h"
#include "TH3F.h"
#include "TStyle.h"
#include "TFile.h"

class EfficiencyAnalyzer {
private:
    struct AnalysisConfig {
        // Histogram configuration
        double medianMin = 10.9;
        double medianMax = 11.9;
        int nBins = 100;
        
        // Geometric cuts
        double xRange = 190;
        double yzRange = 65;
        
        // File paths
        const char* signalFile = "/home/mmhaidra/SliceMethod/Chance_largedrum_bub_tracks35_discr_STE3_dense/"
                                "largedrum_20L_dense_newmetrics_10cm_5cm_3cmVoxel_All.discriminator.root";
        const char* backgroundFile = "/home/mmhaidra/SliceMethod/Chance_largedrum_bub_tracks35_discr_STE3_dense/"
                                   "largedrum_onlybitumen_dense_newmetrics_10cm_5cm_3cmVoxel_All.discriminator.root";
    };

    // Helper function to load 3D histogram from file
    std::unique_ptr<TH3F> LoadHistogram(const char* filename, const char* histName) {
        std::unique_ptr<TFile> file(new TFile(filename, "READ"));
        if (!file || file->IsZombie()) {
            std::cerr << "Failed to open file: " << filename << std::endl;
            return nullptr;
        }

        TH3F* hist = (TH3F*)file->Get(histName);
        if (hist) hist->SetDirectory(0); // Important: Detach from file directory
        return std::unique_ptr<TH3F>(hist);
    }

    // Process 3D histogram into 1D projection with cuts
    std::unique_ptr<TH1D> Process3DHistogram(TH3F* hist3D, const char* name, 
                                            const AnalysisConfig& config) {
        if (!hist3D) return nullptr;

        auto hist1D = std::make_unique<TH1D>(name, "Discr", config.nBins, 
                                            config.medianMin, config.medianMax);

        // Iterate through all bins within specified range
        for (int i = hist3D->GetXaxis()->FindBin(-config.xRange); 
             i < hist3D->GetXaxis()->FindBin(config.xRange); i++) {
            for (int j = hist3D->GetYaxis()->FindBin(-config.yzRange);
                 j < hist3D->GetYaxis()->FindBin(config.yzRange); j++) {
                for (int k = hist3D->GetZaxis()->FindBin(-config.yzRange);
                     k < hist3D->GetZaxis()->FindBin(config.yzRange); k++) {
                    hist1D->Fill(hist3D->GetBinContent(i,j,k));
                }
            }
        }
        return hist1D;
    }

public:
    void Analyze() {
        AnalysisConfig config;

        // Load 3D histograms
        auto hist3D_signal = LoadHistogram(config.signalFile, "histMedianMetric");
        auto hist3D_background = LoadHistogram(config.backgroundFile, "histMedianMetric");

        if (!hist3D_signal || !hist3D_background) {
            std::cerr << "Failed to load histograms" << std::endl;
            return;
        }

        // Process histograms
        auto h_signal = Process3DHistogram(hist3D_signal.get(), "h_discr1", config);
        auto h_background = Process3DHistogram(hist3D_background.get(), "h_discr2", config);

        // Configure histograms
        h_signal->SetLineColor(kRed);
        h_background->SetLineColor(kBlack);
        
        h_signal->GetYaxis()->SetTitle("Entries");
        h_signal->GetXaxis()->SetTitle("Median Metric");
        h_signal->SetTitle("Median of metric distributions for voxels in hydrogen bubbles and "
                          "voxels in bitumen (3cm Voxel) 20L");

        // Print statistics
        std::cout << "Signal Entries: " << h_signal->GetEntries() 
                 << " Background Entries: " << h_background->GetEntries() << std::endl;

        // Create canvas and draw
        gStyle->SetOptStat(0);
        auto canvas = std::make_unique<TCanvas>("cAll1", "Graph", 200, 100, 1300, 700);
        
        h_signal->Draw();
        h_background->Draw("SAME");
        
        canvas->SaveAs("111A_MedianMetric_3cmVoxel_20L_SignalVsBkg.pdf", "pdf");
    }
};

void Eff() {
    EfficiencyAnalyzer analyzer;
    analyzer.Analyze();
}

int main(int argc, char** argv) {
    Eff();
    return 0;
}
