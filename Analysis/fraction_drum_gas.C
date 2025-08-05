// This macro analyzes and plots relative uncertainties of reconstructed gas volumes
// for different drum sizes (Small, Medium, Large) against the gas/drum volume fraction

#include <memory>
#include <vector>
#include <array>
// ROOT graphics includes
#include "TCanvas.h"
#include "TGraph.h"
#include "TGraphErrors.h"
#include "TMultiGraph.h"
#include "TLine.h"
#include "TStyle.h"
// ROOT data handling includes
#include "TH1D.h"
#include "TFile.h"
#include "TTree.h"

class DrumVolumeAnalyzer {
private:
    struct DrumConfig {
        const char* name;
        double drumVolume;
        double concreteVolume;
    };

    struct VolumeData {
        static constexpr int nPoints = 7;
        std::vector<double> volumes;
        std::vector<double> relativeUncertainties;
        std::vector<double> volumeErrors;
        std::vector<double> uncertaintyErrors;
        int color;
    };

    // Initialize drum configurations
    const std::array<DrumConfig, 3> drumConfigs = {{
        {"Small", 26.4207, 18.3170},   // Small drum
        {"Medium", 87.1320, 63.9105},  // Medium drum
        {"Large", 228.0, 172.5210}     // Large drum
    }};

    // Base volumes for all drums
    const std::vector<double> baseVolumes = {
        1.021, 3.015, 4.020, 5.005, 9.000, 11.780, 14.541
    };

    // Helper function to create and configure graph
    std::unique_ptr<TGraphErrors> CreateGraph(const VolumeData& data, int color) {
        auto graph = std::make_unique<TGraphErrors>(data.nPoints,
            data.volumes.data(),
            data.relativeUncertainties.data(),
            data.volumeErrors.data(),
            data.uncertaintyErrors.data());
        
        graph->SetMarkerStyle(8);
        graph->SetMarkerSize(1.0);
        graph->SetMarkerColor(color);
        
        return graph;
    }

    // Initialize data for a specific drum
    VolumeData InitializeDrumData(const DrumConfig& config, 
                                 const std::vector<double>& uncertainties,
                                 const std::vector<double>& errors,
                                 int color) {
        VolumeData data;
        // Calculate volume fractions
        data.volumes.reserve(VolumeData::nPoints);
        for (const auto& vol : baseVolumes) {
            data.volumes.push_back(vol / config.drumVolume);
        }
        
        data.relativeUncertainties = uncertainties;
        data.volumeErrors = std::vector<double>(VolumeData::nPoints, 0);
        data.uncertaintyErrors = errors;
        data.color = color;
        
        return data;
    }

public:
    void Analyze() {
        // Initialize data for each drum
        std::vector<VolumeData> drumData;
        
        // Small drum data
        drumData.push_back(InitializeDrumData(
            drumConfigs[0],
            {0.06, 0.008, -0.007, 0.009, -0.001, 0.012, 0.011},
            {0.16, 0.0192, 0.0250, 0.0255, 0.0238, 0.0195, 0.0197},
            kBlack
        ));
        
        // Medium drum data
        std::vector<double> mediumUncertainties;
        for (size_t i = 0; i < baseVolumes.size(); i++) {
            // Calculate relative uncertainties for medium drum
            double reconVolume = baseVolumes[i];
            double measuredVolume = (i == 0) ? 1.091 : (i == 1) ? 2.932 : 
                                  (i == 2) ? 4.038 : (i == 3) ? 5.011 : 
                                  (i == 4) ? 8.940 : (i == 5) ? 11.881 : 14.661;
            mediumUncertainties.push_back(1 - (measuredVolume/reconVolume));
        }
        
        drumData.push_back(InitializeDrumData(
            drumConfigs[1],
            mediumUncertainties,
            {0.06, 0.0480, 0.046, 0.043, 0.043, 0.041, 0.044},
            kBlue
        ));
        
        // Large drum data
        drumData.push_back(InitializeDrumData(
            drumConfigs[2],
            {1-(0.671/1.021), 1-(2.891/3.015), 1-(3.892/4.020), 
             1-(4.644/5.005), 1-(8.895/9.000), 1-(11.465/11.780), 
             1-(14.448/14.541)},
            {0.05, 0.048, 0.045, 0.044, 0.042, 0.046, 0.047},
            kGreen+2
        ));

        // Create and configure canvas
        auto canvas = std::make_unique<TCanvas>("c1", "Graph", 200, 50, 1200, 600);
        auto mg = std::make_unique<TMultiGraph>();

        // Create and add graphs
        std::vector<std::unique_ptr<TGraphErrors>> graphs;
        for (const auto& data : drumData) {
            graphs.push_back(CreateGraph(data, data.color));
            mg->Add(graphs.back().get());
        }

        // Configure and draw multigraph
        mg->Draw("AP");
        mg->SetTitle("Relative uncertainty on the reconstructed volume against the "
                    "Gas/Drum volume fraction;Gas/Drum volume (L);"
                    "Relative uncertainty on the reconstructed gas volume");

        // Add reference line
        auto line = std::make_unique<TLine>(0, 0, 0.58, 0);
        line->SetLineStyle(1);
        line->SetLineColor(29);
        line->SetLineWidth(1);
        line->Draw();

        // Configure canvas
        canvas->SetLogx();
        canvas->Update();
        canvas->Modified();
        
        canvas->SaveAs("Relative_Uncertainty_VS_VolumeFraction_logX.pdf", "pdf");
    }
};

void fraction_drum_gas() {
    DrumVolumeAnalyzer analyzer;
    analyzer.Analyze();
}

int main(int argc, char** argv) {
    fraction_drum_gas();
    return 0;
}

