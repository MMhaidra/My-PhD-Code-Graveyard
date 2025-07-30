// M.MHAIDRA
// This macro analyzes and plots uncertainty metrics for volume reconstruction
// in different material configurations (Concrete, Eurobitum, STE3)

#include <iostream>
#include <memory>
#include <vector>
// ROOT graphics includes
#include "TCanvas.h"
#include "TGraph.h"
#include "TGraphErrors.h"
#include "TH1D.h"
#include "TLine.h"
#include "TStyle.h"
#include "TF1.h"
// ROOT analysis includes
#include "TTree.h"
#include "TFile.h"
#include "TSpectrum.h"
#include "TVirtualFitter.h"

class UncertaintyAnalyzer {
private:
    struct VolumeData {
        static constexpr int nPoints = 9;
        const std::vector<double> volumes = {
            1.021, 1.5, 2.00, 3.015, 4.020, 5.005, 9.000, 11.780, 14.541
        };
        std::vector<double> uncertainties;
        std::vector<double> errors;
        const char* materialType;
    };

    // Helper function to calculate relative uncertainties
    std::vector<double> CalculateRelativeUncertainties(
        const std::vector<double>& measured, 
        const std::vector<double>& reference) {
        
        std::vector<double> uncertainties(measured.size());
        for (size_t i = 0; i < measured.size(); ++i) {
            uncertainties[i] = 1.0 - (measured[i] / reference[i]);
        }
        return uncertainties;
    }

    // Helper function to create and configure histogram
    std::unique_ptr<TH1D> CreateUncertaintyHistogram(
        const VolumeData& data, const char* title) {
        
        auto hist = std::make_unique<TH1D>("Uncertainty_hist", 
            (std::string("Uncertainty_hist_") + data.materialType).c_str(),
            20, -0.6, 0.6);
        
        // Fill and configure histogram
        for (const auto& uncertainty : data.uncertainties) {
            hist->Fill(uncertainty);
        }

        hist->SetXTitle("Relative Uncertainty of reconstructed volume");
        hist->SetYTitle("Entries");
        hist->SetLineColor(1);
        
        return hist;
    }

public:
    void Analyze() {
        // Configure data for Eurobitum analysis
        VolumeData eurobitumData{
            // Measured volumes for Eurobitum
            {0.671, 1.341, 1.881, 2.751, 4.022, 4.744, 9.151, 11.385, 14.533},
            // Standard errors
            {0.05, 0.047, 0.043, 0.048, 0.045, 0.044, 0.042, 0.046, 0.047},
            "Eurobitum"
        };

        // Calculate relative uncertainties
        eurobitumData.uncertainties = CalculateRelativeUncertainties(
            {0.671, 1.341, 1.881, 2.751, 4.022, 4.744, 9.151, 11.385, 14.533},
            eurobitumData.volumes);

        // Create and configure histogram
        gStyle->SetOptStat(0);
        auto uncertaintyHist = CreateUncertaintyHistogram(
            eurobitumData, "Distribution of Relative Uncertainties");

        // Fit with Gaussian
        auto gaussFit = std::make_unique<TF1>("gaussFit", "gaus");
        uncertaintyHist->Fit(gaussFit.get());

        // Print statistics
        std::cout << "Statistics Summary:\n"
                  << "Std Dev = " << uncertaintyHist->GetStdDev() << "\n"
                  << "Error Std Dev = " << uncertaintyHist->GetStdDevError() << "\n"
                  << "Fit Mean = " << gaussFit->GetParameter(1) << "\n"
                  << "Fit Mean Error = " << gaussFit->GetParError(1) << std::endl;

        // Create canvas and draw
        auto canvas = std::make_unique<TCanvas>("c2", "Graph", 200, 50, 1200, 600);
        uncertaintyHist->Draw();
        
        // Save results
        canvas->SaveAs("Distribution_Relative_uncertainty_against_gas_volume_(L)_Eurobitum.pdf");
    }
};

void erro_recovol_plot() {
    UncertaintyAnalyzer analyzer;
    analyzer.Analyze();
}

int main(int argc, char** argv) {
    erro_recovol_plot();
    return 0;
}

