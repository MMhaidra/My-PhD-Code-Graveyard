/* This macro calculates the means of the discriminator distributions for a range 
   of gas bubble sizes and plots them against the gas volume. */

#include <memory>
#include <vector>
#include <iostream>
#include "TCanvas.h"
#include "TTree.h"
#include "TH1D.h"
#include "TFile.h"
#include "TStyle.h"
#include "TF1.h"
#include "TGraph.h"
#include "TGraphErrors.h"

class DiscrVolumeAnalyzer {
private:
    struct AnalysisConfig {
        static constexpr int nPoints = 8;
        // STE3 Dense volumes in liters
        const std::vector<double> volumes = {
            2.95, 4.42, 5.0, 6.03, 7.12, 9.42, 11.78, 17.19, 20.70
        };
        // File path configuration
        const char* baseDir = "/home/mmhaidra/SliceMethod/Chance_largedrum_bub_tracks15_discr_density3/";
        const char* suffix = "_bitumen_dense3_lowZ_tracks15.discriminator.root";
    };

    struct HistogramResult {
        double mean;
        double error;
    };

    // Helper function to process a single histogram from file
    HistogramResult ProcessHistogram(const char* filename) {
        // Open file and get tree
        std::unique_ptr<TFile> file(new TFile(filename, "READ"));
        if (!file || file->IsZombie()) {
            std::cerr << "Error opening file: " << filename << std::endl;
            return {0, 0};
        }

        TTree* tree = (TTree*)file->Get("T");
        if (!tree) return {0, 0};

        // Create and fill histogram with geometric cuts
        auto hist = std::make_unique<TH1D>("discr_hist", "hist", 200, 0, 140);
        tree->Draw("discr>>discr_hist", 
                  "((z*z)+(y*y)<(300*300))&&(x>-480)&&(x<480)", "goff");
        
        // Important: Detach histogram from file to prevent memory issues
        hist->SetDirectory(0);
        hist->GetXaxis()->SetRangeUser(5.0, 15.0);

        return {hist->GetMean(), hist->GetMeanError()};
    }

public:
    void Analyze() {
        AnalysisConfig config;
        std::vector<double> errorVol(config.nPoints, 0);
        std::vector<double> discrMean(config.nPoints);
        std::vector<double> errorMean(config.nPoints);

        // Process each volume point
        for (int i = 0; i < config.nPoints; ++i) {
            // Construct file path
            TString filename = TString::Format("%slargedrum_bub_%d%s", 
                             config.baseDir, i, config.suffix);
            
            // Process histogram and store results
            auto result = ProcessHistogram(filename.Data());
            discrMean[i] = result.mean;
            errorMean[i] = result.error;

            std::cout << "Point " << i << " - Mean: " << result.mean 
                     << " Error: " << result.error << std::endl;
        }

        // Create and configure graph
        auto graph = std::make_unique<TGraphErrors>(config.nPoints, 
                                                  config.volumes.data(), 
                                                  discrMean.data(),
                                                  errorVol.data(), 
                                                  errorMean.data());
        
        // Configure and perform linear fit
        auto fit = std::make_unique<TF1>("f2", "[0]*x+[1]", 1e-6, 9.1);
        fit->SetLineColor(kBlack+2);
        fit->SetLineWidth(3);
        
        // Create canvas and plot
        auto canvas = std::make_unique<TCanvas>("c1", "Graph", 200, 50, 1200, 600);
        
        graph->SetTitle("Gas volume against mean of discriminator distribution in "
                       "2.00 STE3 bitumen drum;Hydrogen Gas volume (L);#mu_{discriminator}");
        graph->SetMarkerStyle(8);
        graph->SetMarkerSize(1.2);
        
        graph->Fit(fit.get());
        graph->Draw("A*");
        
        canvas->Update();
        canvas->Modified();
        canvas->SaveAs("Gas_volume_Hydrogena_gainst_Mean_discr_tracks15_STE3_dense3.pdf");
    }
};

void discr_vol_plot() {
    DiscrVolumeAnalyzer analyzer;
    analyzer.Analyze();
}

int main(int argc, char** argv) {
    discr_vol_plot();
    return 0;
}

