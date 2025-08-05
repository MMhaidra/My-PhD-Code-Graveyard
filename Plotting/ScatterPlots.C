/* 
 * M.Mhaidra 2020/11/25
 * This macro analyzes and compares median metric distributions between
 * bitumen and hydrogen regions in drums at different radial distances.
 * It creates scatter plots and histograms to visualize the spatial
 * distribution of measurements.
 */

#include <iostream>
#include <memory>
#include <vector>
#include <cmath>

// ROOT core and data handling
#include "TFile.h"
#include "TH1D.h"
#include "TH3F.h"
#include "TTree.h"
// ROOT graphics and visualization
#include "TCanvas.h"
#include "TGraph.h"
#include "TMultiGraph.h"
#include "TLine.h"
#include "TStyle.h"
#include "TLegend.h"

class ScatterPlotAnalyzer {
private:
    struct AnalysisConfig {
        // Geometric parameters (mm)
        double dX = 30;  // 30mm voxel size
        double dY = 30;
        double dZ = 30;
        double cylinderRadius = 262;
        double medianCut = 11.265;
        
        // Histogram parameters
        int nBins = 100;
        double histMin = 9.5;
        double histMax = 15.0;
        
        // Analysis arrays size
        static constexpr int arraySize = 100000;
    };

    struct HistogramCollection {
        std::vector<std::unique_ptr<TH1D>> bitumenHists;
        std::vector<std::unique_ptr<TH1D>> hydrogenHists;
        
        // Create histograms for each radial distance
        void InitializeHistograms(const std::vector<double>& distances, 
                                const AnalysisConfig& config) {
            for (const auto& dist : distances) {
                auto name = Form("h_histMedianMetricBitumen_%.0f", dist);
                bitumenHists.push_back(std::make_unique<TH1D>(name, name, 
                    config.nBins, config.histMin, config.histMax));
                
                name = Form("h_histMedianMetricHydrogen_%.0f", dist);
                hydrogenHists.push_back(std::make_unique<TH1D>(name, name, 
                    config.nBins, config.histMin, config.histMax));
            }
        }
    };

    // Process 3D histogram with geometric cuts
    void ProcessHistogram(TH3F* hist, double* values, double* distances, 
                         bool isHydrogen, const AnalysisConfig& config) {
        double nbinsXCenter = hist->GetNbinsX()/2;
        double nbinsYCenter = hist->GetNbinsY()/2;
        double nbinsZCenter = hist->GetNbinsZ()/2;
        
        int idx = 0;
        // Iterate through histogram bins with geometric cuts
        for (int i = hist->GetXaxis()->FindBin(-40); 
             i < hist->GetXaxis()->FindBin(40); i++) {
            for (int j = hist->GetYaxis()->FindBin(-40); 
                 j < hist->GetYaxis()->FindBin(40); j++) {
                for (int k = hist->GetZaxis()->FindBin(-40); 
                     k < hist->GetZaxis()->FindBin(40); k++) {
                    
                    // Calculate distances from center
                    double y = (j-nbinsYCenter) * config.dY;
                    double z = (k-nbinsZCenter) * config.dZ;
                    double x = (i-nbinsXCenter) * config.dX;
                    
                    // Apply cylindrical geometric cut
                    if (pow(y, 2.0) + pow(z, 2.0) < pow(config.cylinderRadius, 2.0) && 
                        x > -400 && x <= 400) {
                        
                        double value = hist->GetBinContent(i,j,k);
                        if (isHydrogen && value < config.medianCut) continue;
                        
                        values[idx] = value;
                        distances[idx] = sqrt(pow(y, 2.0) + pow(z, 2.0));
                        idx++;
                    }
                }
            }
        }
    }

public:
    void Analyze() {
        AnalysisConfig config;
        
        // Load histograms
        auto file1 = std::make_unique<TFile>("/home/mmhaidra/SliceMethod/largedrum_onlybitumen_dense_newmetrics_3cmVoxel_April2021.discriminator.root", "READ");
        auto file2 = std::make_unique<TFile>("/home/mmhaidra/SliceMethod/largedrum_0.5L_8Cubes_dense_newmetrics_3cmVoxel_April2021.discriminator.root", "READ");
        
        auto hist3D = std::unique_ptr<TH3F>((TH3F*)file1->Get("histMedianMetric"));
        auto hist3D_H2 = std::unique_ptr<TH3F>((TH3F*)file2->Get("histMedianMetric"));
        
        if (!hist3D || !hist3D_H2) {
            std::cerr << "Failed to load histograms" << std::endl;
            return;
        }
        
        // Detach histograms from files
        hist3D->SetDirectory(0);
        hist3D_H2->SetDirectory(0);
        
        // Initialize analysis arrays
        auto medianValues = std::make_unique<double[]>(config.arraySize);
        auto medianValuesH2 = std::make_unique<double[]>(config.arraySize);
        auto distances = std::make_unique<double[]>(config.arraySize);
        auto distancesH2 = std::make_unique<double[]>(config.arraySize);
        
        // Process histograms
        ProcessHistogram(hist3D.get(), medianValues.get(), distances.get(), 
                        false, config);
        ProcessHistogram(hist3D_H2.get(), medianValuesH2.get(), distancesH2.get(), 
                        true, config);
        
        // Create and configure graphs
        auto graph1 = std::make_unique<TGraph>(config.arraySize, medianValues.get(), 
                                             distances.get());
        graph1->SetMarkerStyle(1);
        graph1->SetMarkerColor(kBlack);
        
        auto graph2 = std::make_unique<TGraph>(config.arraySize, medianValuesH2.get(), 
                                             distancesH2.get());
        graph2->SetMarkerStyle(1);
        graph2->SetMarkerColor(kMagenta);
        
        // Create multigraph
        auto mg = std::make_unique<TMultiGraph>();
        mg->Add(graph1.get());
        mg->Add(graph2.get());
        
        // Create canvas and draw
        auto canvas = std::make_unique<TCanvas>("c1", "Graph", 200, 50, 1200, 600);
        mg->Draw("AP");
        mg->SetTitle("Voxel distance from centre of drum vs median metric;"
                    "Log (Median) of metric;Distance from centre [mm]");
        
        // Add reference line
        auto line = std::make_unique<TLine>(config.medianCut, 0, 
                                          config.medianCut, config.cylinderRadius);
        line->SetLineStyle(1);
        line->SetLineColor(29);
        line->Draw();
        
        canvas->Update();
        canvas->SaveAs("Median_distance_central_axis_3CubesHydrogenVSBitumen_CylinderCut_400_262.pdf");
    }
};

void ScatterPlots() {
    ScatterPlotAnalyzer analyzer;
    analyzer.Analyze();
}

int main(int argc, char** argv) {
    ScatterPlots();
    return 0;
}
