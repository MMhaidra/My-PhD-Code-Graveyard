// M.Mhaidra 18/09/2019 //


#include "TCanvas.h"
#include "TTree.h"
#include "TH1D.h"
#include "TH2D.h"
#include "TH3D.h"
#include "TCutG.h"
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
#include <algorithm>
#include <iterator>
#include <vector>
#include <numeric>
#include <iomanip>

struct SliceData {
    std::vector<double> centers;
    std::vector<double> means;
    std::vector<double> errors;
};

// Helper functions
TH3D* LoadHistogram(const char* filename, const char* histname) {
    TFile* file = new TFile(filename, "READ");
    if (!file) return nullptr;
    
    TH3D* hist = (TH3D*)file->Get(histname);
    if (hist) hist->SetDirectory(0);
    
    file->Close();
    delete file;
    return hist;
}

void ProcessSlice(TH3D* hist, TH1D* projHist, double xLow, double xHigh) {
    projHist->Reset();
    
    for (int i = hist->GetXaxis()->FindBin(xLow); i < hist->GetXaxis()->FindBin(xHigh); i++) {
        for (int j = hist->GetYaxis()->FindBin(-100); j < hist->GetYaxis()->FindBin(100); j++) {
            for (int k = hist->GetZaxis()->FindBin(-100); k < hist->GetZaxis()->FindBin(100); k++) {
                projHist->Fill(hist->GetBinContent(i,j,k));
            }
        }
    }
    
    projHist->GetXaxis()->SetRangeUser(7.0, 14.0);
    projHist->SetMarkerStyle(21);
    projHist->SetMarkerColor(kBlue+8);
}

TGraphErrors* CreateGraph(const SliceData& data) {
    int n = data.centers.size();
    TGraphErrors* graph = new TGraphErrors(n, 
        data.centers.data(), data.means.data(),
        std::vector<double>(n, 0).data(), data.errors.data());
    
    graph->SetFillColor(4);
    graph->SetMarkerStyle(8);
    graph->SetMarkerSize(1.2);
    graph->SetMarkerColor(kBlue);
    
    return graph;
}

void slices_plot_discr_difference() {
    // Configuration
    const double x_start = -380;
    const double x_finish = 380;
    const double slice_width = 40;
    const int n_slices = (x_finish - x_start) / slice_width;
    
    // Load histograms
    TH3D* hist_3D = LoadHistogram("/home/mmhaidra/SliceMethod/hist3D_After_removing_highZ_Materials_STE3_hardCuts.root", "histBS");
    TH3D* hist_3D_reference = LoadHistogram("/home/mmhaidra/SliceMethod/Chance_largedrum_bub_tracks35_discr_STE3/largedrum_bub_STE3_OnlyBitumen_ForVoxelsCleaning_tracks35.discriminator.root", "histBS");
    
    if (!hist_3D || !hist_3D_reference) {
        std::cerr << "Failed to load histograms" << std::endl;
        return;
    }
    
    // Create projection histograms
    TH1D* hx1 = new TH1D("hx1", "BC_Xrange", 2000, 0, 140);
    TH1D* hx2 = new TH1D("hx2", "BC_Xrange", 2000, 0, 140);
    
    // Process slices
    SliceData sliceData;
    double x_high = x_start + slice_width;
    
    for (double x_low = x_start; x_low < x_finish; x_low += slice_width) {
        std::cout << "Range is " << x_low << " to " << x_high << "\n";
        
        sliceData.centers.push_back(0.5 * (x_low + x_high));
        
        // Process both histograms
        ProcessSlice(hist_3D, hx1, x_low, x_high);
        ProcessSlice(hist_3D_reference, hx2, x_low, x_high);
        
        // Calculate means and errors
        double mean1 = hx1->GetMean();
        double error1 = hx1->GetMeanError();
        double mean2 = hx2->GetMean();
        double error2 = hx2->GetMeanError();
        
        double meanDiff = (mean1 - mean2) / (error1 + error2);
        sliceData.means.push_back(meanDiff);
        sliceData.errors.push_back(error1 + error2);
        
        x_high += slice_width;
    }
    
    // Create and configure graph
    TGraphErrors* gr1 = CreateGraph(sliceData);
    
    // Create and configure canvas
    TCanvas* c2 = new TCanvas("c2", "Graph", 200, 100, 1300, 700);
    c2->SetFillColor(0);
    
    // Create and configure multigraph
    TMultiGraph* mg_YZ = new TMultiGraph();
    mg_YZ->Add(gr1);
    mg_YZ->Draw("AB");
    mg_YZ->SetTitle("#mu_{discr_difference} in drum slices;YZ plane (mm);#mu_{discr_difference}/error");
    mg_YZ->GetYaxis()->SetRangeUser(-20, 20);
    mg_YZ->GetYaxis()->SetTitleSize(0.05);
    mg_YZ->GetXaxis()->SetTitleSize(0.05);
    mg_YZ->GetYaxis()->SetTitleOffset(0.70);
    mg_YZ->GetXaxis()->SetTitleOffset(0.80);
    
    // Update and save
    c2->Update();
    c2->GetFrame()->SetLineWidth(1);
    c2->GetFrame()->SetFillColor(10);
    c2->Modified();
    c2->SaveAs("Slice_plotDiscr_4l_Ucube_CleanedVoxels_hardcuts9.2_STE3_YZslice_35tracks_40steps.pdf", "pdf");
    
    // Cleanup
    delete hist_3D;
    delete hist_3D_reference;
    delete hx1;
    delete hx2;
    delete c2;
}

int main(int argc, char** argv) {
    slices_plot_discr_difference();
    return 0;
}
