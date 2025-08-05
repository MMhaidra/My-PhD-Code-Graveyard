/*
 * This macro analyzes and plots the relationship between discriminator values
 * and various cuts for STE3 bitumen filled drums. It handles both high Z
 * and medium Z material voxels analysis.
 */

#include <memory>
#include <vector>
#include <array>
// ROOT graphics includes
#include "TCanvas.h"
#include "TGraph.h"
#include "TGraphErrors.h"
#include "TLine.h"
#include "TPaveText.h"
#include "TLegend.h"
#include "TStyle.h"
// ROOT data handling includes
#include "TTree.h"
#include "TH1D.h"
#include "TFile.h"
#include "TCut.h"

class DiscrAnalyzer {
private:
    struct PlotConfig {
        int nPoints;
        std::vector<double> cuts;
        std::vector<double> discriminators;
        const char* title;
        const char* outputFile;
    };

    // Helper function to create and configure graph
    std::unique_ptr<TGraphErrors> CreateGraph(const PlotConfig& config) {
        // Create error arrays filled with zeros
        std::vector<double> errorCuts(config.nPoints, 0);
        std::vector<double> errorDiscr(config.nPoints, 0);

        // Create and configure graph
        auto graph = std::make_unique<TGraphErrors>(config.nPoints,
            config.discriminators.data(), config.cuts.data(),
            errorDiscr.data(), errorCuts.data());

        graph->SetTitle(config.title);
        graph->SetLineColorAlpha(kBlue, 0.35);
        graph->SetMarkerStyle(8);
        graph->SetMarkerSize(1.2);

        return graph;
    }

public:
    void Analyze() {
        // Configure plot parameters
        PlotConfig config{
            7,  // nPoints
            {22, 35, 36, 37, 35, 50, 72},  // cuts
            {1.4, 2, 2, 2, 2, 4, 8},       // discriminators
            "Entries against Hydrogen Volume (L); Volume in (L) ; Entries",
            "VolumeVsEntries.pdf"
        };

        // Create canvas
        auto canvas = std::make_unique<TCanvas>("c1", "Graph", 200, 50, 1200, 600);

        // Create and configure graph
        auto graph = CreateGraph(config);
        
        // Draw graph
        graph->Draw("A*");

        // Optional: Add reference line (commented out for now)
        /*
        auto line = std::make_unique<TLine>(7.63, 0, 9.56, 0);
        line->SetLineStyle(1);
        line->SetLineColor(33);
        line->SetLineWidth(1);
        line->Draw();
        */

        // Save plot
        canvas->SaveAs(config.outputFile, "pdf");
    }
};

void DiscrVsCut() {
    DiscrAnalyzer analyzer;
    analyzer.Analyze();
}

int main(int argc, char** argv) {
    DiscrVsCut();
    return 0;
}

