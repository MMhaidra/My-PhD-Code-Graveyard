/* This macro calculates and plots the means of Bristol discriminator distributions 
 * in the voxels inside the drum.
 *
 * Memory Management Note:
 * ROOT histograms are associated with a TDirectory upon creation (usually the open file).
 * When directory changes (new file opened), histogram pointers become invalid.
 * Using TH1::SetDirectory(0) detaches histogram from file for safer handling.
 */

#include <iostream>
#include <memory>
#include <vector>

// ROOT core classes
#include "TFile.h"
#include "TTree.h"
#include "TCut.h"
#include "TString.h"

// ROOT histograms
#include "TH1D.h"
#include "TH3D.h"
#include "TSpectrum.h"

// ROOT graphics and utilities
#include "TCanvas.h"
#include "TStyle.h"
#include "TLine.h"
#include "TPaveText.h"
#include "TLegend.h"
#include "TFrame.h"

class DrumAnalyzer {
private:
    struct AnalysisConfig {
        // Geometric parameters (in mm)
        struct Range {
            double start;
            double end;
            double width;
        };
        
        Range x{-480, 480, 20};
        Range y{-300, 300, 20};
        Range z{-300, 300, 20};
        
        // Analysis parameters
        static constexpr double discrMin = 7.0;
        static constexpr double discrMax = 14.0;
        
        // File paths
        const char* inputFile = "/home/mmhaidra/SliceMethod/Chance_largedrum_bub_tracks35_discr/"
                               "largedrum_12.92lBubble_tracks10.discriminator.root";
        const char* outputFile = "MeanDiscr_3Dhist_12L_10tracks_20Steps.root";
    };

    // Validate slice parameters
    bool ValidateRange(const AnalysisConfig::Range& range, const char* axis) {
        if ((static_cast<int>(range.end - range.start) % static_cast<int>(range.width)) != 0) {
            std::cout << "Invalid " << axis << " slice width. Current: " << range.width << std::endl;
            return false;
        }
        return true;
    }

    // Process single voxel with proper memory management
    double ProcessVoxel(TTree* tree, const TCut& cuts) {
        // Create histogram detached from file
        auto hist = std::make_unique<TH1D>("discr_hist", "Discriminator Distribution", 200, 0, 140);
        hist->SetDirectory(nullptr);  // Important: Detach from current directory
        
        // Fill and analyze histogram
        tree->Draw("discr>>discr_hist", cuts, "goff");
        hist->GetXaxis()->SetRangeUser(AnalysisConfig::discrMin, AnalysisConfig::discrMax);
        
        return hist->GetMean();
    }

public:
    void Analyze() {
        AnalysisConfig config;
        
        // Validate all ranges
        if (!ValidateRange(config.x, "X") || 
            !ValidateRange(config.y, "Y") || 
            !ValidateRange(config.z, "Z")) {
            return;
        }

        // Create 3D histogram
        auto hist3D = std::make_unique<TH3D>("hist_3D", "Discriminator Mean Distribution",
            20, config.x.start, config.x.end,
            20, config.y.start, config.y.end,
            20, config.z.start, config.z.end);

        // Open input file
        std::unique_ptr<TFile> inFile(new TFile(config.inputFile, "READ"));
        if (!inFile || inFile->IsZombie()) {
            std::cerr << "Error opening input file" << std::endl;
            return;
        }

        auto tree = (TTree*)inFile->Get("T");
        if (!tree) return;

        // Process all voxels
        for (double x = config.x.start; x < config.x.end; x += config.x.width) {
            std::cout << "Processing X slice: " << x << " to " << x + config.x.width << std::endl;
            
            for (double y = config.y.start; y < config.y.end; y += config.y.width) {
                for (double z = config.z.start; z < config.z.end; z += config.z.width) {
                    // Create combined cuts for current voxel
                    TCut cuts = TString::Format(
                        "x>%f && x<%f && y>%f && y<%f && z>%f && z<%f",
                        x, x + config.x.width,
                        y, y + config.y.width,
                        z, z + config.z.width
                    );

                    // Process voxel and fill histogram
                    double mean = ProcessVoxel(tree, cuts);
                    hist3D->SetBinContent(hist3D->FindBin(x, y, z), mean);
                }
            }
        }

        // Save results
        std::unique_ptr<TFile> outFile(new TFile(config.outputFile, "UPDATE"));
        hist3D->Write();
    }
};

void Slices_3D() {
    DrumAnalyzer analyzer;
    analyzer.Analyze();
}

int main(int argc, char** argv) {
    Slices_3D();
    return 0;
}
