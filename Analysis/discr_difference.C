/*
 * This macro analyzes the difference in discriminator distributions
 * between hydrogen gas volumes and concrete in a large drum.
 * It processes multiple files and creates a graph showing the 
 * relationship between gas volume and discriminator differences.
 */

#include <memory>
#include <vector>
#include <iostream>
// ROOT RooFit includes
#include "RooRealVar.h"
#include "RooDataSet.h"
#include "RooDataHist.h"
#include "RooGaussian.h"
#include "RooCBShape.h"
#include "RooHistPdf.h"
#include "RooKeysPdf.h"
#include "RooExponential.h"
#include "RooChebychev.h"
#include "RooAddPdf.h"
// ROOT graphics includes
#include "TCanvas.h"
#include "RooPlot.h"
#include "RooHist.h"
#include "TLine.h"
#include "TPaveText.h"
#include "TLegend.h"
#include "TStyle.h"
#include "TFrame.h"
// ROOT data handling includes
#include "TTree.h"
#include "TH1D.h"
#include "TFile.h"
#include "TGraph.h"
#include "TMultiGraph.h"
#include "TGraphErrors.h"
#include "TF1.h"

class DiscriminatorAnalyzer {
private:
    struct AnalysisConfig {
        static constexpr int nPoints = 10;
        const std::vector<double> volumes = {
            1.178, 2.001, 2.925, 4.423, 6.032, 
            7.125, 9.425, 12.925, 17.191, 20.701
        };
        const char* baseDir = "/home/mmhaidra/SliceMethod/Chance_largedrum_bub_tracks15_discr/";
        const char* refFile = "largedrum_onlyconcrete_tracks15.discriminator.root";
    };

    struct HistogramResult {
        double mean;
        double error;
    };

    // Processes a single histogram from file
    HistogramResult ProcessHistogram(TTree* tree) {
        if (!tree) return {0.0, 0.0};

        auto hist = std::make_unique<TH1D>("discr_hist", "hist", 200, 0, 140);
        
        // Apply geometric cuts and fill histogram
        tree->Draw("discr>>discr_hist", 
                  "((z*z)+(y*y)<(300*300))&&(x>-480)&&(x<480)", "goff");
        
        hist->SetDirectory(0);  // Detach histogram from file
        hist->GetXaxis()->SetRangeUser(5.0, 15.0);

        return {hist->GetMean(), hist->GetMeanError()};
    }

public:
    void Analyze() {
        AnalysisConfig config;
        std::vector<double> discrMean(config.nPoints);
        std::vector<double> errorMean(config.nPoints);
        std::vector<double> errorVol(config.nPoints, 0);

        // Process each volume point
        for (int i = 0; i < config.nPoints; ++i) {
            // Construct file path for current volume
            TString currentFile = TString::Format("%slargedrum_bub_%d_tracks15.discriminator.root", 
                                                config.baseDir, i);
            
            // Process main file
            std::unique_ptr<TFile> file1(new TFile(currentFile, "READ"));
            auto result1 = ProcessHistogram((TTree*)file1->Get("T"));
            
            // Process reference file
            std::unique_ptr<TFile> file2(new TFile(TString::Format("%s%s", 
                                        config.baseDir, config.refFile), "READ"));
            auto result2 = ProcessHistogram((TTree*)file2->Get("T"));
            
            // Calculate differences
            discrMean[i] = result1.mean - result2.mean;
            errorMean[i] = result1.error + result2.error;
        }

        // Create and configure graph
        auto graph = std::make_unique<TGraphErrors>(config.nPoints, 
                                                  config.volumes.data(), 
                                                  discrMean.data(),
                                                  errorVol.data(), 
                                                  errorMean.data());
        
        // Configure and perform fit
        auto fit = std::make_unique<TF1>("f2", "[0]*x+[1]", 1e-6, 9.1);
        fit->SetLineColor(kMagenta+2);
        fit->SetLineWidth(3);
        
        // Create canvas and draw
        auto canvas = std::make_unique<TCanvas>("c1", "Graph", 200, 50, 1200, 600);
        
        graph->SetTitle("Difference of #mu_{discriminator} against gas volume for large size drum;"
                       "Hydrogen Gas volume (L);#mu_{discriminator} difference");
        graph->SetMarkerStyle(8);
        graph->SetMarkerSize(1.2);
        
        graph->Fit(fit.get());
        graph->Draw("A*");
        
        canvas->Update();
        canvas->Modified();
        canvas->SaveAs("difference_Mean_discr_against_gas_volume_Hydrogen_(L)_LargeDrum.pdf");
    }
};

void discr_difference() {
    DiscriminatorAnalyzer analyzer;
    analyzer.Analyze();
}

int main(int argc, char** argv) {
    discr_difference();
    return 0;
}

