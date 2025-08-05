/* This macro extracts discriminator histograms from two discriminator.root files and plots
 * them on the same axes. It performs two types of analysis:
 * 1. Direct histogram comparison with loop-based processing
 * 2. Tree-based discriminator comparison with geometric cuts
 */

#include <memory>
#include <vector>
#include <string>

// RooFit includes
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
#include "TLine.h"
#include "TPaveText.h"
#include "TLegend.h"
#include "TStyle.h"
#include "TFrame.h"

// ROOT data handling
#include "TTree.h"
#include "TH1D.h"
#include "TH3F.h"
#include "TFile.h"
#include "TSpectrum.h"

class DiscriminatorAnalyzer {
private:
    struct AnalysisConfig {
        // Time configuration
        static constexpr int nTimePoints = 7;
        const std::vector<double> timePoints = {3, 6, 10, 15, 20, 25, 30};
        
        // Histogram parameters
        const int nBins = 500;
        const double histMin = -4;
        const double histMax = 4;
        
        // File paths
        const char* basePath = "/home/mmhaidra/SliceMethod/Exposure_time_study/";
        const char* signalPrefix = "largedrum_bub_4.42l_STE3_dense_tracks5_";
        const char* bkgPrefix = "largedrum_OnlyBitumen_STE3_dense_tracks5_";
        const char* suffix = ".discriminator.root";
    };

    // Helper function to load 3D histogram from file
    std::unique_ptr<TH3F> LoadHistogram(const char* filename) {
        std::unique_ptr<TFile> file(new TFile(filename, "READ"));
        if (!file || file->IsZombie()) return nullptr;

        TH3F* hist = (TH3F*)file->Get("histBS");
        if (hist) hist->SetDirectory(0); // Important: Detach from file
        return std::unique_ptr<TH3F>(hist);
    }

    // Process histograms with geometric cuts
    std::unique_ptr<TH1D> ProcessDiscriminatorHistogram(TTree* tree, const char* name) {
        auto hist = std::make_unique<TH1D>(name, "hist", 500, 5, 12);
        
        // Apply geometric cuts and fill histogram
        tree->Draw(Form("discr>>%s", name),
                  "(z*z)+(y*y)<(300*300) && x>-440 && x<440", 
                  "goff");
        
        // Configure histogram
        hist->SetLineWidth(1);
        hist->GetXaxis()->SetRangeUser(5, 17);
        hist->GetYaxis()->SetTitleSize(0.05);
        hist->GetXaxis()->SetTitleSize(0.05);
        hist->GetYaxis()->SetTitleOffset(0.70);
        hist->GetXaxis()->SetTitleOffset(0.80);
        hist->Rebin(4);
        
        return hist;
    }

public:
    void Analyze() {
        AnalysisConfig config;
        
        // Part 1: Loop-based histogram comparison
        auto signal3D = LoadHistogram(Form("%s%s%d%s", 
            config.basePath, config.signalPrefix, 7, config.suffix));
        auto bkg3D = LoadHistogram(Form("%s%s%d%s", 
            config.basePath, config.bkgPrefix, 7, config.suffix));

        if (!signal3D || !bkg3D) {
            std::cerr << "Failed to load 3D histograms" << std::endl;
            return;
        }

        // Create difference histograms
        auto h_signal = std::make_unique<TH1D>("h_diff1", "Signal", 
                                              config.nBins, config.histMin, config.histMax);
        auto h_background = std::make_unique<TH1D>("h_diff2", "Background", 
                                                  config.nBins, config.histMin, config.histMax);

        // Process histograms
        for (int i = signal3D->GetXaxis()->FindBin(-440); 
             i < signal3D->GetXaxis()->FindBin(440); i++) {
            for (int j = signal3D->GetYaxis()->FindBin(-300); 
                 j < signal3D->GetYaxis()->FindBin(300); j++) {
                for (int k = signal3D->GetZaxis()->FindBin(-300); 
                     k < signal3D->GetZaxis()->FindBin(300); k++) {
                    
                    if (signal3D->GetBinContent(i,j,k) == 0 || 
                        bkg3D->GetBinContent(i,j,k) == 0) continue;
                    
                    double diff = signal3D->GetBinContent(i,j,k) - 
                                bkg3D->GetBinContent(i,j,k);
                    h_signal->Fill(diff);
                }
            }
        }

        // Create and configure first plot
        gStyle->SetOptStat(0);
        auto canvas1 = std::make_unique<TCanvas>("c1", "Loop Analysis", 200, 100, 1300, 700);
        
        h_signal->SetTitle("");
        h_signal->GetYaxis()->SetTitle("Entries");
        h_signal->GetXaxis()->SetTitle("BC Discriminator");
        h_signal->SetLineColor(kBlack);
        h_signal->Draw();
        
        auto legend1 = std::make_unique<TLegend>(0.7, 0.7, 0.9, 0.9);
        legend1->AddEntry(h_signal.get(), "4L", "L");
        legend1->Draw();
        
        canvas1->SaveAs("4LvsBitumen_withloop.pdf");

        // Part 2: Tree-based analysis
        auto canvas2 = std::make_unique<TCanvas>("c2", "Tree Analysis", 200, 100, 1300, 700);
        
        // Process signal and background
        std::unique_ptr<TFile> signalFile(new TFile(Form("%s%s%d%s", 
            config.basePath, config.signalPrefix, 7, config.suffix), "READ"));
        std::unique_ptr<TFile> bkgFile(new TFile(Form("%s%s%d%s", 
            config.basePath, config.bkgPrefix, 7, config.suffix), "READ"));

        auto signalHist = ProcessDiscriminatorHistogram(
            (TTree*)signalFile->Get("T"), "discr_hist_1");
        auto bkgHist = ProcessDiscriminatorHistogram(
            (TTree*)bkgFile->Get("T"), "discr_hist_2");

        signalHist->SetLineColor(kBlack);
        bkgHist->SetLineColor(kRed);
        
        signalHist->Draw();
        bkgHist->Draw("SAME");
        
        auto legend2 = std::make_unique<TLegend>(0.7, 0.7, 0.9, 0.9);
        legend2->AddEntry(signalHist.get(), "4L", "L");
        legend2->AddEntry(bkgHist.get(), "Bitumen", "L");
        legend2->Draw();
        
        canvas2->SaveAs("4LvsBitumen_withoutloop.pdf");
    }
};

void plot_discr_hists() {
    DiscriminatorAnalyzer analyzer;
    analyzer.Analyze();
}

int main(int argc, char** argv) {
    plot_discr_hists();
    return 0;
}
