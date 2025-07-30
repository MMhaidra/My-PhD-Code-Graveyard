/* This macro calculates the means of the discriminator distributions for a range 
   of gas bubble sizes and plots them against the gas volume. */

#include <memory>
#include <vector>
#include <iostream>
#include "TCanvas.h"
#include "TTree.h"
#include "TH1D.h"
#include "TH3F.h"
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
            2.95, 4.42, 6.03, 7.12, 9.42, 11.78, 17.19, 20.70
        };
        const char* baseDir = "/home/mmhaidra/SliceMethod/Chance_largedrum_bub_tracks15_discr_STE3_dense/";
        const char* suffix = "_STE3_dense_tracks35.discriminator.root";
    };

    struct HistogramResult {
        double mean;
        double error;
    };

    // Helper function to load 3D histogram from file
    std::unique_ptr<TH3F> LoadHistogram(const char* filename) {
        std::unique_ptr<TFile> file(new TFile(filename, "READ"));
        if (!file || file->IsZombie()) return nullptr;

        TH3F* hist = (TH3F*)file->Get("histBS");
        if (hist) hist->SetDirectory(0); // Important: Detach from file directory
        return std::unique_ptr<TH3F>(hist);
    }

    // Process single histogram and calculate mean/error
    HistogramResult ProcessHistogram(TH3F* hist3D) {
        if (!hist3D) return {0.0, 0.0};

        auto discr_hist = std::make_unique<TH1D>("discr_hist", "hist", 200, 0, 140);

        // Iterate through all bins within specified range
        for (int i = hist3D->GetXaxis()->FindBin(-440); i < hist3D->GetXaxis()->FindBin(440); i++) {
            for (int j = hist3D->GetYaxis()->FindBin(-300); j < hist3D->GetYaxis()->FindBin(300); j++) {
                for (int k = hist3D->GetZaxis()->FindBin(-300); k < hist3D->GetZaxis()->FindBin(300); k++) {
                    discr_hist->Fill(hist3D->GetBinContent(i,j,k));
                }
            }
        }

        // Configure histogram display
        discr_hist->GetXaxis()->SetRangeUser(5.0, 15.0);
        discr_hist->SetMarkerStyle(21);
        discr_hist->SetMarkerColor(kBlue+8);

        return {discr_hist->GetMean(), discr_hist->GetMeanError()};
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
            
            // Load and process histogram
            auto hist3D = LoadHistogram(filename.Data());
            if (!hist3D) {
                std::cerr << "Failed to load histogram for volume point " << i << std::endl;
                continue;
            }

            auto result = ProcessHistogram(hist3D.get());
            discrMean[i] = result.mean;
            errorMean[i] = result.error;

            std::cout << "Volume point " << i << " - Mean: " << result.mean 
                     << " Error: " << result.error << std::endl;
        }

        // Create and configure graph
        auto graph = std::make_unique<TGraphErrors>(config.nPoints, 
                                                  config.volumes.data(), 
                                                  discrMean.data(),
                                                  errorVol.data(), 
                                                  errorMean.data());
        
        // Configure and perform fit
        auto fit = std::make_unique<TF1>("f2", "[0]*x+[1]", 1e-6, 9.1);
        fit->SetLineColor(kGreen+2);
        fit->SetLineWidth(3);
        
        // Create canvas and plot
        auto canvas = std::make_unique<TCanvas>("c1", "Graph", 200, 50, 1200, 600);
        
        graph->SetTitle("Gas volume against mean of discriminator distribution in STE3 bitumen drum;"
                       "Hydrogen Gas volume (L);#mu_{discriminator}");
        graph->SetMarkerStyle(8);
        graph->SetMarkerSize(1.2);
        
        graph->Fit(fit.get());
        graph->Draw("A*");
        
        canvas->Update();
        canvas->Modified();
        canvas->SaveAs("gas_volume_Hydrogena_gainst_Mean_discr_tracks35_STE3_dense.pdf");
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

