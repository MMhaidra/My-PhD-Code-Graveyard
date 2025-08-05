// M.Mhaidra 04/10/2021 //


#include <algorithm>
#include <vector>
#include <memory>
#include <iostream>
#include "TCanvas.h"
#include "TH3F.h"
#include "TH1D.h"
#include "TFile.h"
#include "TStyle.h"

class ClusterAnalyzer {
private:
    struct VoxelData {
        std::vector<double> neighborCounts;
        std::vector<double> neighborCountsBelow;
        std::vector<double> maxValues;
        double medianCut;
    };

    TH3F* hist3D_;
    TH1D* hNeighborCount_;
    TH1D* hNeighborCountBelow_;
    VoxelData voxelData_;

    void InitializeHistograms() {
        hNeighborCount_ = new TH1D("h_NeighbourVoxelCount", "NeighbourVoxelCount", 
                                  27, -0.5, 26.5);
        hNeighborCount_->SetTitle("Neighbour voxels count, central voxel is Hydrogen-like for 0.7L bubble");
        hNeighborCount_->GetXaxis()->SetTitle("Neighbour voxels count");
        hNeighborCount_->GetYaxis()->SetTitle("Entries");

        hNeighborCountBelow_ = new TH1D("h_NeighbourVoxelCountBelow", "NeighbourVoxelCountBelow", 
                                       27, -0.5, 26.5);
        hNeighborCountBelow_->SetTitle("Neighbour voxels count, central voxel is Hydrogen-like for 0.7L bubble");
        hNeighborCountBelow_->GetXaxis()->SetTitle("Neighbour voxels count");
        hNeighborCountBelow_->GetYaxis()->SetTitle("Entries");
    }

    int CountNeighbors(int i, int j, int k, bool countAboveCut) {
        int count = 0;
        for (int a = -1; a <= 1; a++) {
            for (int b = -1; b <= 1; b++) {
                for (int c = -1; c <= 1; c++) {
                    double value = hist3D_->GetBinContent(i+a, j+b, k+c);
                    if (countAboveCut ? (value >= voxelData_.medianCut) : 
                                      (value < voxelData_.medianCut)) {
                        count++;
                    }
                }
            }
        }
        return count;
    }

    void ProcessVoxels() {
        for (int i = hist3D_->GetXaxis()->FindBin(-400); 
             i < hist3D_->GetXaxis()->FindBin(-300); i++) {
            for (int j = hist3D_->GetYaxis()->FindBin(-115); 
                 j < hist3D_->GetYaxis()->FindBin(115); j++) {
                for (int k = hist3D_->GetZaxis()->FindBin(-160); 
                     k < hist3D_->GetZaxis()->FindBin(160); k++) {
                    
                    double voxelValue = hist3D_->GetBinContent(i, j, k);
                    if (voxelValue >= voxelData_.medianCut) {
                        int neighborsAbove = CountNeighbors(i, j, k, true);
                        int neighborsBelow = CountNeighbors(i, j, k, false);
                        
                        hNeighborCount_->Fill(neighborsAbove);
                        hNeighborCountBelow_->Fill(neighborsBelow);
                    }
                }
            }
        }
    }

    void SaveResults() {
        gStyle->SetOptStat(0);
        std::unique_ptr<TCanvas> canvas(new TCanvas("c_test2",
            "Neighbour voxels count, central voxel is Hydrogen-like for 0.7L bubble",
            200, 100, 1400, 700));
            
        hNeighborCount_->SetLineColor(kRed);
        hNeighborCountBelow_->SetLineColor(kBlack);
        
        hNeighborCountBelow_->Draw();
        hNeighborCount_->Draw("same");
        
        canvas->SaveAs("/home/mmhaidra/SliceMethod/results_Roc_Slices_May2021/"
                      "Clusters_3_NeighbourVoxelCount_0.7L(6Cubes)_STE3_MedianCut_40_30_Test.pdf",
                      "pdf");
    }

public:
    ClusterAnalyzer() : hist3D_(nullptr), hNeighborCount_(nullptr), 
                        hNeighborCountBelow_(nullptr) {
        voxelData_.medianCut = 11.17;
    }
    
    ~ClusterAnalyzer() {
        delete hist3D_;
        delete hNeighborCount_;
        delete hNeighborCountBelow_;
    }

    bool Initialize(const char* filename) {
        std::unique_ptr<TFile> file(new TFile(filename, "READ"));
        if (!file || file->IsZombie()) return false;
        
        hist3D_ = (TH3F*)file->Get("histMedianMetric");
        if (!hist3D_) return false;
        
        hist3D_->SetDirectory(0);
        InitializeHistograms();
        return true;
    }

    void Analyze() {
        ProcessVoxels();
        SaveResults();
    }
};

void Cluster() {
    ClusterAnalyzer analyzer;
    
    if (!analyzer.Initialize("/home/mmhaidra/SliceMethod/largedrum_0.7L_6Cubes_dense_"
                           "Aligned_3cmVoxel_May2021.discriminator.root")) {
        std::cerr << "Failed to initialize analyzer" << std::endl;
        return;
    }
    
    analyzer.Analyze();
}

int main(int argc, char** argv) {
    Cluster();
    return 0;
}
