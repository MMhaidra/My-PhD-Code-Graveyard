/*
 * This macro analyzes discriminator distributions from ROOT files
 * and calculates their statistical properties. It handles both
 * single file analysis and comparison between multiple files.
 */

#include <iostream>
#include <memory>
#include <vector>

// ROOT histogram and analysis
#include "TH1D.h"
#include "TH3F.h"
#include "TTree.h"
#include "TSpectrum.h"
#include "TVirtualFitter.h"
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
#include "TGraph.h"
#include "TMultiGraph.h"
#include "TGraphErrors.h"

class DiscriminatorAnalyzer {
private:
    struct AnalysisConfig {
        // Histogram parameters
        static constexpr int nBins = 200;
        static constexpr double histMin = 0;
        static constexpr double histMax = 140;
        static constexpr double analysisRangeMin = 5.0;
        static constexpr double analysisRangeMax = 15.0;
        
        // Geometric cuts
        static constexpr double cylinderRadius = 300;
        static constexpr double xMin = -480;
        static constexpr double xMax = 480;
        
        // File paths
        const char* inputFile = "/home/mmhaidra/SliceMethod/Exposure_time_study/"
                               "largedrum_OnlyBitumen_STE3_dense_tracks40_3.discriminator.root";
    };

    // Process single histogram with geometric cuts
    std::unique_ptr<TH1D> ProcessHistogram(const char* filename, const char* histName) {
        // Open file and get tree
        std::unique_ptr<TFile> file(new TFile(filename, "READ"));
        if (!file || file->IsZombie()) {
            std::cerr << "Failed to open file: " << filename << std::endl;
            return nullptr;
        }

        TTree* tree = (TTree*)file->Get("T");
        if (!tree) return nullptr;

        // Create and fill histogram with geometric cuts
        auto hist = std::make_unique<TH1D>(histName, "Discriminator Distribution",
            AnalysisConfig::nBins, AnalysisConfig::histMin, AnalysisConfig::histMax);

        // Apply geometric cuts using cylindrical coordinates
        TString cuts = Form("((z*z)+(y*y)<%f)&&(x>%f)&&(x<%f)", 
            pow(AnalysisConfig::cylinderRadius, 2),
            AnalysisConfig::xMin, AnalysisConfig::xMax);
        
        tree->Draw(Form("discr>>%s", histName), cuts, "goff");
        
        // Important: Detach histogram from file to prevent memory issues
        hist->SetDirectory(0);
        
        return hist;
    }

public:
    void Analyze() {
        AnalysisConfig config;
        
        // Process main histogram
        auto histogram = ProcessHistogram(config.inputFile, "discr_hist1");
        if (!histogram) {
            std::cerr << "Failed to process histogram" << std::endl;
            return;
        }

        // Set analysis range and calculate statistics
        histogram->GetXaxis()->SetRangeUser(
            AnalysisConfig::analysisRangeMin,
            AnalysisConfig::analysisRangeMax);

        double mean = histogram->GetMean();
        double error = histogram->GetMeanError();

        // Print results
        std::cout << "Statistics Summary:\n"
                  << "Mean: " << mean << "\n"
                  << "Error: " << error << std::endl;
    }
};

void print_mean_discr() {
    DiscriminatorAnalyzer analyzer;
    analyzer.Analyze();
}

int main(int argc, char** argv) {
    print_mean_discr();
    return 0;
}

