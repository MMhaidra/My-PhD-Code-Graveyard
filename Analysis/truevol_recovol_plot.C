/* This macro compares reconstructed bubble volumes against true volumes
 * for different drum configurations (STE3, Eurobitum, concrete).
 * It creates scatter plots with error bars and linear fits.
 */

#include <memory>
#include <vector>
#include <array>

// ROOT data analysis
#include "TF1.h"
#include "TGraph.h"
#include "TGraphErrors.h"

// ROOT graphics and visualization
#include "TCanvas.h"
#include "TStyle.h"
#include "TFrame.h"
#include "TPaveText.h"
#include "TLegend.h"

class VolumeAnalyzer {
private:
    struct AnalysisConfig {
        // Number of data points
        static constexpr int nPoints = 7;
        
        // Analysis parameters
        double fitRangeMin = 1e-6;
        double fitRangeMax = 9.1;
        
        // Data arrays for different configurations
        struct DataSet {
            std::array<double, nPoints> trueVolume = {1.021, 3.015, 4.020, 5.005, 9.000, 11.780, 14.541};
            std::array<double, nPoints> recoVolume;
            std::array<double, nPoints> recoError;
        };
        
        // STE3 configuration data
        DataSet ste3 = {
            .recoVolume = {0.939, 2.717, 4.335, 5.454, 9.170, 12.017, 14.345},
            .recoError = {0.34, 0.31, 0.24, 0.23, 0.22, 0.21, 0.20}
        };
        
        // Commented configurations for reference
        /* Eurobitum configuration
        DataSet eurobitum = {
            .recoVolume = {1.107, 2.191, 4.312, 5.454, 8.975, 11.995, 14.048},
            .recoError = {0.3, 0.192, 0.250, 0.255, 0.238, 0.195, 0.197}
        }; */
    };

public:
    void Analyze() {
        AnalysisConfig config;
        
        // Create graph with errors
        // Note: Using nullptr for x-errors as they're all zero
        auto graph = std::make_unique<TGraphErrors>(
            config.nPoints,
            config.ste3.trueVolume.data(),
            config.ste3.recoVolume.data(),
            nullptr,
            config.ste3.recoError.data()
        );

        // Configure graph appearance
        graph->SetTitle("Reconstructed bubble volume against true volume for the STE3 bitumen drum;"
                       "True Gas volume (L);Reco. Gas volume (L)");
        graph->SetMarkerStyle(8);
        graph->SetMarkerSize(1.2);

        // Create and configure linear fit
        auto fit = std::make_unique<TF1>("linearFit", "[0]*x+[1]", 
                                        config.fitRangeMin, config.fitRangeMax);
        fit->SetLineColor(kBlue);
        fit->SetLineWidth(3);
        
        // Create canvas and draw
        auto canvas = std::make_unique<TCanvas>("c1", "Volume Comparison", 
                                              200, 50, 1200, 600);
        
        // Perform fit and draw graph
        graph->Fit(fit.get());
        graph->Draw("A*");
        
        // Update and save
        canvas->Update();
        canvas->Modified();
        canvas->SaveAs("recoVol_STE3_test.pdf", "pdf");
    }
};

void truevol_recovol_plot() {
    VolumeAnalyzer analyzer;
    analyzer.Analyze();
}

int main(int argc, char** argv) {
    truevol_recovol_plot();
    return 0;
}

