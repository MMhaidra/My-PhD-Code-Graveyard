//M.Mhaidra 25/04/2019//
// This script plots the difference of the mean value of Bristol discriminator in slices
// and compare it in case of different background.


#include <memory>
#include <iostream>
#include "TCanvas.h"
#include "TTree.h"
#include "TH1D.h"
#include "TFile.h"
#include "TRandom.h"
#include "TLine.h"
#include "TPaveText.h"
#include "TLegend.h"
#include "TStyle.h"
#include "TGraph.h"
#include "TMultiGraph.h"
#include "TGraphErrors.h"

struct MaterialData {
    TString basePath;
    TString suffix;
    TString referenceFile;
    const char* description;
    double* material;
    double* discrDiff;
};

class DiscriminatorAnalyzer {
private:
    static constexpr int n = 2;
    double e_diff[n] = {0};
    double e_mat[n] = {0};

    struct HistogramResult {
        double mean;
        double error;
    };

    HistogramResult ProcessHistogram(const char* filename, const char* histName) {
        std::unique_ptr<TFile> file(new TFile(filename, "READ"));
        if (!file || file->IsZombie()) {
            std::cerr << "Error opening file: " << filename << std::endl;
            return {0, 0};
        }

        TTree* tree = (TTree*)file->Get("T");
        if (!tree) return {0, 0};

        auto hist = std::make_unique<TH1D>(histName, histName, 200, 0, 140);
        tree->Draw(Form("discr>>%s", histName),
                  "((z*z)+(y*y)<(300*300))&&(x>-480)&&(x<480)", "goff");
        
        hist->SetDirectory(0);
        hist->GetXaxis()->SetRangeUser(5.0, 15.0);

        return {hist->GetMean(), hist->GetMeanError()};
    }

    void ProcessMaterial(const MaterialData& data) {
        for (int i = 0; i < n; i++) {
            TString currentFile = data.basePath + Form("%d", i) + data.suffix;
            
            auto result1 = ProcessHistogram(currentFile.Data(), 
                Form("hist_with_hydrogen_%d", i));
            auto result2 = ProcessHistogram(data.referenceFile.Data(), 
                Form("hist_reference_%d", i));

            std::cout << "Mean with hydrogen: " << result1.mean 
                     << " ± " << result1.error << std::endl;
            std::cout << "Mean reference: " << result2.mean 
                     << " ± " << result2.error << std::endl;

            data.discrDiff[i] = (result1.mean - result2.mean) / 
                               (result1.error + result2.error);
        }
    }

    TGraphErrors* CreateGraph(const MaterialData& data, int color) {
        auto graph = new TGraphErrors(n, data.material, data.discrDiff, e_mat, e_diff);
        graph->SetFillColor(color);
        graph->SetMarkerStyle(8);
        graph->SetMarkerSize(1.2);
        graph->SetMarkerColor(color);
        return graph;
    }

public:
    void Analyze() {
        double material_1[n] = {6, 8};
        double material_2[n] = {12, 14};
        double material_3[n] = {18, 20};
        double discr_diff_1[n], discr_diff_2[n], discr_diff_3[n];

        MaterialData materials[] = {
            {"/home/mmhaidra/SliceMethod/discr_DifferentMaterials/largedrum_bub_",
             "_EUROBITUM_tracks35.discriminator.root",
             "/home/mmhaidra/SliceMethod/discr_DifferentMaterials/largedrum_EUROBITUM_tracks35.discriminator.root",
             "EUROBITUM", material_1, discr_diff_1},
            
            {"/home/mmhaidra/SliceMethod/discr_DifferentMaterials/largedrum_bub_",
             "_STE3_tracks35.discriminator.root",
             "/home/mmhaidra/SliceMethod/discr_DifferentMaterials/largedrum_STE3_tracks35.discriminator.root",
             "STE3", material_2, discr_diff_2},
            
            {"/home/mmhaidra/SliceMethod/discr_DifferentMaterials/largedrum_bub_",
             "_STE3_dense_tracks35.discriminator.root",
             "/home/mmhaidra/SliceMethod/discr_DifferentMaterials/largedrum_STE3_dense_tracks35.discriminator.root",
             "Dense STE3", material_3, discr_diff_3}
        };

        for (const auto& material : materials) {
            ProcessMaterial(material);
        }

        auto mg = std::make_unique<TMultiGraph>();
        mg->Add(CreateGraph(materials[0], kMagenta));
        mg->Add(CreateGraph(materials[1], kRed));
        mg->Add(CreateGraph(materials[2], kBlue));

        auto canvas = std::make_unique<TCanvas>("c1", "Graph", 200, 50, 1200, 600);
        mg->Draw("AB");
        mg->SetTitle("Mean of discriminator difference for gas inside different types of bitumen ; Materials ; #mu_{discr_difference}/error");
        mg->GetYaxis()->SetRangeUser(0, 9);
        mg->GetYaxis()->SetTitleSize(0.05);
        mg->GetXaxis()->SetTitleSize(0.05);
        mg->GetYaxis()->SetTitleOffset(0.70);
        mg->GetXaxis()->SetTitleOffset(0.80);

        canvas->Update();
        canvas->GetFrame()->SetLineWidth(1);
        canvas->GetFrame()->SetFillColor(10);
        canvas->Modified();
        canvas->SaveAs("Mean_of_discriminator_difference_in_EUROBITUM_Vs_STE3_Vs_DenseSTE3.pdf", "pdf");
    }
};

void Detection_InDifferentMaterial() {
    DiscriminatorAnalyzer analyzer;
    analyzer.Analyze();
}

int main(int argc, char** argv) {
    Detection_InDifferentMaterial();
    return 0;
}

