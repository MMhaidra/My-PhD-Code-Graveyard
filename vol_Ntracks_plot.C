/* This macro analyzes and plots the relationship between bubble gas volume 
 * and number of tracks in the detector.
 * 
 * Memory Management Note:
 * ROOT objects should be properly managed to prevent memory leaks.
 * Using smart pointers and RAII principles for better resource management.
 */

#include <memory>
#include <array>

// ROOT data analysis
#include "TF1.h"
#include "TGraph.h"
#include "TGraphErrors.h"

// ROOT graphics and visualization
#include "TCanvas.h"
#include "TStyle.h"
#include "TFrame.h"

class TrackVolumeAnalyzer {
private:
    struct AnalysisConfig {
        // Number of data points
        static constexpr int nPoints = 6;
        
        // Analysis parameters
        double fitRangeMin = 1e-6;
        double fitRangeMax = 9.1;
        
        // Data arrays
        struct DataSet {
            std::array<double, nPoints> volume = {
                6.032, 7.125, 9.425, 12.925, 17.191, 20.701
            };
            std::array<double, nPoints> nTracks = {
                59169081, 59176150, 59187805, 59201672, 59233801, 59241086
            };
            // Alternative track data for comparison
            std::array<double, nPoints> nTracksAlt = {
                58693021, 58707207, 58717725, 58726805, 58744087, 58757879
            };
            // Error arrays (currently zeros)
            std::array<double, nPoints> volumeErrors = {0};
            std::array<double, nPoints> trackErrors = {0};
        };
    };

public:
    void Analyze() {
        AnalysisConfig config;
        AnalysisConfig::DataSet data;
        
        // Create graph with errors
        auto graph = std::make_unique<TGraphErrors>(
            config.nPoints,
            data.volume.data(),
            data.nTracks.data(),
            data.volumeErrors.data(),
            data.trackErrors.data()
        );

        // Configure graph appearance
        graph->SetTitle("Number of tracks against bubble gas volume;"
                       "Gas volume (L);Number of tracks");
        graph->SetMarkerStyle(33);
        graph->SetMarkerSize(1.2);
        graph->SetMarkerColorAlpha(kBlue, 1.2);

        // Create and configure linear fit
        auto fit = std::make_unique<TF1>("linearFit", "[0]*x+[1]", 
                                        config.fitRangeMin, config.fitRangeMax);
        fit->SetLineColor(kMagenta+2);
        fit->SetLineWidth(2);

        // Create canvas and perform analysis
        auto canvas = std::make_unique<TCanvas>("c1", "Graph", 
                                              200, 50, 1200, 600);
        
        // Perform fit and draw
        graph->Fit(fit.get());
        graph->Draw("A*");
        
        // Update and save
        canvas->Update();
        canvas->Modified();
        canvas->SaveAs("NumberOfTracks_against_bubble_gasVolume(L).pdf", "pdf");
    }
};

void vol_Ntracks_plot() {
    TrackVolumeAnalyzer analyzer;
    analyzer.Analyze();
}

int main(int argc, char** argv) {
    vol_Ntracks_plot();
    return 0;
}

