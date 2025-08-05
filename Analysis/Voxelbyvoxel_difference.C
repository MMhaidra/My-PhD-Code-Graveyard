// M.Mhaidra 10/09/2019 //
// Cleaned and reorganized by GitHub Copilot, 2025

#include "TCanvas.h"
#include "TTree.h"
#include "TH3F.h"
#include "TH1D.h"
#include "TFile.h"
#include "TStyle.h"
#include <vector>
#include <string>

struct HistSet {
    TH3F* hist;
    std::string name;
    std::string color;
};

TH3F* LoadTH3F(const char* file, const char* histname) {
    TFile f(file, "READ");
    TH3F* h = (TH3F*)f.Get(histname);
    if (h) h->SetDirectory(0);
    return h;
}

void ComputeVoxelDiff(TH3F* h1, TH3F* h2, TH3F* hdiff, TH1D* h1d, bool invert = false) {
    hdiff->Reset();
    h1d->Reset();
    for (int i = h1->GetXaxis()->FindBin(-440); i < h1->GetXaxis()->FindBin(440); ++i) {
        for (int j = h1->GetYaxis()->FindBin(-300); j < h1->GetYaxis()->FindBin(300); ++j) {
            for (int k = h1->GetZaxis()->FindBin(-300); k < h1->GetZaxis()->FindBin(300); ++k) {
                if (h1->GetBinContent(i, j, k) == 0. || h2->GetBinContent(i, j, k) == 0.) continue;
                double diff = h1->GetBinContent(i, j, k) - h2->GetBinContent(i, j, k);
                hdiff->SetBinContent(i, j, k, diff);
                h1d->Fill(invert ? -diff : diff);
            }
        }
    }
}

void DrawAndSaveHist(TH1D* h, const char* fname, int color, const char* xtitle, const char* ytitle) {
    h->SetLineColor(color);
    h->GetXaxis()->SetTitle(xtitle);
    h->GetYaxis()->SetTitle(ytitle);
    TCanvas* c = new TCanvas();
    h->Draw();
    c->SaveAs(fname, "pdf");
    delete c;
}

void Voxelbyvoxel_difference() {
    // File paths and colors
    const char* files[] = {
        "/home/mmhaidra/SliceMethod/Chance_largedrum_bub_tracks35_discr_STE3_dense/largedrum_bub_20.70l_STE3_dense_tracks35.discriminator.root",
        "/home/mmhaidra/SliceMethod/Chance_largedrum_bub_tracks35_discr_STE3/largedrum_bub_STE3_OnlyBitumen_ForVoxelsCleaning_tracks35.discriminator.root",
        "/home/mmhaidra/SliceMethod/Chance_largedrum_bub_tracks35_discr_STE3_dense/largedrum_bub_17.19l_STE3_dense_tracks35.discriminator.root",
        "/home/mmhaidra/SliceMethod/Chance_largedrum_bub_tracks35_discr_STE3_dense/largedrum_bub_2.95l_STE3_dense_tracks35.discriminator.root"
    };
    const int colors[] = { kBlack, kBlue, kRed };
    const char* diff_names[] = {
        "Distributions_diff_RefDrumVsXdrum_STE3_20l_Sept_mirror.pdf",
        "Distributions_diff_RefDrumVsXdrum_STE3_17l_Sept_mirror.pdf",
        "Distributions_diff_RefDrumVsXdrum_STE3_2l_Sept_mirror.pdf"
    };

    // Load histograms
    std::vector<TH3F*> hists;
    for (int i = 0; i < 4; ++i) {
        TH3F* h = LoadTH3F(files[i], "histBS");
        hists.push_back(h);
    }

    // Prepare difference histograms
    std::vector<TH1D*> h_diffs;
    std::vector<TH3F*> h3_diffs;
    for (int i = 0; i < 3; ++i) {
        h_diffs.push_back(new TH1D(Form("h_diff%d", i), Form("Diff_Xbin%d", i), 500, -2, 2));
        h3_diffs.push_back((TH3F*)hists[0]->Clone(Form("h3_diff%d", i)));
    }

    // Compute and draw differences
    for (int idx = 0; idx < 3; ++idx) {
        ComputeVoxelDiff(hists[idx == 0 ? 0 : idx + 1], hists[1], h3_diffs[idx], h_diffs[idx], idx != 1);
        DrawAndSaveHist(h_diffs[idx], diff_names[idx], colors[idx], "Voxel By Voxel Diff", "Entries");
    }

    // Draw all on one canvas
    gStyle->SetOptStat(0);
    TCanvas* c_all = new TCanvas("c_test_all", "Graph_test_all", 200, 100, 1400, 700);
    for (int i = 2; i >= 0; --i) {
        h_diffs[i]->SetStats(false);
        h_diffs[i]->SetLineColor(colors[i]);
        h_diffs[i]->Draw(i == 2 ? "" : "same");
    }
    c_all->SaveAs("Distributions_diff_RefDrumVsXdrum_STE3_3cases_Sept_mirror.pdf", "pdf");
    delete c_all;

    // Clean up
    for (auto h : hists) delete h;
    for (auto h : h_diffs) delete h;
    for (auto h : h3_diffs) delete h;
}

int main(int argc, char** argv) {
    Voxelbyvoxel_difference();
    return 0;
}
