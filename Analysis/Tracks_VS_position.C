/* This macro plots the Muon tracks as a function of detector position
 *
 * Memory Management Note:
 * ROOT histograms are associated with a TDirectory upon creation (usually open file).
 * When directory changes (new file opened), histogram pointers become invalid.
 * Using TH3::SetDirectory(0) detaches histogram from file for safer handling.
 */

#include <algorithm>
#include <iterator>
#include <vector>
#include <memory>
#include <iomanip>
#include <cmath>

// ROOT histograms
#include "TH1D.h"
#include "TH2D.h"
#include "TH3D.h"
#include "TH3F.h"
// ROOT graphics
#include "TCanvas.h"
#include "TStyle.h"
#include "TLine.h"
#include "TPaveText.h"
#include "TLegend.h"
#include "TFrame.h"
// ROOT analysis
#include "TFile.h"
#include "TTree.h"
#include "TCut.h"
#include "TF1.h"
#include "TSpectrum.h"
#include "TVirtualFitter.h"
// ROOT utilities
#include "TGraph.h"
#include "TMultiGraph.h"
#include "TGraphErrors.h"
#include "TRandom.h"
#include "TRandom3.h"

class TrackAnalyzer {
private:
    struct AnalysisConfig {
        // Histogram parameters
        int nBins = 200;
        double histMin = -20;
        double histMax = 20;
        
        // File paths
        const char* inputFile = "/home/mmhaidra/SliceMethod/Chance_largedrum_bub_tracks35_discr_STE3/"
                               "largedrum_bub_STE3_OnlyBitumen_ForVoxelsCleaning_tracks35.discriminator.root";
    };

    // Process 3D histogram with proper memory management
    std::unique_ptr<TH1D> ProcessHistogram(TH3F* hist3D) {
        // Create result histogram
        auto hist = std::make_unique<TH1D>("h_discr", "Discriminator Distribution",
            AnalysisConfig::nBins, AnalysisConfig::histMin, AnalysisConfig::histMax);
            
        // Iterate through all bins
        for (int i = 1; i <= hist3D->GetNbinsX(); i++) {
            for (int j = 1; j <= hist3D->GetNbinsY(); j++) {
                for (int k = 1; k <= hist3D->GetNbinsZ(); k++) {
                    hist->Fill(hist3D->GetBinContent(i,j,k));
                }
            }
        }
        
        return hist;
    }

public:
    void Analyze() {
        AnalysisConfig config;
        
        // Open input file and get histogram
        std::unique_ptr<TFile> inputFile(new TFile(config.inputFile, "READ"));
        if (!inputFile || inputFile->IsZombie()) {
            std::cerr << "Error opening input file" << std::endl;
            return;
        }

        auto hist3D = static_cast<TH3F*>(inputFile->Get("histNS"));
        if (!hist3D) {
            std::cerr << "Error loading 3D histogram" << std::endl;
            return;
        }

        // Important: Detach from file directory
        hist3D->SetDirectory(nullptr);
        inputFile->Close();

        // Process histogram
        auto resultHist = ProcessHistogram(hist3D);
        
        // Configure histogram appearance
        resultHist->SetMarkerStyle(21);
        resultHist->SetMarkerColor(kBlack+8);
        resultHist->SetFillColor(1);
        resultHist->GetYaxis()->SetTitle("Entries");
        resultHist->GetXaxis()->SetTitle("BC Discriminator");

        // Create canvas and draw
        auto canvas = std::make_unique<TCanvas>("discr_Iron", "Graph", 200, 100, 1300, 700);
        resultHist->Draw("HIST");
        
        canvas->SaveAs("Tracks_per_position.pdf", "pdf");
    }
};

void Tracks_VS_position() {
    TrackAnalyzer analyzer;
    analyzer.Analyze();
}

int main(int argc, char** argv) {
    Tracks_VS_position();
    return 0;
}
