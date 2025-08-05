// M.Mhaidra 2020/11/30
// This macro analyzes efficiency metrics for different voxel configurations 
// in STE3 bitumen drums with hydrogen bubbles

#include <iostream>
#include <memory>
#include <vector>
#include <fstream>
#include <cmath>
// ROOT includes grouped by functionality
#include "TCanvas.h"
#include "TH1D.h"
#include "TH3F.h"
#include "TFile.h"
#include "TGraphErrors.h"
#include "TMultiGraph.h"
#include "TStyle.h"

class EfficiencyAnalyzer {
private:
    struct AnalysisConfig {
        // Histogram parameters
        double medianMin = 10.5;
        double medianMax = 12.5;
        int nBins = 100;
        
        // Geometric cuts in mm
        double dX = 30;  // 30mm voxel size
        double dY = 30;
        double dZ = 30;
        double cylinderRadius = 240;
        double xRange = 1000;
        double zRange = 500;
        
        // File paths
        const char* signalFile = "/home/mmhaidra/SliceMethod/largedrum_onlyhydrogen_dense_newmetrics_3cmVoxel_April2021.discriminator.root";
        const char* backgroundFile = "/home/mmhaidra/SliceMethod/largedrum_onlybitumen_dense_newmetrics_3cmVoxel_April2021.discriminator.root";
    };

    struct HistogramPair {
        std::unique_ptr<TH3F> signal;
        std::unique_ptr<TH3F> background;
    };

    // Load histograms with proper memory management
    HistogramPair LoadHistograms(const AnalysisConfig& config) {
        auto file1 = std::make_unique<TFile>(config.signalFile, "READ");
        auto file2 = std::make_unique<TFile>(config.backgroundFile, "READ");
        
        HistogramPair result;
        if (file1 && file2) {
            // Important: Clone and detach histograms from files
            result.signal.reset((TH3F*)file1->Get("histMedianMetric"));
            result.background.reset((TH3F*)file2->Get("histMedianMetric"));
            
            if (result.signal) result.signal->SetDirectory(0);
            if (result.background) result.background->SetDirectory(0);
        }
        return result;
    }

    // Process 3D histogram with geometric cuts
    std::unique_ptr<TH1D> Process3DHistogram(TH3F* hist3D, const char* name, 
                                           const AnalysisConfig& config, bool isSignal) {
        if (!hist3D) return nullptr;

        auto hist1D = std::make_unique<TH1D>(name, "Discr", config.nBins, 
                                            config.medianMin, config.medianMax);
        
        // Calculate bin centers for geometric cuts
        double nbinsXCenter = hist3D->GetNbinsX()/2;
        double nbinsYCenter = hist3D->GetNbinsY()/2;
        double nbinsZCenter = hist3D->GetNbinsZ()/2;

        // Iterate through bins with geometric cuts
        for (int i = hist3D->GetXaxis()->FindBin(-config.xRange); 
             i < hist3D->GetXaxis()->FindBin(config.xRange); i++) {
            for (int j = hist3D->GetYaxis()->FindBin(-config.xRange); 
                 j < hist3D->GetYaxis()->FindBin(config.xRange); j++) {
                for (int k = hist3D->GetZaxis()->FindBin(-config.zRange); 
                     k < hist3D->GetZaxis()->FindBin(config.zRange); k++) {
                    
                    // Complex geometric cut condition
                    bool isInCylinder = pow(((j-nbinsYCenter)*config.dY), 2.0) + 
                                      pow(((k-nbinsZCenter)*config.dZ), 2.0) < 
                                      pow(config.cylinderRadius, 2.0);
                    
                    bool isInXRange = ((i-nbinsXCenter)*config.dX > 300) && 
                                    ((i-nbinsXCenter)*config.dX <= 400);
                    
                    bool isInZExclusion = !(((k-nbinsZCenter)*config.dZ > -240 && 
                                           (k-nbinsZCenter)*config.dZ <= -100) || 
                                          ((k-nbinsZCenter)*config.dZ > 100 && 
                                           (k-nbinsZCenter)*config.dZ <= 240));

                    if (isInCylinder && isInXRange && isInZExclusion) {
                        hist1D->Fill(hist3D->GetBinContent(i,j,k));
                    }
                }
            }
        }
        return hist1D;
    }

    // Calculate efficiency and purity metrics
    std::pair<std::unique_ptr<TGraphErrors>, std::unique_ptr<TGraphErrors>> 
    CalculateMetrics(TH1D* signalHist, TH1D* backgroundHist) {
        double integralSignal = signalHist->Integral();
        int nBins = signalHist->GetNbinsX();
        
        auto graphEff = std::make_unique<TGraphErrors>(nBins);
        auto graphPurity = std::make_unique<TGraphErrors>(nBins);

        for (int i = 0; i < nBins; i++) {
            double integralTemp = signalHist->Integral(i+1, nBins);
            double efficiency = integralTemp/integralSignal;
            double effError = sqrt(efficiency*(1-efficiency)/integralSignal);
            
            graphEff->SetPoint(i, signalHist->GetBinCenter(i+1), efficiency);
            graphEff->SetPointError(i, 0, effError);
        }

        // Configure graph appearance
        graphEff->SetLineColor(kRed);
        graphPurity->SetLineColor(kBlack);
        
        return {std::move(graphEff), std::move(graphPurity)};
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
        auto h_signal = Process3DHistogram(histograms.signal.get(), "h_discr1", config, true);
        auto h_background = Process3DHistogram(histograms.background.get(), "h_discr2", config, false);

        // Calculate metrics
        auto [graphEff, graphPurity] = CalculateMetrics(h_signal.get(), h_background.get());

        // Create and configure canvas
        gStyle->SetOptStat(0);
        auto canvas = std::make_unique<TCanvas>("cAll1", "Graph", 200, 100, 1300, 700);
        
        auto mg = std::make_unique<TMultiGraph>();
        mg->Add(graphEff.get());
        mg->Add(graphPurity.get());
        
        mg->SetTitle("Efficiency for signal and purity as a function of the Median Metric "
                    "(3cm voxel)(30cm,40cm) Central Slice;Median Metric;");
        
        mg->Draw("APC");
        canvas->Update();
        
        canvas->SaveAs("/home/mmhaidra/SliceMethod/results_Roc_Slices_May2021/"
                      "Efficiency_Purity_Overlays_MedianMeanMetric_3cmVoxel_withErrors_30cm_40cm_Central.pdf");
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
