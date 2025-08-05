/* This macro calculates and plots the means of the discriminator distributions in slices of the
 * drum in the YZ plane.
 *
 * Memory Management Note:
 * ROOT objects should be properly managed to prevent memory leaks.
 * Using smart pointers and RAII principles for better resource management.
 */

#include <iostream>
#include <memory>
#include <vector>

// ROOT data handling
#include "TFile.h"
#include "TTree.h"
#include "TCut.h"
#include "TH1D.h"
#include "TRandom.h"

// ROOT graphics and visualization
#include "TCanvas.h"
#include "TGraph.h"
#include "TMultiGraph.h"
#include "TGraphErrors.h"
#include "TLine.h"
#include "TPaveText.h"
#include "TLegend.h"
#include "TStyle.h"
#include "TFrame.h"

class DrumSliceAnalyzer {
private:
    struct AnalysisConfig {
        // Geometric parameters (mm)
        double y_start = -300;
        double y_finish = 300;
        double slice_width = 40;
        
        // Analysis parameters
        static constexpr double discrMin = 7.0;
        static constexpr double discrMax = 14.0;
        
        // File paths
        const char* signal_file = "/home/mmhaidra/SliceMethod/Chance_largedrum_bub_tracks15_discr/"
                                 "largedrum_9.42lBubble_Ucube_4cm_tracks15.discriminator.root";
        const char* background_file = "/home/mmhaidra/SliceMethod/Chance_largedrum_bub_tracks15_discr/"
                                    "largedrum_onlyconcrete_tracks15.discriminator.root";
    };

    // Process single slice with proper memory management
    std::pair<double, double> ProcessSlice(TTree* tree, const TCut& cuts, const char* histName) {
        // Create histogram detached from file
        auto hist = std::make_unique<TH1D>(histName, "hist", 200, 0, 140);
        hist->SetDirectory(nullptr);  // Important: Detach from current directory
        
        // Fill and analyze histogram
        tree->Draw(Form("discr>>%s", histName), cuts, "goff");
        hist->GetXaxis()->SetRangeUser(AnalysisConfig::discrMin, AnalysisConfig::discrMax);
        
        return {hist->GetMean(), hist->GetMeanError()};
    }

public:
    void Analyze() {
        AnalysisConfig config;
        
        // Validate slice width
        if ((static_cast<int>(config.y_finish - config.y_start) % 
             static_cast<int>(config.slice_width)) != 0) {
            std::cerr << "Invalid slice width configuration" << std::endl;
            return;
        }

        int n_slices = (config.y_finish - config.y_start) / config.slice_width;
        std::vector<double> slice_centres(n_slices);
        std::vector<double> slice_discr_mean_1(n_slices);
        std::vector<double> slice_discr_mean_2(n_slices);
        std::vector<double> err_discr_mean_1(n_slices);
        std::vector<double> err_discr_mean_2(n_slices);

        // Base geometric cut for drum interior
        TCut cut1 = "(z*z)+(x*x)<(300*480)";
        
        // Process input files
        std::unique_ptr<TFile> signal_file(new TFile(config.signal_file, "READ"));
        std::unique_ptr<TFile> bkg_file(new TFile(config.background_file, "READ"));
        
        if (!signal_file || !bkg_file || signal_file->IsZombie() || bkg_file->IsZombie()) {
            std::cerr << "Error opening input files" << std::endl;
            return;
        }

        auto signal_tree = (TTree*)signal_file->Get("T");
        auto bkg_tree = (TTree*)bkg_file->Get("T");

        // Process all slices
        double y_high = config.y_start + config.slice_width;
        for (int i = 0; i < n_slices; i++) {
            double y_low = config.y_start + i * config.slice_width;
            std::cout << "Processing slice: " << y_low << " to " << y_high << std::endl;

            slice_centres[i] = 0.5 * (y_low + y_high);
            
            // Create slice cut
            TCut cut2 = Form("y>%f && y<%f", y_low, y_high);
            TCut combined_cut = cut1 && cut2;

            // Process signal and background
            auto [mean1, error1] = ProcessSlice(signal_tree, combined_cut, "signal_hist");
            auto [mean2, error2] = ProcessSlice(bkg_tree, combined_cut, "bkg_hist");
            
            slice_discr_mean_1[i] = mean1;
            slice_discr_mean_2[i] = mean2;
            err_discr_mean_1[i] = error1;
            err_discr_mean_2[i] = error2;
            
            y_high += config.slice_width;
        }

        // Create visualization
        std::vector<double> zero_errors(n_slices, 0);
        auto canvas = std::make_unique<TCanvas>("c2", "Graph", 200, 100, 1300, 700);
        canvas->SetFillColor(0);

        // Create and configure graphs
        auto gr1 = std::make_unique<TGraphErrors>(n_slices, 
            slice_centres.data(), slice_discr_mean_1.data(), 
            zero_errors.data(), err_discr_mean_1.data());
        gr1->SetMarkerStyle(8);
        gr1->SetMarkerSize(1.0);
        gr1->SetMarkerColor(kRed);

        auto gr2 = std::make_unique<TGraphErrors>(n_slices, 
            slice_centres.data(), slice_discr_mean_2.data(), 
            zero_errors.data(), err_discr_mean_2.data());
        gr2->SetMarkerStyle(8);
        gr2->SetMarkerSize(1.0);
        gr2->SetMarkerColor(kBlack);

        // Create and configure multigraph
        auto mg = std::make_unique<TMultiGraph>();
        mg->Add(gr1.get());
        mg->Add(gr2.get());
        mg->SetTitle("Discriminator mean in drum slices;XZ plane (mm);#mu_{discriminator}");
        mg->Draw("AP");
        
        mg->GetYaxis()->SetRangeUser(8.7, 10);
        mg->GetYaxis()->SetTitleSize(0.05);
        mg->GetXaxis()->SetTitleSize(0.05);
        mg->GetYaxis()->SetTitleOffset(0.70);
        mg->GetXaxis()->SetTitleOffset(0.80);

        canvas->Update();
        canvas->GetFrame()->SetLineWidth(1);
        canvas->GetFrame()->SetFillColor(10);
        canvas->Modified();
        
        canvas->SaveAs("Slice_plotDiscr_concerteVS_Ucube_9.42L_largedrum_XZslice.pdf");
    }
};

void slices_plot_discr_hists_xz() {
    DrumSliceAnalyzer analyzer;
    analyzer.Analyze();
}

int main(int argc, char** argv) {
    slices_plot_discr_hists_xz();
    return 0;
}
