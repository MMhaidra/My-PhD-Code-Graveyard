//M.Mhaidra 2018//
//////////////////

#include <iostream>
#include <memory>
#include <cmath>
#include "TCanvas.h"
#include "TTree.h"
#include "TLeaf.h"
#include "TH1D.h"
#include "TFile.h"
#include "TStyle.h"
#include "TROOT.h"

class VertexAnalyzer {
private:
    struct HistogramConfig {
        const char* name;
        const char* title;
        int bins;
        double xMin;
        double xMax;
    };

    struct PlotSettings {
        int lineColor;
        int lineWidth;
        int titleSize;
        int titleFont;
        double titleOffset;
    };

    std::unique_ptr<TH1D> CreateHistogram(const HistogramConfig& config) {
        auto hist = std::make_unique<TH1D>(config.name, config.title, 
                                         config.bins, config.xMin, config.xMax);
        hist->GetXaxis()->SetTitle("VertexPerVoxel");
        hist->GetYaxis()->SetTitle("Entries");
        return hist;
    }

    void ProcessTree(TTree* tree, TH1D* hist) {
        if (!tree || !hist) return;
        
        TLeaf* leaf = tree->GetLeaf("tracks");
        if (!leaf) return;

        for (int i = 0; i < tree->GetEntries(); i++) {
            leaf->GetBranch()->GetEntry(i);
            hist->Fill(leaf->GetValue());
        }
    }

    void ConfigureHistogram(TH1D* hist, const PlotSettings& settings) {
        hist->SetLineColor(settings.lineColor);
        hist->SetLineWidth(settings.lineWidth);
        hist->GetYaxis()->SetTitleSize(settings.titleSize);
        hist->GetYaxis()->SetTitleFont(settings.titleFont);
        hist->GetYaxis()->SetTitleOffset(settings.titleOffset);
        hist->SetStats(0);
    }

public:
    void Analyze() {
        // Open input files
        std::unique_ptr<TFile> file1(new TFile("/Users/mhaidra/Desktop/onlyHydrogen.discriminator.root"));
        std::unique_ptr<TFile> file2(new TFile("/Users/mhaidra/Desktop/onlyConcrete.discriminator.root"));

        if (!file1 || !file2) {
            std::cerr << "Error opening input files" << std::endl;
            return;
        }

        // Get trees
        TTree* tree1 = (TTree*)file1->Get("T");
        TTree* tree2 = (TTree*)file2->Get("T");

        if (!tree1 || !tree2) {
            std::cerr << "Error accessing trees" << std::endl;
            return;
        }

        // Create histogram
        HistogramConfig config = {
            "h1", "hist1", 50, 0, 140
        };
        auto h1 = CreateHistogram(config);

        // Process data
        ProcessTree(tree1, h1.get());

        // Configure plot settings
        PlotSettings settings = {
            kBlue+1,  // lineColor
            1,        // lineWidth
            20,       // titleSize
            43,       // titleFont
            1.55      // titleOffset
        };
        ConfigureHistogram(h1.get(), settings);

        // Create and configure canvas
        std::unique_ptr<TCanvas> can(new TCanvas("can",
            "Normalized_VertexPerVoxel_HydrogenVsConcrete", 900, 700));
        
        gROOT->SetStyle("Plain");
        gStyle->SetOptStat(0);
        gPad->SetLogy(1);

        // Draw and save
        h1->SetTitle("Normalized_VertexPerVoxel_HydrogenVsConcrete");
        h1->Draw();

        can->SaveAs("Normalized_VertexPerVoxel_HydrogenVsConcrete.pdf", "pdf");
        can->SaveAs("Normalized_VertexPerVoxel_HydrogenVsConcrete.root", "root");
    }
};

void VertexPerVoxel() {
    VertexAnalyzer analyzer;
    analyzer.Analyze();
}

int main(int argc, char** argv) {
    VertexPerVoxel();
    return 0;
}
