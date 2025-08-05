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
        double z_start = -300;
        double z_finish = 300;
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
        if ((static_cast<int>(config.z_finish - config.z_start) % 
             static_cast<int>(config.slice_width)) != 0) {
            std::cerr << "Invalid slice width configuration" << std::endl;
            return;
        }

        int n_slices = (config.z_finish - config.z_start) / config.slice_width;
        std::vector<double> slice_centres(n_slices);
        std::vector<double> mean_signal(n_slices);
        std::vector<double> mean_background(n_slices);
        std::vector<double> error_signal(n_slices);
        std::vector<double> error_background(n_slices);

        // Base geometric cut for drum interior
        TCut base_cut = "(x*x)+(y*y)<(480*300)";
        
        // Open input files with RAII
        std::unique_ptr<TFile> signal_file(new TFile(config.signal_file, "READ"));
        std::unique_ptr<TFile> bkg_file(new TFile(config.background_file, "READ"));
        
        if (!signal_file || !bkg_file || signal_file->IsZombie() || bkg_file->IsZombie()) {
            std::cerr << "Error opening input files" << std::endl;
            return;
        }

        auto signal_tree = (TTree*)signal_file->Get("T");
        auto bkg_tree = (TTree*)bkg_file->Get("T");

        // Process all slices
        double z_high = config.z_start + config.slice_width;
        for (int i = 0; i < n_slices; i++) {
            double z_low = config.z_start + i * config.slice_width;
            std::cout << "Processing slice: " << z_low << " to " << z_high << std::endl;
            
            slice_centres[i] = 0.5 * (z_low + z_high);
            
            // Create slice cut
            TCut slice_cut = Form("z>%f && z<%f", z_low, z_high);
            TCut combined_cut = base_cut && slice_cut;

            // Process signal and background
            auto [sig_mean, sig_error] = ProcessSlice(signal_tree, combined_cut, "signal_hist");
            auto [bkg_mean, bkg_error] = ProcessSlice(bkg_tree, combined_cut, "bkg_hist");
            
            mean_signal[i] = sig_mean;
            mean_background[i] = bkg_mean;
            error_signal[i] = sig_error;
            error_background[i] = bkg_error;
            
            z_high += config.slice_width;
        }

        // Create visualization
        std::vector<double> zero_errors(n_slices, 0);
        auto canvas = std::make_unique<TCanvas>("c2", "Graph", 200, 100, 1300, 700);
        canvas->SetFillColor(0);

        // Create and configure graphs
        auto gr_signal = std::make_unique<TGraphErrors>(n_slices, 
            slice_centres.data(), mean_signal.data(), 
            zero_errors.data(), error_signal.data());
        gr_signal->SetMarkerStyle(8);
        gr_signal->SetMarkerSize(1.0);
        gr_signal->SetMarkerColor(kRed);

        auto gr_background = std::make_unique<TGraphErrors>(n_slices, 
            slice_centres.data(), mean_background.data(), 
            zero_errors.data(), error_background.data());
        gr_background->SetMarkerStyle(8);
        gr_background->SetMarkerSize(1.0);
        gr_background->SetMarkerColor(kBlack);

        // Create and configure multigraph
        auto mg = std::make_unique<TMultiGraph>();
        mg->Add(gr_signal.get());
        mg->Add(gr_background.get());
        mg->SetTitle("Discriminator mean in drum slices;YX plane (mm);#mu_{discriminator}");
        mg->Draw("AP");
        
        mg->GetYaxis()->SetRangeUser(8.3, 10.7);
        mg->GetYaxis()->SetTitleSize(0.05);
        mg->GetXaxis()->SetTitleSize(0.05);
        mg->GetYaxis()->SetTitleOffset(0.70);
        mg->GetXaxis()->SetTitleOffset(0.80);

        canvas->Update();
        canvas->GetFrame()->SetLineWidth(1);
        canvas->GetFrame()->SetFillColor(10);
        canvas->Modified();
        
        canvas->SaveAs("Slice_plotDiscr_concerteVS_Ucube_9.42L_largedrum_YXslice.pdf");
    }
};

void slices_plot_discr_hists_yx() {
    DrumSliceAnalyzer analyzer;
    analyzer.Analyze();
}

int main(int argc, char** argv) {
    slices_plot_discr_hists_yx();
    return 0;
}
