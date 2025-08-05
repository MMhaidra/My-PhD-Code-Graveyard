

#include <TFile.h>
#include <TString.h>
#include <TH1D.h>
#include <TCanvas.h>
#include <TLegend.h>

struct EfficiencyMetrics {
    double efficiency;
    double purity;
    double signalIntegral;
    double backgroundIntegral;
};

EfficiencyMetrics CalculateEfficiencyPurity(TH1D* hSignal, TH1D* hBackground, int binCut) {
    EfficiencyMetrics metrics;
    int nBins = hSignal->GetNbinsX();
    
    metrics.signalIntegral = hSignal->Integral(binCut, nBins);
    metrics.backgroundIntegral = hBackground->Integral(binCut, nBins);
    
    double totalSignal = hSignal->Integral();
    metrics.efficiency = metrics.signalIntegral / totalSignal;
    metrics.purity = metrics.signalIntegral / (metrics.signalIntegral + metrics.backgroundIntegral);
    
    return metrics;
}

void SetupHistogram(TH1D* hist, const char* title, int color, bool isPurity) {
    hist->SetLineColor(color);
    hist->SetTitle(title);
    hist->GetXaxis()->SetTitle("cut on mean metric");
    hist->GetYaxis()->SetTitle("Efficiency / Purity");
    if (isPurity) hist->SetMaximum(1.2);
}

void calcEffPurity(TString inFileName = "Median_signal_background_4cmvoxel.root") {
    // Load histograms
    TFile* fIn = new TFile(inFileName);
    TH1D* hSignal = (TH1D*)fIn->Get("h_discr1");
    TH1D* hBg = (TH1D*)fIn->Get("h_discr2");

    // Create efficiency and purity histograms
    TH1D* hEff = (TH1D*)hSignal->Clone("hEff");
    TH1D* hPurity = (TH1D*)hSignal->Clone("hPurity");
    hEff->Reset();
    hPurity->Reset();

    // Calculate efficiency and purity for each bin
    for(int bin = 1; bin < hSignal->GetNbinsX(); bin++) {
        EfficiencyMetrics metrics = CalculateEfficiencyPurity(hSignal, hBg, bin);
        hEff->SetBinContent(bin, metrics.efficiency);
        hPurity->SetBinContent(bin, metrics.purity);
    }

    // Setup plotting
    SetupHistogram(hPurity, "Purity", kBlue, true);
    SetupHistogram(hEff, "Efficiency", kRed, false);
    hSignal->SetTitle("Signal");
    hBg->SetTitle("Background");

    // Create canvas and draw
    TCanvas* c1 = new TCanvas("c","eff",800,1000);
    c1->Divide(1,2);
    
    c1->cd(2);
    hPurity->Draw();
    hEff->Draw("same");
    c1->cd(2)->BuildLegend();
    
    c1->cd(1);
    hSignal->Draw();
    hBg->Draw("same");
    c1->cd(1)->BuildLegend();

    c1->SaveAs("A4_test_EFF_Puri.pdf","pdf");

    // Cleanup
    delete c1;
    fIn->Close();
}

int main(int argc, char** argv) {
    calcEffPurity();
    return 0;
}
