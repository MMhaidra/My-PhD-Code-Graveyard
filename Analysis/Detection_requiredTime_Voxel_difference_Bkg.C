// M.Mhaidra 22/05/2020 //
// This macro gives an idea on the minimum time required to detect hydrogen bubble inside bituminized drum.


#include <memory>
#include <vector>
#include <iostream>
#include "TCanvas.h"
#include "TTree.h"
#include "TH1D.h"
#include "TH3F.h"
#include "TFile.h"
#include "TGraph.h"
#include "TMultiGraph.h"
#include "TStyle.h"

class VoxelDifferenceAnalyzer {
private:
    struct FileConfig {
        TString baseDir;
        TString refDir;
        int fileNumber;
        const char* suffix;
    };

    struct HistogramData {
        std::unique_ptr<TH3F> hist3D;
        std::unique_ptr<TH3F> hist3DRef;
        std::unique_ptr<TH1D> diffHist;
        std::unique_ptr<TH3F> diff3D;
    };

    static constexpr int nPoints = 7;
    std::vector<double> timePoints = {3, 6, 10, 15, 20, 25, 30};
    std::vector<double> results;

    // Helper function to load histogram from file
    TH3F* LoadHistogram(const char* filename) {
        std::unique_ptr<TFile> file(new TFile(filename, "READ"));
        if (!file || file->IsZombie()) return nullptr;

        TH3F* hist = (TH3F*)file->Get("histBS");
        if (hist) hist->SetDirectory(0); // Detach from file
        return hist;
    }

    // Process single pair of histograms
    double ProcessHistogramPair(TH3F* hist1, TH3F* hist2, TH3F* diffHist) {
        if (!hist1 || !hist2 || !diffHist) return 0.0;
        
        diffHist->Reset();

        // Iterate through histogram bins
        for (int i = hist1->GetXaxis()->FindBin(-440); i < hist1->GetXaxis()->FindBin(440); i++) {
            for (int j = hist1->GetYaxis()->FindBin(-300); j < hist1->GetYaxis()->FindBin(300); j++) {
                for (int k = hist1->GetZaxis()->FindBin(-300); k < hist1->GetZaxis()->FindBin(300); k++) {
                    // Skip empty bins
                    if (hist1->GetBinContent(i,j,k) == 0. || hist2->GetBinContent(i,j,k) == 0.) 
                        continue;
                    
                    // Calculate difference
                    diffHist->SetBinContent(i,j,k, 
                        hist1->GetBinContent(i,j,k) - hist2->GetBinContent(i,j,k));
                }
            }
        }

        return diffHist->Integral();
    }

    // Process file set for given configuration
    std::vector<double> ProcessFileSet(const FileConfig& config) {
        std::vector<double> results(nPoints);
        
        for (int i = 0; i < nPoints; i++) {
            // Construct file paths
            TString mainFile = config.baseDir + Form("%d%s", i+1, config.suffix);
            TString refFile = config.refDir + Form("%d%s", i+1, config.suffix);
            
            // Load histograms
            std::unique_ptr<TH3F> hist(LoadHistogram(mainFile.Data()));
            std::unique_ptr<TH3F> histRef(LoadHistogram(refFile.Data()));
            std::unique_ptr<TH3F> diffHist(hist ? (TH3F*)hist->Clone("h3_diff") : nullptr);
            
            if (!hist || !histRef || !diffHist) {
                std::cerr << "Failed to load histograms for iteration " << i << std::endl;
                continue;
            }
            
            // Calculate difference
            results[i] = ProcessHistogramPair(hist.get(), histRef.get(), diffHist.get());
            std::cout << "Integral " << config.fileNumber << " is: " << results[i] << std::endl;
        }
        
        return results;
    }

public:
    void Analyze() {
        // Base configuration for all file sets
        const char* suffix = "_STE3_dense_tracks5_.discriminator.root";
        const char* baseDir = "/home/mmhaidra/SliceMethod/Exposure_time_study_BKG/";
        
        std::vector<FileConfig> configs = {
            {baseDir + "largedrum_OnlyBitumen_9_", baseDir + "largedrum_OnlyBitumen_1_", 1, suffix},
            {baseDir + "largedrum_OnlyBitumen_9_", baseDir + "largedrum_OnlyBitumen_2_", 2, suffix},
            // Add more configurations as needed
        };

        std::vector<std::vector<double>> allResults;
        for (const auto& config : configs) {
            allResults.push_back(ProcessFileSet(config));
        }

        // Create and save plots
        PlotResults(allResults);
    }

private:
    void PlotResults(const std::vector<std::vector<double>>& allResults) {
        auto mg = std::make_unique<TMultiGraph>();
        std::vector<std::unique_ptr<TGraphErrors>> graphs;

        // Create graphs for each result set
        for (size_t i = 0; i < allResults.size(); i++) {
            auto graph = std::make_unique<TGraphErrors>(
                nPoints, timePoints.data(), allResults[i].data(),
                std::vector<double>(nPoints, 0).data(),  // x errors
                std::vector<double>(nPoints, 0).data()); // y errors

            graph->SetMarkerStyle(20 + i);
            graph->SetMarkerSize(1.2);
            graph->SetMarkerColor(kBlue + i);
            
            mg->Add(graph.get());
            graphs.push_back(std::move(graph));
        }

        // Create and configure canvas
        auto canvas = std::make_unique<TCanvas>("c1", "Graph", 200, 50, 1200, 600);
        
        mg->Draw("AP");
        mg->SetTitle(";Time (Days);H_{2} Signal");
        mg->GetYaxis()->SetRangeUser(-4, 4000);
        mg->GetYaxis()->SetTitleSize(0.05);
        mg->GetXaxis()->SetTitleSize(0.05);
        mg->GetYaxis()->SetTitleOffset(0.70);
        mg->GetXaxis()->SetTitleOffset(0.80);

        gPad->BuildLegend();
        canvas->Update();
        canvas->GetFrame()->SetLineWidth(1);
        canvas->GetFrame()->SetFillColor(10);
        canvas->Modified();

        canvas->SaveAs("Mean_of_discriminator_difference_STE3_dense_against_Muon_time_exposure.pdf");
    }
};

void Detection_requiredTime_Voxel_difference_Bkg() {
    VoxelDifferenceAnalyzer analyzer;
    analyzer.Analyze();
}

int main(int argc, char** argv) {
    Detection_requiredTime_Voxel_difference_Bkg();
    return 0;
}

