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
#include "TStyle.h"
#include "TFrame.h"

class DrumSliceAnalyzer {
private:
    struct AnalysisConfig {
        // Geometric parameters (mm)
        double x_start = -400;
        double x_finish = 400;
        double slice_width = 40;
        
        // Analysis parameters
        static constexpr double discrMin = 7.0;
        static constexpr double discrMax = 14.0;
        
        // File paths
        const char* file1 = "/home/mmhaidra/SliceMethod/Chance_largedrum_bub_tracks35_discr_EUROBITUM/"
                           "largedrum_EUROBITUM_tracks35.discriminator.root";
        const char* file2 = "/home/mmhaidra/SliceMethod/Chance_largedrum_bub_tracks35_discr_EUROBITUM/"
                           "largedrum_ShiftedDrum10cm_EUROBITUM_tracks35.discriminator.root";
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
        if ((static_cast<int>(config.x_finish - config.x_start) % 
             static_cast<int>(config.slice_width)) != 0) {
            std::cerr << "Invalid slice width configuration" << std::endl;
            return;
        }

        int n_slices = (config.x_finish - config.x_start) / config.slice_width;
        std::vector<double> slice_centres(n_slices);
        std::vector<double> mean_1(n_slices);
        std::vector<double> mean_2(n_slices);
        std::vector<double> error_1(n_slices);
        std::vector<double> error_2(n_slices);

        // Base geometric cut for drum interior
        TCut base_cut = "(z*z)+(y*y)<(300*300)";
        
        double x_high = config.x_start + config.slice_width;
        for (int i = 0; i < n_slices; i++) {
            double x_low = config.x_start + i * config.slice_width;
            std::cout << "Processing slice: " << x_low << " to " << x_high << std::endl;
            
            slice_centres[i] = 0.5 * (x_low + x_high);
            
            // Create slice cut
            TCut slice_cut = Form("x>%f && x<%f", x_low, x_high);
            TCut combined_cut = base_cut && slice_cut;

            // Process first file
            std::unique_ptr<TFile> file1(new TFile(config.file1, "READ"));
            auto tree1 = (TTree*)file1->Get("T");
            auto [mean1, error1] = ProcessSlice(tree1, combined_cut, "hist1");
            mean_1[i] = mean1;
            error_1[i] = error1;

            // Process second file
            std::unique_ptr<TFile> file2(new TFile(config.file2, "READ"));
            auto tree2 = (TTree*)file2->Get("T");
            auto [mean2, error2] = ProcessSlice(tree2, combined_cut, "hist2");
            mean_2[i] = mean2;
            error_2[i] = error2;
            
            x_high += config.slice_width;
        }

        // Create visualization
        std::vector<double> zero_errors(n_slices, 0);
        auto canvas = std::make_unique<TCanvas>("c2", "Graph", 200, 100, 1300, 700);
        canvas->SetFillColor(0);

        // Create and configure graphs
        auto gr1 = std::make_unique<TGraphErrors>(n_slices, 
            slice_centres.data(), mean_1.data(), 
            zero_errors.data(), error_1.data());
        gr1->SetMarkerStyle(8);
        gr1->SetMarkerSize(1.0);
        gr1->SetMarkerColor(kRed);

        auto gr2 = std::make_unique<TGraphErrors>(n_slices, 
            slice_centres.data(), mean_2.data(), 
            zero_errors.data(), error_2.data());
        gr2->SetMarkerStyle(8);
        gr2->SetMarkerSize(1.0);
        gr2->SetMarkerColor(kBlack);

        // Create and configure multigraph
        auto mg = std::make_unique<TMultiGraph>();
        mg->Add(gr1.get());
        mg->Add(gr2.get());
        mg->SetTitle("Discriminator mean in drum slices;YZ plane (mm);#mu_{discriminator}");
        mg->Draw("AP");
        
        mg->GetYaxis()->SetRangeUser(9.3, 10.6);
        mg->GetYaxis()->SetTitleSize(0.05);
        mg->GetXaxis()->SetTitleSize(0.05);
        mg->GetYaxis()->SetTitleOffset(0.70);
        mg->GetXaxis()->SetTitleOffset(0.80);

        canvas->Update();
        canvas->GetFrame()->SetLineWidth(1);
        canvas->GetFrame()->SetFillColor(10);
        canvas->Modified();
        
        canvas->SaveAs("EurobitumDrum_Vs_EurobitumDrum10cmShifted_35Tracks_steps40.pdf");
    }
};

void slices_plot_discr_hists() {
    DrumSliceAnalyzer analyzer;
    analyzer.Analyze();
}

int main(int argc, char** argv) {
    slices_plot_discr_hists();
    return 0;
}
