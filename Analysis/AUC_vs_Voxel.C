// M.Mhaidra 28/09/2019 //
// This macro calculates and plots the mean of the discriminator difference

#include "TCanvas.h"
#include "TTree.h"
#include "TH1D.h"
#include "TFile.h"
#include "TLegend.h"
#include "TStyle.h"
#include "TGraph.h"
#include "TMultiGraph.h"
#include "TGraphErrors.h"

struct GraphData {
    double* voxelSize;
    double* auc;
    double* volumeError;
    double* uncertainties;
    int color;
    int n;
};

TGraphErrors* CreateGraph(const GraphData& data) {
    TGraphErrors* graph = new TGraphErrors(data.n, data.voxelSize, data.auc, 
                                         data.volumeError, data.uncertainties);
    graph->SetMarkerStyle(8);
    graph->SetMarkerSize(1.0);
    graph->SetMarkerColor(data.color);
    graph->SetLineColor(data.color);
    return graph;
}

void SaveMultiGraph(TMultiGraph* mg, const char* filename) {
    TCanvas* canvas = new TCanvas("c1", "Graph", 200, 50, 1200, 600);
    mg->Draw("APC");
    mg->SetTitle("AUC against the voxel size; Voxel Size (cm) ; AUC");
    canvas->Update();
    canvas->Modified();
    canvas->SaveAs(filename, "pdf");
    delete canvas;
}

void AUC_vs_Voxel() {
    const int n = 5;

    // First dataset
    double voxelsize_1[n] = {1, 2, 3, 4, 5};
    double auc_1[n] = {0.538, 0.743, 0.913, 0.851, 0.872};
    double e_volume_1[n] = {0};
    double e_relative_uncertainty_1[n] = {0.011859, 0.0148246, 0.0235481, 
                                        0.025452, 0.0317845};

    // Second dataset
    double voxelsize_2[n] = {1, 2, 3, 4, 5};
    double auc_2[n] = {0.515, 0.724, 0.790, 0.784, 0.646};
    double e_volume_2[n] = {0};
    double e_relative_uncertainty_2[n] = {0.0187056, 0.0164327, 0.0231813, 
                                        0.0224707, 0.0426023};

    // Create graphs using structured data
    GraphData data1 = {voxelsize_1, auc_1, e_volume_1, e_relative_uncertainty_1, 
                      kRed, n};
    GraphData data2 = {voxelsize_2, auc_2, e_volume_2, e_relative_uncertainty_2, 
                      kBlack, n};

    TGraphErrors* gr1 = CreateGraph(data1);
    TGraphErrors* gr2 = CreateGraph(data2);

    // Create and configure multigraph
    TMultiGraph* mg = new TMultiGraph();
    mg->Add(gr1);
    mg->Add(gr2);

    // Save the result
    SaveMultiGraph(mg, "AUC_vs_Voxel_withErrors.pdf");

    // Cleanup
    delete mg; // This also deletes gr1 and gr2
}

int main(int argc, char** argv) {
    AUC_vs_Voxel();
    return 0;
}

