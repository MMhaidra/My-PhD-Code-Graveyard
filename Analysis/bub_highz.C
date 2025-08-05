// M.Mhaidra


#include "TCanvas.h"
#include "TTree.h"
#include "TH1D.h"
#include "TFile.h"
#include "TRandom.h"
#include "TLine.h"
#include "TPaveText.h"
#include "TLegend.h"
#include "TStyle.h"
#include "TSpectrum.h"
#include "TVirtualFitter.h"
#include "TFrame.h"
#include "Riostream.h"
#include "TCut.h"
#include "TF1.h"
#include "TGraph.h"
#include "TMultiGraph.h"
#include "TGraphErrors.h"

struct GraphData {
    const int size;
    double* volumes;
    double* uncertainties;
    double* volumeErrors;
    double* uncertaintyErrors;
    int color;
};

TGraphErrors* CreateGraph(const GraphData& data) {
    TGraphErrors* graph = new TGraphErrors(
        data.size, 
        data.volumes, 
        data.uncertainties,
        data.volumeErrors, 
        data.uncertaintyErrors
    );
    
    graph->SetMarkerStyle(8);
    graph->SetMarkerSize(1.0);
    graph->SetMarkerColor(data.color);
    
    return graph;
}

void DrawReferenceLine(double xMin, double xMax, double y, int color = 29) {
    TLine* line = new TLine(xMin, y, xMax, y);
    line->SetLineStyle(1);
    line->SetLineColor(color);
    line->SetLineWidth(1);
    line->Draw();
}

void bub_highz() {
    const int n = 4;
    
    // Dataset 1
    double volume_1[n] = {4.42, 6.03, 7.12, 9.42};
    double relative_uncertainty_1[n] = {8.32, 8.81, 9.13, 9.56};
    double e_volume_1[n] = {0};
    double e_relative_uncertainty_1[n] = {0};
    
    // Dataset 2
    double volume_2[n] = {4.42, 6.03, 7.12, 9.42};
    double relative_uncertainty_2[n] = {1.70, 2.45, 2.08, 2.39};
    double e_volume_2[n] = {0};
    double e_relative_uncertainty_2[n] = {0};
    
    // Create graphs using structured data
    GraphData data1 = {n, volume_1, relative_uncertainty_1, 
                      e_volume_1, e_relative_uncertainty_1, kRed};
    GraphData data2 = {n, volume_2, relative_uncertainty_2, 
                      e_volume_2, e_relative_uncertainty_2, kBlack};
    
    TGraphErrors* gr1 = CreateGraph(data1);
    TGraphErrors* gr2 = CreateGraph(data2);
    
    // Create and configure canvas
    TCanvas* c1 = new TCanvas("c1", "Graph", 200, 50, 1200, 600);
    
    // Create and configure multigraph
    TMultiGraph* mg = new TMultiGraph();
    mg->Add(gr1);
    mg->Add(gr2);
    mg->Draw("AP");
    mg->SetTitle("The reconstructed volume shift after adding high/low Z materials ; True gas volume (L) ;(V_{rec}-V_{true})");
    
    // Draw reference lines
    DrawReferenceLine(4.2, 9.65, 8.95);
    DrawReferenceLine(4.2, 9.65, 2.15);
    
    // Update and save
    c1->Update();
    c1->Modified();
    c1->SaveAs("TheRecoVshift_Against_TrueVolume.pdf", "pdf");
    
    // Cleanup
    delete c1; // This also deletes the multigraph and its contents
}

int main(int argc, char** argv) {
    bub_highz();
    return 0;
}

