// M.Mhaidra 13/09/2021
// This macro analyzes 3D histogram data with median cuts and neighbor counting
// for hydrogen bubble detection in bitumen drums

#include <iostream>
#include <vector>
#include <memory>
#include <cmath>

// ROOT histogram and analysis
#include "TH1D.h"
#include "TH2D.h"
#include "TH3D.h"
#include "TH3F.h"
#include "TTree.h"
#include "TSpectrum.h"
// ROOT graphics and style
#include "TCanvas.h"
#include "TStyle.h"
#include "TLine.h"
#include "TPaveText.h"
#include "TLegend.h"
#include "TFrame.h"
// ROOT utilities
#include "TFile.h"
#include "TCut.h"
#include "TF1.h"
#include "TVirtualFitter.h"

class MedianCutAnalyzer {
private:
    struct AnalysisConfig {
        // Geometric parameters (in mm)
        double dX = 30;  // 30mm voxel size
        double dY = 30;
        double dZ = 30;
        double cylinderRadius = 240;
        double medianCut = 11.17;  // Local threshold
        
        // Histogram parameters
        int nBins = 100;
        double histMin = 9.5;
        double histMax = 15.0;
        
        // File paths
        const char* inputFile = "/home/mmhaidra/SliceMethod/largedrum_0.7L_6Cubes_dense_Aligned_3cmVoxel_May2021.discriminator.root";
    };

    // Helper function to create and configure 1D histogram
    std::unique_ptr<TH1D> CreateHistogram(const char* name, const char* title) {
        auto hist = std::make_unique<TH1D>(name, title, 
                                         config.nBins, config.histMin, config.histMax);
        hist->GetXaxis()->SetTitle("Median Metric");
        hist->GetYaxis()->SetTitle("Entries");
        return hist;
    }

    // Process neighbor counting for a voxel
    int CountNeighbors(TH3F* hist, int i, int j, int k, bool aboveCut) {
        int count = 0;
        for (int a = -1; a <= 1; a++) {
            for (int b = -1; b <= 1; b++) {
                for (int c = -1; c <= 1; c++) {
                    double value = hist->GetBinContent(i+a, j+b, k+c);
                    if (aboveCut ? value >= config.medianCut : value < config.medianCut) {
                        count++;
                    }
                }
            }
        }
        return count;
    }

    // Check if point is within analysis region
    bool IsInAnalysisRegion(double x, double y, double z) {
        bool inCylinder = (pow(y, 2.0) + pow(z, 2.0)) < pow(config.cylinderRadius, 2.0);
        bool inXRange = (x > -400 && x <= -300);
        bool notInZExclusion = !(
            (z > -240 && z <= -100) || 
            (z > 100 && z <= 240)
        );
        return inCylinder && inXRange && notInZExclusion;
    }

    AnalysisConfig config;

public:
    void Analyze() {
        // Load input file and histogram
        auto file = std::make_unique<TFile>(config.inputFile, "READ");
        if (!file || file->IsZombie()) {
            std::cerr << "Error opening file: " << config.inputFile << std::endl;
            return;
        }

        auto hist3D = std::unique_ptr<TH3F>((TH3F*)file->Get("histMedianMetric"));
        if (!hist3D) {
            std::cerr << "Error loading histogram" << std::endl;
            return;
        }
        hist3D->SetDirectory(0);  // Detach from file
        file->Close();

        // Create analysis histograms
        auto histMedianMetric = CreateHistogram("h_histMedianMetric", "histMedianMetric");
        auto histNoCut = CreateHistogram("h_histMedianMetric_noCut", "histMedianMetric_noCut");
        auto histNeighbors = std::make_unique<TH1D>("h_NeighbourVoxelCount", 
            "Neighbour voxels count for 0.7L bubble", 27, -0.5, 26.5);
        auto histNeighborsBelow = std::make_unique<TH1D>("h_NeighbourVoxelCountBelow",
            "Neighbour voxels count below cut", 27, -0.5, 26.5);

        // Calculate bin centers
        double nbinsXCenter = hist3D->GetNbinsX()/2;
        double nbinsYCenter = hist3D->GetNbinsY()/2;
        double nbinsZCenter = hist3D->GetNbinsZ()/2;

        // Process histogram
        for (int i = hist3D->GetXaxis()->FindBin(-1000); 
             i < hist3D->GetXaxis()->FindBin(1000); i++) {
            for (int j = hist3D->GetYaxis()->FindBin(-1000); 
                 j < hist3D->GetYaxis()->FindBin(1000); j++) {
                for (int k = hist3D->GetZaxis()->FindBin(-500); 
                     k < hist3D->GetZaxis()->FindBin(500); k++) {
                    
                    double x = (i-nbinsXCenter) * config.dX;
                    double y = (j-nbinsYCenter) * config.dY;
                    double z = (k-nbinsZCenter) * config.dZ;

                    if (!IsInAnalysisRegion(x, y, z)) continue;

                    double value = hist3D->GetBinContent(i,j,k);
                    histNoCut->Fill(value);

                    if (value >= config.medianCut) {
                        histMedianMetric->Fill(value);
                        histNeighbors->Fill(CountNeighbors(hist3D.get(), i, j, k, true));
                    } else {
                        histNeighborsBelow->Fill(CountNeighbors(hist3D.get(), i, j, k, false));
                    }
                }
            }
        }

        // Create plot
        gStyle->SetOptStat(0);
        auto canvas = std::make_unique<TCanvas>("c_test2", 
            "Neighbour voxels count analysis", 200, 100, 1400, 700);
        
        histNeighborsBelow->SetLineColor(kBlack);
        histNeighbors->SetLineColor(kRed);
        
        histNeighborsBelow->Draw();
        histNeighbors->Draw("same");
        
        canvas->SaveAs("/home/mmhaidra/SliceMethod/results_Roc_Slices_May2021/"
                      "1_NeighbourVoxelCount_0.7L(6Cubes)_STE3_MedianCut_40_30.pdf");
    }
};

void medianCut3D() {
    MedianCutAnalyzer analyzer;
    analyzer.Analyze();
}

int main(int argc, char** argv) {
    medianCut3D();
    return 0;
}
