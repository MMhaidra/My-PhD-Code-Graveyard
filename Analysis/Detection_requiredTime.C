// M.Mhaidra 22/05/2020 //

/*This macro gives an idea on the minimum time required
 to detect hydrogen bubble inside bituminized drum.
 The trick is that histograms are associated with a TDirectory
 upon creation, which is most likely the open file. When the
 directory changes when a new file is opened, the pointer to
 the histogram is invalidated. TH3::SetDirectory is used to
 detach a histogram from a file, thus the reconstruction is much more quicker.*/

#include <memory>
#include <vector>
#include <iostream>
#include "TCanvas.h"
#include "TTree.h"
#include "TH1D.h"
#include "TFile.h"
#include "TGraph.h"
#include "TMultiGraph.h"
#include "TGraphErrors.h"
#include "TStyle.h"

class DetectionTimeAnalyzer {
private:
    struct FileConfig {
        TString basePath;
        TString referencePath;
        int nTracks;
        const char* suffix;
    };

    struct HistogramResult {
        double mean;
        double error;
    };

    static constexpr int nPoints = 9;
    std::vector<double> timePoints = {3, 4, 5, 6, 10, 15, 20, 25, 30};
    std::vector<double> errorTime = std::vector<double>(nPoints, 0);
    std::vector<double> errorDiff = std::vector<double>(nPoints, 0);

    // Processes a single histogram from file
    // Returns mean and error of the histogram
    HistogramResult ProcessHistogram(const char* filename) {
        // Open file and get tree
        std::unique_ptr<TFile> file(new TFile(filename, "READ"));
        if (!file || file->IsZombie()) return {0, 0};

        TTree* tree = (TTree*)file->Get("T");
        if (!tree) return {0, 0};

        // Create and fill histogram
        auto hist = std::make_unique<TH1D>("hist", "hist", 200, 0, 140);
        tree->Draw("discr>>hist", "((z*z)+(y*y)<(300*300))&&(x>-480)&&(x<480)", "goff");
        
        // Important: Detach histogram from file to prevent invalidation
        hist->SetDirectory(0);
        hist->GetXaxis()->SetRangeUser(5.0, 15.0);

        return {hist->GetMean(), hist->GetMeanError()};
    }

    // Processes a pair of files and returns discriminator differences
    std::vector<double> ProcessFilePair(const FileConfig& config) {
        std::vector<double> results(nPoints);
        
        for (int i = 1; i < nPoints; i++) {
            // Construct file paths
            TString mainFile = config.basePath + Form("%d", i+1) + config.suffix;
            TString refFile = config.referencePath + Form("%d", i+1) + config.suffix;
            
            // Process both files
            auto result1 = ProcessHistogram(mainFile.Data());
            auto result2 = ProcessHistogram(refFile.Data());
            
            // Calculate difference
            results[i] = (result1.mean - result2.mean) / (result1.error + result2.error);
            std::cout << "Tracks " << config.nTracks << " - Point " << i 
                     << " difference: " << results[i] << std::endl;
        }
        
        return results;
    }

    // Creates and configures a graph for the results
    TGraphErrors* CreateGraph(const std::vector<double>& data, 
                            int markerStyle, int markerColor, 
                            const char* title = nullptr) {
        auto graph = new TGraphErrors(nPoints, timePoints.data(), data.data(), 
                                    errorTime.data(), errorDiff.data());
        graph->SetMarkerStyle(markerStyle);
        graph->SetMarkerSize(1.2);
        graph->SetMarkerColor(markerColor);
        if (title) graph->SetTitle(title);
        return graph;
    }

public:
    void Analyze() {
        // Configure all file sets
        std::vector<FileConfig> configs = {
            {"/home/mmhaidra/SliceMethod/Exposure_time_study/largedrum_bub_4.42l_STE3_dense_tracks40_",
             "/home/mmhaidra/SliceMethod/Exposure_time_study/largedrum_OnlyBitumen_STE3_dense_tracks40_",
             40, ".discriminator.root"},
            // Add other configurations here
        };

        auto mg = std::make_unique<TMultiGraph>();
        std::vector<std::unique_ptr<TGraphErrors>> graphs;

        // Process each configuration
        int graphIndex = 0;
        for (const auto& config : configs) {
            auto results = ProcessFilePair(config);
            graphs.push_back(std::unique_ptr<TGraphErrors>(
                CreateGraph(results, 29 + graphIndex, kBlack + graphIndex)));
            mg->Add(graphs.back().get());
            graphIndex++;
        }

        // Create and configure canvas
        auto canvas = std::make_unique<TCanvas>("c1", "Graph", 200, 50, 1200, 600);
        
        mg->Draw("AP");
        mg->SetTitle("Mean of discriminator difference against Muon time exposure;"
                    "Time (Days);#mu_{d}/error");
        mg->GetYaxis()->SetRangeUser(-2, 6);
        mg->GetYaxis()->SetTitleSize(0.05);
        mg->GetXaxis()->SetTitleSize(0.05);
        mg->GetYaxis()->SetTitleOffset(0.70);
        mg->GetXaxis()->SetTitleOffset(0.80);

        gPad->BuildLegend();
        
        canvas->Update();
        canvas->GetFrame()->SetLineWidth(1);
        canvas->GetFrame()->SetFillColor(10);
        canvas->Modified();
        
        canvas->SaveAs("WM_Mean_of_discriminator_difference_STE3_dense_4l_against_"
                      "Muon_time_exposure_and_background_9.pdf");
    }
};

void Detection_requiredTime() {
    DetectionTimeAnalyzer analyzer;
    analyzer.Analyze();
}

int main(int argc, char** argv) {
    Detection_requiredTime();
    return 0;
}

