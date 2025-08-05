// M.Mhaidra 22/05/2020


#include <TCanvas.h>
#include <TFile.h>
#include <TH1D.h>
#include <TH3F.h>
#include <TLegend.h>
#include <TPad.h>
#include <TStyle.h>
#include <string>
#include <vector>

// Helper struct to hold histogram data
struct HistogramData {
    TH3F* hist;
    std::string name;
    int color;
};

// Helper functions
TH3F* LoadHistogram(const char* filepath, const char* histname) {
    TFile f(filepath, "READ");
    TH3F* h = (TH3F*)f.Get(histname);
    if (h) h->SetDirectory(0);
    return h;
}

void ProcessHistogramDifference(TH3F* hist1, TH3F* hist2, TH3F* diffHist, TH1D* diffHist1D) {
    diffHist->Reset();
    diffHist1D->Reset();
    
    for (int i = hist1->GetXaxis()->FindBin(-440); i < hist1->GetXaxis()->FindBin(440); ++i) {
        for (int j = hist1->GetYaxis()->FindBin(-300); j < hist1->GetYaxis()->FindBin(300); ++j) {
            for (int k = hist1->GetZaxis()->FindBin(-300); k < hist1->GetZaxis()->FindBin(300); ++k) {
                if (hist1->GetBinContent(i,j,k) == 0. || hist2->GetBinContent(i,j,k) == 0.) continue;
                
                double diff = hist1->GetBinContent(i,j,k) - hist2->GetBinContent(i,j,k);
                diffHist->SetBinContent(i,j,k, diff);
                diffHist1D->Fill(diff);
            }
        }
    }
}

void SetupRatioPlot(TH1D* ratio, const char* ytitle) {
    ratio->SetTitle("");
    ratio->SetMinimum(0.95);
    ratio->SetMaximum(1.1);
    ratio->SetStats(0);
    
    // Y axis settings
    ratio->GetYaxis()->SetTitle(ytitle);
    ratio->GetYaxis()->SetNdivisions(505);
    ratio->GetYaxis()->SetTitleSize(20);
    ratio->GetYaxis()->SetTitleFont(43);
    ratio->GetYaxis()->SetTitleOffset(1.55);
    ratio->GetYaxis()->SetLabelFont(43);
    ratio->GetYaxis()->SetLabelSize(15);
    
    // X axis settings
    ratio->GetXaxis()->SetTitleSize(20);
    ratio->GetXaxis()->SetTitleFont(43);
    ratio->GetXaxis()->SetTitleOffset(4.);
    ratio->GetXaxis()->SetLabelFont(43);
    ratio->GetXaxis()->SetLabelSize(15);
}

void Acumulated() {
    // File paths
    const char* signalPath = "/home/mmhaidra/SliceMethod/Exposure_time_study/largedrum_bub_1l_STE3_dense_tracks5_";
    const char* bkgPath = "/home/mmhaidra/SliceMethod/Exposure_time_study_BKG/largedrum_OnlyBitumen_9_STE3_dense_tracks5_";
    
    // Create histograms
    TH1D* h_diff1 = new TH1D("h_diff1", "Acumulative_discr", 200, 0, 5);
    TH1D* h_diff2 = new TH1D("h_diff2", "Acumulative_discr", 200, 0, 5);
    
    // Load and process histograms for time points 2 and 3
    for (int i = 1; i < 3; ++i) {
        TH3F* hist_signal = LoadHistogram(Form("%s%d.discriminator.root", signalPath, i+1), "histBS");
        TH3F* hist_bkg = LoadHistogram(Form("%s%d.discriminator.root", bkgPath, i+1), "histBS");
        
        TH3F* h3_diff = (TH3F*)hist_signal->Clone(Form("h3_diff%d", i));
        ProcessHistogramDifference(hist_signal, hist_bkg, h3_diff, i == 1 ? h_diff1 : h_diff2);
        
        delete hist_signal;
        delete hist_bkg;
        delete h3_diff;
    }
    
    // Create cumulative distributions
    TH1* h1_cumulative = h_diff1->GetCumulative(kFALSE);
    TH1* h2_cumulative = h_diff2->GetCumulative(kFALSE);
    
    h1_cumulative->Scale(100./h1_cumulative->GetBinContent(1));
    h2_cumulative->Scale(100./h2_cumulative->GetBinContent(1));
    
    // Setup plotting
    gStyle->SetOptStat(0);
    TCanvas* can = new TCanvas("c1", "Graph", 200, 50, 1200, 600);
    
    // Create upper pad
    TPad* pad1 = new TPad("pad1", "pad1", 0, 0.3, 1, 1.0);
    pad1->SetBottomMargin(0);
    pad1->Draw();
    pad1->cd();
    
    // Draw cumulative plots
    h1_cumulative->SetLineColor(kBlue+1);
    h1_cumulative->SetLineWidth(1);
    h1_cumulative->Draw();
    
    h2_cumulative->SetLineColor(kRed);
    h2_cumulative->SetLineWidth(1);
    h2_cumulative->Draw("same");
    
    // Create and setup ratio plot
    can->cd();
    TPad* pad2 = new TPad("pad2", "pad2", 0, 0.05, 1, 0.3);
    pad2->SetTopMargin(0);
    pad2->SetBottomMargin(0.25);
    pad2->Draw();
    pad2->cd();
    
    TH1D* ratio = (TH1D*)h1_cumulative->Clone("ratio");
    ratio->Divide(h2_cumulative);
    SetupRatioPlot(ratio, "Signal/Bkg");
    ratio->Draw("ep");
    
    // Save plot
    can->SaveAs("Acumulated_1L_6days.pdf", "pdf");
    
    // Cleanup
    delete h_diff1;
    delete h_diff2;
    delete can;
}

int main(int argc, char** argv) {
    Acumulated();
    return 0;
}

