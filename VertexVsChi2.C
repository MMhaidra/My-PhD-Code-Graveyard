/* This macro analyzes the relationship between vertex position and chi-square values
 * for bubble experiments in vacuum conditions.
 * 
 * Memory Management Note:
 * ROOT objects should be properly managed to prevent memory leaks.
 * Using smart pointers and RAII principles for better resource management.
 */

#include <iostream>
#include <memory>

// ROOT data structures
#include "TFile.h"
#include "TTree.h"
#include "TLeaf.h"
#include "TH2D.h"

// ROOT graphics and style
#include "TCanvas.h"
#include "TStyle.h"
#include "TROOT.h"

class VertexAnalyzer {
private:
    struct AnalysisConfig {
        // Histogram parameters
        struct HistParams {
            int nBins = 10;
            double xMin = -1500;
            double xMax = 1500;
            double yMin = 0;
            double yMax = 1000;
        } histParams;
        
        // Input file path
        const char* inputFile = "/home/mmhaidra/Bubble_InVacuum.root";
        
        // Branch names
        const char* minuitBranch = "minuitparams";
        const char* scatterBranch = "fitscattering";
        const char* fminLeaf = "fmin";  // range 0-2500
        const char* vxLeaf = "vx";      // range -25000-25000
    };

    // Create and configure histogram
    std::unique_ptr<TH2D> CreateHistogram(const AnalysisConfig::HistParams& params) {
        auto hist = std::make_unique<TH2D>("h1", "vertexVSchi2",
            params.nBins, params.xMin, params.xMax,
            params.nBins, params.yMin, params.yMax);
            
        hist->GetXaxis()->SetTitle("Vertex.x");
        hist->GetYaxis()->SetTitle("Chi2");
        hist->SetTitle("Vertex_As_A_function_Of_Chi2_Bubble_In_Vacuum");
        hist->SetLineColor(kBlue+1);
        hist->SetLineWidth(1);
        hist->SetStats(0);
        
        return hist;
    }

public:
    void Analyze() {
        AnalysisConfig config;
        
        // Open input file
        std::unique_ptr<TFile> file(new TFile(config.inputFile, "READ"));
        if (!file || file->IsZombie()) {
            std::cerr << "Error opening input file" << std::endl;
            return;
        }

        // Get tree and configure branches
        auto tree = static_cast<TTree*>(file->Get("T"));
        if (!tree) return;

        tree->GetBranch(config.minuitBranch);
        tree->GetBranch(config.scatterBranch);
        
        auto fminLeaf = tree->GetLeaf(config.fminLeaf);
        auto vxLeaf = tree->GetLeaf(config.vxLeaf);
        
        // Create histogram
        auto hist = CreateHistogram(config.histParams);

        // Process all entries
        for (int i = 0; i < tree->GetEntries(); i++) {
            fminLeaf->GetBranch()->GetEntry(i);
            vxLeaf->GetBranch()->GetEntry(i);
            hist->Fill(vxLeaf->GetValue(), fminLeaf->GetValue());
        }

        // Configure and draw canvas
        auto canvas = std::make_unique<TCanvas>("can",
            "Vertex_As_A_function_Of_Chi2_Bubble_In_Vacuum", 900, 700);
            
        gROOT->SetStyle("Plain");
        gStyle->SetOptStat(0);

        // Configure axis titles
        hist->GetYaxis()->SetTitleSize(20);
        hist->GetYaxis()->SetTitleFont(43);
        hist->GetYaxis()->SetTitleOffset(1.55);

        // Draw and save
        hist->Draw("CONTZ");
        canvas->SaveAs("Vertex_AsAfunctionOf_Chi2_vx_Bubble_Bubble_In_Vacuum.pdf", "pdf");
    }
};

void VertexVsChi2() {
    VertexAnalyzer analyzer;
    analyzer.Analyze();
}

int main(int argc, char** argv) {
    VertexVsChi2();
    return 0;
}
