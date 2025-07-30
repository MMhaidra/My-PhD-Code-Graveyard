// M.Mhaidra 2020/11/30
// This macro analyzes efficiency metrics for different voxel configurations 
// in STE3 bitumen drums with hydrogen bubbles

#include <iostream>
#include <memory>
#include <vector>
#include <cmath>
// ROOT includes grouped by functionality
// Histogram and Analysis
#include "TH1D.h"
#include "TH3F.h"
#include "TEfficiency.h"
#include "TSpectrum.h"
// Graphics and Style
#include "TCanvas.h"
#include "TStyle.h"
#include "TGraph.h"
#include "TGraphErrors.h"
#include "TMultiGraph.h"
// File and System
#include "TFile.h"
#include "TSystem.h"

class EfficiencyAnalyzer {
private:
    struct AnalysisConfig {
        // Geometric parameters (in mm)
        double dX = 30;  // 30mm voxel size
        double dY = 30;
        double dZ = 30;
        
        // Histogram parameters
        int nBins = 100;
        double histMin = 9.0;
        double histMax = 14.0;
        
        // File paths
        const char* bitumenFile = "/home/mmhaidra/SliceMethod/largedrum_onlybitumen_dense_"
                                 "newmetrics_3cmVoxel_April2021.discriminator.root";
        const char* hydrogenFile = "/home/mmhaidra/SliceMethod/largedrum_onlyhydrogen_dense_"
                                  "newmetrics_3cmVoxel_April2021.discriminator.root";
    };

    struct HistogramPair {
        std::unique_ptr<TH3F> signal;
        std::unique_ptr<TH3F> background;
    };

    // Load 3D histograms with proper memory management
    HistogramPair LoadHistograms(const AnalysisConfig& config) {
        auto file1 = std::make_unique<TFile>(config.bitumenFile, "READ");
        auto file2 = std::make_unique<TFile>(config.hydrogenFile, "READ");
        
        HistogramPair result;
        if (file1 && file2) {
            // Important: Clone and detach histograms from files to prevent memory issues
            result.signal.reset((TH3F*)file1->Get("histMedianMetric"));
            result.background.reset((TH3F*)file2->Get("histMedianMetric"));
            
            if (result.signal) result.signal->SetDirectory(0);
            if (result.background) result.background->SetDirectory(0);
        }
        return result;
    }

    // Process 3D histogram with geometric cuts
    std::unique_ptr<TH1D> Process3DHistogram(TH3F* hist3D, const char* name, 
                                           bool isSignal) {
        if (!hist3D) return nullptr;

        // Calculate bin centers for geometric cuts
        double nbinsXCenter = hist3D->GetNbinsX()/2;
        double nbinsYCenter = hist3D->GetNbinsY()/2;
        double nbinsZCenter = hist3D->GetNbinsZ()/2;

        auto hist1D = std::make_unique<TH1D>(name, "Discr", 100, 9.0, 14.0);

        // Apply geometric cuts and fill histogram
        for (int i = hist3D->GetXaxis()->FindBin(-1000); 
             i < hist3D->GetXaxis()->FindBin(1000); i++) {
            for (int j = hist3D->GetYaxis()->FindBin(-1000); 
                 j < hist3D->GetYaxis()->FindBin(1000); j++) {
                for (int k = hist3D->GetZaxis()->FindBin(-500); 
                     k < hist3D->GetZaxis()->FindBin(500); k++) {
                    
                    // Complex geometric cut conditions
                    bool zCut = ((k-nbinsZCenter)*30 > -65) && 
                               ((k-nbinsZCenter)*30 <= 65);
                    bool yCut = ((j-nbinsYCenter)*30 > -126) && 
                               ((j-nbinsYCenter)*30 <= 126);
                    bool xCut = ((i-nbinsXCenter)*30 > 300) && 
                               ((i-nbinsXCenter)*30 <= 400);

                    if (zCut && yCut && xCut) {
                        hist1D->Fill(hist3D->GetBinContent(i,j,k));
                    }
                }
            }
        }

        // Configure histogram appearance
        hist1D->SetLineColor(isSignal ? kRed : kBlack);
        hist1D->GetYaxis()->SetTitle("Entries");
        hist1D->GetXaxis()->SetTitle("Median Metric");

        return hist1D;
    }

    // Calculate efficiency metrics
    void CalculateEfficiency(TH1D* signal, TH1D* background) {
        double integralSignal = signal->Integral();
        int nBins = signal->GetNbinsX();
        
        std::vector<double> efficiency(nBins);
        std::vector<double> purity(nBins);
        std::vector<double> errors(nBins);

        for (int i = 0; i < nBins; i++) {
            double sigTemp = signal->Integral(i+1, nBins);
            double bkgTemp = background->Integral(i+1, nBins);
            
            efficiency[i] = sigTemp / integralSignal;
            purity[i] = sigTemp / (sigTemp + bkgTemp);
            
            // Calculate errors using error propagation
            errors[i] = std::sqrt(efficiency[i] * (1-efficiency[i]) / integralSignal);
            
            std::cout << "Bin " << i << " - Efficiency: " << efficiency[i] 
                     << " Purity: " << purity[i] << std::endl;
        }
    }

public:
    void Analyze() {
        AnalysisConfig config;
        auto histograms = LoadHistograms(config);
        
        if (!histograms.signal || !histograms.background) {
            std::cerr << "Failed to load histograms" << std::endl;
            return;
        }

        // Process histograms
        auto signalHist = Process3DHistogram(histograms.signal.get(), "h_discr1", true);
        auto backgroundHist = Process3DHistogram(histograms.background.get(), "h_discr2", false);

        // Calculate efficiency metrics
        CalculateEfficiency(signalHist.get(), backgroundHist.get());

        // Create and save plots
        gStyle->SetOptStat(0);
        auto canvas = std::make_unique<TCanvas>("c_first", "Graph", 200, 50, 1200, 600);
        
        signalHist->Draw();
        signalHist->SetTitle("Median of metric distributions for voxels (30,40) "
                           "centre in hydrogen and voxels in bitumen (3cm Voxel)");
        backgroundHist->Draw("SAME");
        
        canvas->SaveAs("Overlay_MedianMetric_30cm_40cm_Centre_3cmVoxel.pdf", "pdf");
    }
};

void EffSlices() {
    EfficiencyAnalyzer analyzer;
    analyzer.Analyze();
}

int main(int argc, char** argv) {
    EffSlices();
    return 0;
}
