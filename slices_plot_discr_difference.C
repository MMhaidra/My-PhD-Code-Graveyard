// M.Mhaidra 20/02/2019 //
/* This macro calculates and plots the mean of the discriminator difference 
 * between the matrix drum and the gas inside.
 *
 * Important Memory Management Note:
 * ROOT histograms are associated with a TDirectory upon creation (usually current file).
 * When directory changes (new file opened), histogram pointers become invalid.
 * Use TH1::SetDirectory(0) to detach histograms from file for safer handling.
 */

#include <iostream>
#include <memory>
#include <vector>
#include <algorithm>
#include <iomanip>

// ROOT core classes
#include "TFile.h"
#include "TTree.h"
#include "TCut.h"
#include "TString.h"
#include "TRandom.h"

// ROOT histograms and graphs
#include "TH1D.h"
#include "TGraph.h"
#include "TMultiGraph.h"
#include "TGraphErrors.h"

// ROOT graphics and style
#include "TCanvas.h"
#include "TStyle.h"
#include "TLine.h"
#include "TPaveText.h"
#include "TLegend.h"
#include "TFrame.h"

class DiscriminatorAnalyzer {
private:
    struct AnalysisConfig {
        // Slice parameters (in mm)
        double x_start = -480;
        double x_finish = 480;
        double slice_width = 40;
        
        // Geometric cut parameters
        double cylinder_radius = 50;
        
        // Analysis range
        double discr_min = 7.0;
        double discr_max = 14.0;
        
        // File paths
        const char* signal_file = "/home/mmhaidra/SliceMethod/Chance_largedrum_bub_tracks35_discr/"
                                 "largedrum_bub_2.92l_tracks35.discriminator.root";
        const char* background_file = "/home/mmhaidra/SliceMethod/Chance_largedrum_bub_tracks35_discr/"
                                    "largedrum_onlyconcrete_tracks35.discriminator.root";
    };

    // Process single slice with proper memory management
    std::pair<double, double> ProcessSlice(TTree* tree, const TCut& cuts, const char* name) {
        auto hist = std::make_unique<TH1D>(name, "hist", 200, 0, 140);
        hist->SetDirectory(nullptr);  // Important: Detach from file
        
        tree->Draw(Form("discr>>%s", name), cuts, "goff");
        hist->GetXaxis()->SetRangeUser(AnalysisConfig::discr_min, AnalysisConfig::discr_max);
        
        return {hist->GetMean(), hist->GetMeanError()};
    }

    // Validate slice width parameters
    bool ValidateSliceWidth(double start, double end, double width) {
        if ((static_cast<int>(end - start) % static_cast<int>(width)) != 0) {
            std::cout << "Invalid slice width. Current: " << width << std::endl;
            return false;
        }
        return true;
    }

public:
    void Analyze() {
        AnalysisConfig config;
        
        // Validate slice width
        if (!ValidateSliceWidth(config.x_start, config.x_finish, config.slice_width)) {
            return;
        }

        int n_slices = (config.x_finish - config.x_start) / config.slice_width;
        std::vector<double> slice_centres(n_slices);
        std::vector<double> mean_diff(n_slices);
        std::vector<double> error_diff(n_slices);
        
        // Base geometric cut
        TCut base_cut = Form("(z*z)+(y*y)<%f", pow(config.cylinder_radius, 2));
        
        // Process each slice
        double x_high = config.x_start + config.slice_width;
        for (int i = 0; i < n_slices; ++i) {
            double x_low = config.x_start + i * config.slice_width;
            slice_centres[i] = 0.5 * (x_low + x_high);
            
            // Create slice cut
            TCut slice_cut = Form("x>%f && x<%f", x_low, x_high);
            TCut combined_cut = base_cut && slice_cut;
            
            // Process signal file
            std::unique_ptr<TFile> signal_file(new TFile(config.signal_file, "READ"));
            auto signal_tree = (TTree*)signal_file->Get("T");
            auto [signal_mean, signal_error] = ProcessSlice(signal_tree, combined_cut, "signal_hist");
            
            // Process background file
            std::unique_ptr<TFile> bkg_file(new TFile(config.background_file, "READ"));
            auto bkg_tree = (TTree*)bkg_file->Get("T");
            auto [bkg_mean, bkg_error] = ProcessSlice(bkg_tree, combined_cut, "bkg_hist");
            
            // Calculate difference
            mean_diff[i] = (signal_mean - bkg_mean) / (signal_error + bkg_error);
            error_diff[i] = signal_error + bkg_error;
            
            x_high += config.slice_width;
        }

        // Create and configure graph
        std::vector<double> zero_errors(n_slices, 0);
        auto graph = std::make_unique<TGraphErrors>(n_slices, 
            slice_centres.data(), mean_diff.data(), 
            zero_errors.data(), error_diff.data());
            
        graph->SetMarkerStyle(8);
        graph->SetMarkerSize(1.2);
        graph->SetMarkerColor(kBlue);
        graph->SetFillColor(4);

        // Create and configure canvas
        auto canvas = std::make_unique<TCanvas>("c2", "Graph", 200, 100, 1300, 700);
        canvas->SetFillColor(0);
        
        auto mg = std::make_unique<TMultiGraph>();
        mg->Add(graph.get());
        mg->Draw("AB");
        mg->SetTitle("#mu_{discr_difference} in drum slices;YZ plane (mm);"
                    "#mu_{discr_difference}/error");
        
        mg->GetYaxis()->SetRangeUser(-5, 15);
        mg->GetYaxis()->SetTitleSize(0.05);
        mg->GetXaxis()->SetTitleSize(0.05);
        mg->GetYaxis()->SetTitleOffset(0.70);
        mg->GetXaxis()->SetTitleOffset(0.80);
        
        canvas->Update();
        canvas->GetFrame()->SetLineWidth(1);
        canvas->GetFrame()->SetFillColor(10);
        canvas->Modified();
        
        canvas->SaveAs("Slice_plotDiscr_difference_Cylindrical_2l_Ucube_1cm_Lower_YZslice"
                      "_35tracks_40steps_addedCutTest.pdf");
    }
};

void slices_plot_discr_difference() {
    DiscriminatorAnalyzer analyzer;
    analyzer.Analyze();
}

int main(int argc, char** argv) {
    slices_plot_discr_difference();
    return 0;
}
