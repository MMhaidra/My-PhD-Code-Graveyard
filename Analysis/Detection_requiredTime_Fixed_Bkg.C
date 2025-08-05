// M.Mhaidra 07/06/2020 //
// Reorganized by GitHub Copilot, 2025

#include <memory>
#include <iostream>
#include <vector>
#include "TCanvas.h"
#include "TTree.h"
#include "TH1D.h"
#include "TFile.h"
#include "TGraph.h"
#include "TMultiGraph.h"
#include "TGraphErrors.h"
#include "TLegend.h"
#include "TStyle.h"

class ExposureTimeAnalyzer {
private:
    struct DataConfig {
        TString basePath;
        TString referencePath;
        int nTracks;
        int markerStyle;
        int markerColor;
        const char* title;
    };

    struct HistogramResult {
        double mean;
        double error;
    };

    static constexpr int nPoints = 7;
    std::vector<double> timePoints = {3, 6, 10, 15, 20, 25, 30};
    std::vector<double> errorTime = std::vector<double>(nPoints, 0);
    std::vector<double> errorDiff = std::vector<double>(nPoints, 0);

    HistogramResult ProcessHistogram(const char* filename) {
        std::unique_ptr<TFile> file(new TFile(filename, "READ"));
        if (!file || file->IsZombie()) return {0, 0};

        TTree* tree = (TTree*)file->Get("T");
        if (!tree) return {0, 0};

        auto hist = std::make_unique<TH1D>("hist", "hist", 200, 0, 140);
        tree->Draw("discr>>hist", "((z*z)+(y*y)<(300*300))&&(x>-480)&&(x<480)", "goff");
        
        hist->SetDirectory(0);
        hist->GetXaxis()->SetRangeUser(5.0, 15.0);

        return {hist->GetMean(), hist->GetMeanError()};
    }

    std::vector<double> AnalyzeTrackData(const DataConfig& config) {
        std::vector<double> results(nPoints);
        
        for (int i = 1; i < nPoints; i++) {
            TString currentFile = config.basePath + Form("%d", i+1) + ".discriminator.root";
            auto result1 = ProcessHistogram(currentFile.Data());
            auto result2 = ProcessHistogram(config.referencePath.Data());
            
            results[i] = std::abs((result1.mean - result2.mean) / 
                                (result1.error + result2.error));
        }
        
        return results;
    }

    TGraph* CreateGraph(const std::vector<double>& data, const DataConfig& config) {
        auto graph = new TGraphErrors(nPoints, timePoints.data(), data.data(), 
                                    errorTime.data(), errorDiff.data());
        graph->SetMarkerStyle(config.markerStyle);
        graph->SetMarkerSize(1.2);
        graph->SetMarkerColor(config.markerColor);
        graph->SetTitle(config.title);
        return graph;
    }

public:
    void Analyze() {
        std::vector<DataConfig> configs = {
            {"/home/mmhaidra/SliceMethod/Exposure_time_study/largedrum_bub_1l_STE3_dense_tracks5_",
             "/home/mmhaidra/SliceMethod/Exposure_time_study_BKG/largedrum_OnlyBitumen_1_STE3_dense_tracks5_7",
             5, 21, kMagenta, "1l Hydrogen bubble (Ntracks=5)"},
            
            {"/home/mmhaidra/SliceMethod/Exposure_time_study/largedrum_OnlyBitumen_STE3_dense_tracks5_",
             "/home/mmhaidra/SliceMethod/Exposure_time_study_BKG/largedrum_OnlyBitumen_9_STE3_dense_tracks5_7",
             5, 28, kMagenta, "Bitumen only (Ntracks=5)"}
        };

        auto mg = std::make_unique<TMultiGraph>();
        
        for (const auto& config : configs) {
            auto results = AnalyzeTrackData(config);
            mg->Add(CreateGraph(results, config));
        }

        auto canvas = std::make_unique<TCanvas>("c1", "Graph", 200, 50, 1200, 600);
        
        mg->Draw("AP");
        mg->SetTitle("Mean of discriminator difference against Muon time exposure;"
                    "Time (Days);Signal_{Hydrogen}");
        mg->GetYaxis()->SetRangeUser(-5, 400);
        mg->GetYaxis()->SetTitleSize(0.05);
        mg->GetXaxis()->SetTitleSize(0.05);
        mg->GetYaxis()->SetTitleOffset(0.70);
        mg->GetXaxis()->SetTitleOffset(0.80);

        gPad->BuildLegend();
        
        canvas->Update();
        canvas->GetFrame()->SetLineWidth(1);
        canvas->GetFrame()->SetFillColor(10);
        canvas->Modified();
        
        canvas->SaveAs("Mean_of_discriminator_difference_STE3_dense_1l_against_"
                      "Muon_time_exposure_and_30days_BKG.pdf", "pdf");
    }
};

void Detection_requiredTime_Fixed_Bkg() {
    ExposureTimeAnalyzer analyzer;
    analyzer.Analyze();
}

int main(int argc, char** argv) {
    Detection_requiredTime_Fixed_Bkg();
    return 0;
}

