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
#include "TH3F.h"
#include "TFile.h"
#include "TGraph.h"
#include "TMultiGraph.h"
#include "TGraphErrors.h"
#include "TStyle.h"

// Structure to hold file paths configuration
struct FileConfig {
    TString baseDir;
    TString refDir;
    TString suffix;
    const char* title;
    double volume;
};

class VoxelAnalyzer {
private:
    static constexpr int nPoints = 7;
    std::vector<double> timePoints = {3, 6, 10, 15, 20, 25, 30};
    std::vector<double> errorTime = std::vector<double>(nPoints, 0);
    std::vector<double> errorDiff = std::vector<double>(nPoints, 0);

    // Helper function to load and process a histogram pair
    std::vector<double> ProcessHistogramPair(const FileConfig& config) {
        std::vector<double> results(nPoints);
        
        // Create histograms once and reuse
        std::unique_ptr<TH1D> h_diff(new TH1D("h_diff", "Diff_Xbin", 200, -20, 20));
        std::unique_ptr<TH3F> h3_diff;
        bool first = true;

        for (int i = 0; i < nPoints; i++) {
            // Construct file paths
            TString mainFile = config.baseDir + Form("%d", i+1) + config.suffix;
            TString refFile = config.refDir + Form("%d", i+1) + config.suffix;

            // Load histograms with proper cleanup
            auto hist_3D = LoadHistogram(mainFile.Data());
            auto hist_3D_ref = LoadHistogram(refFile.Data());

            if (!hist_3D || !hist_3D_ref) continue;

            // Create difference histogram on first iteration
            if (first) {
                h3_diff.reset((TH3F*)hist_3D->Clone("h3_diff"));
                first = false;
            }

            // Calculate difference between histograms
            results[i] = CalculateDifference(hist_3D.get(), hist_3D_ref.get(), h3_diff.get());
            std::cout << "Volume " << config.volume << "L - Integral " << i 
                     << " is: " << results[i] << std::endl;
        }
        return results;
    }

    // Helper function to load a histogram from file
    std::unique_ptr<TH3F> LoadHistogram(const char* filename) {
        std::unique_ptr<TFile> file(new TFile(filename, "READ"));
        if (!file || file->IsZombie()) return nullptr;

        TH3F* hist = (TH3F*)file->Get("histBS");
        if (hist) hist->SetDirectory(0); // Detach from file
        return std::unique_ptr<TH3F>(hist);
    }

    // Calculate difference between two 3D histograms
    double CalculateDifference(TH3F* hist1, TH3F* hist2, TH3F* diffHist) {
        diffHist->Reset();

        // Iterate through histogram bins
        for (int i = hist1->GetXaxis()->FindBin(-440); i < hist1->GetXaxis()->FindBin(440); i++) {
            for (int j = hist1->GetYaxis()->FindBin(-300); j < hist1->GetYaxis()->FindBin(300); j++) {
                for (int k = hist1->GetZaxis()->FindBin(-300); k < hist1->GetZaxis()->FindBin(300); k++) {
                    // Skip empty bins
                    if (hist1->GetBinContent(i,j,k) == 0. || hist2->GetBinContent(i,j,k) == 0.) 
                        continue;
                    
                    diffHist->SetBinContent(i,j,k, 
                        hist1->GetBinContent(i,j,k) - hist2->GetBinContent(i,j,k));
                }
            }
        }
        return std::abs(diffHist->Integral());
    }

    // Create and configure graph
    TGraphErrors* CreateGraph(const std::vector<double>& data, int style, int color, 
                            const char* title) {
        auto graph = new TGraphErrors(nPoints, timePoints.data(), data.data(), 
                                    errorTime.data(), errorDiff.data());
        graph->SetMarkerStyle(style);
        graph->SetMarkerSize(1.2);
        graph->SetMarkerColor(color);
        graph->SetTitle(title);
        return graph;
    }

public:
    void Analyze() {
        // Configuration for different volumes
        std::vector<FileConfig> configs = {
            {"/home/mmhaidra/SliceMethod/Exposure_time_study/largedrum_bub_1l_STE3_dense_tracks5_",
             "/home/mmhaidra/SliceMethod/Exposure_time_study/largedrum_OnlyBitumen_STE3_dense_tracks5_",
             ".discriminator.root", "1l Hydrogen bubble", 1.0},
            {"/home/mmhaidra/SliceMethod/Exposure_time_study/largedrum_bub_2.95l_STE3_dense_tracks5_",
             "/home/mmhaidra/SliceMethod/Exposure_time_study/largedrum_OnlyBitumen_STE3_dense_tracks5_",
             ".discriminator.root", "2.95l Hydrogen bubble", 2.95},
            {"/home/mmhaidra/SliceMethod/Exposure_time_study/largedrum_bub_0.5l_STE3_dense_tracks5_",
             "/home/mmhaidra/SliceMethod/Exposure_time_study/largedrum_OnlyBitumen_STE3_dense_tracks5_",
             ".discriminator.root", "0.5l Hydrogen bubble", 0.5},
            {"/home/mmhaidra/SliceMethod/Exposure_time_study/largedrum_bub_0.35l_STE3_dense_tracks5_",
             "/home/mmhaidra/SliceMethod/Exposure_time_study/largedrum_OnlyBitumen_STE3_dense_tracks5_",
             ".discriminator.root", "0.35l Hydrogen bubble", 0.35},
            {"/home/mmhaidra/SliceMethod/Exposure_time_study/largedrum_bub_0.25l_STE3_dense_tracks5_",
             "/home/mmhaidra/SliceMethod/Exposure_time_study/largedrum_OnlyBitumen_STE3_dense_tracks5_",
             ".discriminator.root", "0.25l Hydrogen bubble", 0.25},
            // Add more configurations as needed
        };

        auto mg = std::make_unique<TMultiGraph>();
        std::vector<std::unique_ptr<TGraphErrors>> graphs;

        // Process each configuration
        for (const auto& config : configs) {
            auto results = ProcessHistogramPair(config);
            graphs.push_back(std::unique_ptr<TGraphErrors>(
                CreateGraph(results, 21, kMagenta, config.title)));
            mg->Add(graphs.back().get());
        }

        // Create and configure canvas
        auto canvas = std::make_unique<TCanvas>("c1", "Graph", 200, 50, 1200, 600);
        
        mg->Draw("AP");
        mg->SetTitle("; Time (Days) ;H_{2} Signal");
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

void Detection_requiredTime_Voxel_difference() {
    VoxelAnalyzer analyzer;
    analyzer.Analyze();
}

int main(int argc, char** argv) {
    Detection_requiredTime_Voxel_difference();
    return 0;
}

