// M.Mhaidra 10/03/2021 //


#include <algorithm>
#include <iostream>
#include <memory>
#include <vector>
#include "TCanvas.h"
#include "TFile.h"
#include "TH1D.h"
#include "TH3F.h"
#include "TStyle.h"

class MedianAnalyzer {
  private:
    struct HistogramData {
        std::unique_ptr<TH3F> hist3D;
        std::unique_ptr<TH3F> hist3DRef;
        std::unique_ptr<TH1D> histMedianMetric;
        std::unique_ptr<TH1D> histMedianMetricNoCut;
        std::unique_ptr<TH1D> histBubble;
        std::unique_ptr<TH1D> histBitumenLeft;
        std::unique_ptr<TH1D> histBitumenRight;
        std::unique_ptr<TH1D> histBitumenLeftEx;
        std::unique_ptr<TH1D> histBitumenRightEx;
        std::unique_ptr<TH1D> histBitumenCentre;
    };

    HistogramData data_;

    void InitializeHistograms() {
        data_.histMedianMetric = std::make_unique<TH1D>("h_histMedianMetric", "histMedianMetric", 100, 9.5, 15);
        data_.histMedianMetricNoCut = std::make_unique<TH1D>("h_histMedianMetric_noCut", "histMedianMetric_noCut", 100, 9.5, 15);
        data_.histBubble = std::make_unique<TH1D>("h_histMedianMetricBubble", "hist Median Metric Bubble", 100, 9.5, 15);
        data_.histBitumenLeft = std::make_unique<TH1D>("h_histMedianMetricBitumenLeft", "hist Median Metric Bitumen Left", 100, 9.5, 15);
        data_.histBitumenRight = std::make_unique<TH1D>("h_histMedianMetricBitumenRight", "hist Median Metric Bitumen Right", 100, 9.5, 15);
        data_.histBitumenLeftEx = std::make_unique<TH1D>("h_histMedianMetricBitumenLeftEx", "hist Median Metric Bitumen Left Edge", 100, 9.5, 15);
        data_.histBitumenRightEx = std::make_unique<TH1D>("h_histMedianMetricBitumenRightEx", "hist Median Metric Bitumen Right Edge", 100, 9.5, 15);
        data_.histBitumenCentre = std::make_unique<TH1D>("h_histMedianMetricBitumenCentre", "hist Median Metric Bitumen Centre", 100, 9.5, 15);
    }

    bool LoadHistograms() {
        auto file1 = std::make_unique<TFile>("/home/mmhaidra/SliceMethod/largedrum_21L_dense_MedianCut_3cmVoxel_all.discriminator.root", "READ");
        auto file2 = std::make_unique<TFile>("/home/mmhaidra/SliceMethod/Discriminator_data_ROC/largedrum_onlybitumen_dense_newmetrics_10cm_1cm_3cmVoxel_ROC1.discriminator.root", "READ");
        
        if (!file1 || !file2) return false;

        data_.hist3D.reset((TH3F*)file1->Get("histMedianMetric"));
        data_.hist3DRef.reset((TH3F*)file2->Get("histMedianMetric"));
        
        if (data_.hist3D) data_.hist3D->SetDirectory(0);
        if (data_.hist3DRef) data_.hist3DRef->SetDirectory(0);
        
        return (data_.hist3D && data_.hist3DRef);
    }

    void ProcessRegions() {
        ProcessBubbleRegion();
        ProcessBitumenRegions();
    }

    void ProcessBubbleRegion() {
        for (int i = data_.hist3D->GetXaxis()->FindBin(-80); i < data_.hist3D->GetXaxis()->FindBin(80); i++) {
            for (int j = data_.hist3D->GetYaxis()->FindBin(-80); j < data_.hist3D->GetYaxis()->FindBin(80); j++) {
                for (int k = data_.hist3D->GetZaxis()->FindBin(-80); k < data_.hist3D->GetZaxis()->FindBin(80); k++) {
                    data_.histBubble->Fill(data_.hist3D->GetBinContent(i,j,k));
                }
            }
        }
    }

    void ProcessBitumenRegions() {
        ProcessBitumenRegion(-400, -210, data_.histBitumenLeft.get());
        ProcessBitumenRegion(210, 400, data_.histBitumenRight.get());
        ProcessBitumenRegion(-440, -250, data_.histBitumenLeftEx.get());
        ProcessBitumenRegion(250, 440, data_.histBitumenRightEx.get());
    }

    void ProcessBitumenRegion(int xMin, int xMax, TH1D* hist) {
        for (int i = data_.hist3D->GetXaxis()->FindBin(xMin); i < data_.hist3D->GetXaxis()->FindBin(xMax); i++) {
            for (int j = data_.hist3D->GetYaxis()->FindBin(-280); j < data_.hist3D->GetYaxis()->FindBin(280); j++) {
                for (int k = data_.hist3D->GetZaxis()->FindBin(-280); k < data_.hist3D->GetZaxis()->FindBin(280); k++) {
                    hist->Fill(data_.hist3D->GetBinContent(i,j,k));
                }
            }
        }
    }

    void SavePlots() {
        gStyle->SetOptStat(0);
        
        auto canvas = std::make_unique<TCanvas>("canva2", "Graph2", 200, 100, 1400, 700);
        
        data_.histBitumenLeft->SetLineColor(kBlack);
        data_.histBitumenRight->SetLineColor(kRed);
        data_.histBitumenLeftEx->SetLineColor(kBlue);
        data_.histBitumenRightEx->SetLineColor(kGreen+2);
        data_.histBitumenCentre->SetLineColor(kMagenta+2);
        
        for (auto& hist : {data_.histBitumenLeft.get(), data_.histBitumenRight.get(), 
                          data_.histBitumenLeftEx.get(), data_.histBitumenRightEx.get(), 
                          data_.histBitumenCentre.get()}) {
            hist->GetYaxis()->SetRangeUser(0,350);
            hist->SetLineWidth(2);
        }
        
        data_.histBitumenLeft->Draw();
        data_.histBitumenRight->Draw("same");
        data_.histBitumenLeftEx->Draw("same");
        data_.histBitumenRightEx->Draw("same");
        
        canvas->SaveAs("1_discriminator_21L_STE3_Median_InsideBitumen_ManyRegions.pdf");
    }

  public:
    MedianAnalyzer() {
        InitializeHistograms();
    }

    bool Run() {
        if (!LoadHistograms()) {
            std::cerr << "Failed to load histograms" << std::endl;
            return false;
        }
        
        ProcessRegions();
        SavePlots();
        return true;
    }
};

void medianCut3D() {
    MedianAnalyzer analyzer;
    analyzer.Run();
}

int main(int argc, char** argv) {
    medianCut3D();
    return 0;
}
