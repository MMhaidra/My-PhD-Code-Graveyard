/*
 * plotProjections.C
 *
 * Created on: Feb 5, 2019
 * Author: dkikola
 * 
 * This macro provides two main functions:
 * 1. plotProjections: Creates 2D projections from 3D histograms
 * 2. ReadDataAndPlot: Reads cluster data and creates 2D visualizations
 */

#include <memory>
#include <vector>
#include <iostream>
// ROOT data structures
#include <TString.h>
#include <TFile.h>
#include <TH3.h>
#include <TH2.h>
#include <TGraph2D.h>
#include "TH1F.h"
// ROOT tree handling
#include "TTreeReader.h"
#include "TTreeReaderValue.h"
// ROOT graphics
#include "TCanvas.h"

class ProjectionAnalyzer {
private:
    struct HistogramCollection {
        std::unique_ptr<TH3F> hist25;
        std::unique_ptr<TH3F> hist50;
        std::unique_ptr<TH3F> hist75;
        std::unique_ptr<TH3F> histPC;
        std::unique_ptr<TH3F> histBS;
        std::unique_ptr<TH3I> histNS;
    };

    // Load histograms from file with proper memory management
    HistogramCollection LoadHistograms(const char* filename) {
        std::unique_ptr<TFile> file(new TFile(filename, "READ"));
        HistogramCollection histograms;
        
        if (!file || file->IsZombie()) {
            std::cerr << "Error opening file: " << filename << std::endl;
            return histograms;
        }

        // Load all histograms and detach from file
        histograms.hist25.reset((TH3F*)file->Get("hist25"));
        histograms.hist50.reset((TH3F*)file->Get("hist50"));
        histograms.hist75.reset((TH3F*)file->Get("hist75"));
        histograms.histPC.reset((TH3F*)file->Get("histPC"));
        histograms.histBS.reset((TH3F*)file->Get("histBS"));
        histograms.histNS.reset((TH3I*)file->Get("histNS"));

        // Detach histograms from file
        if (histograms.histBS) histograms.histBS->SetDirectory(0);
        if (histograms.histPC) histograms.histPC->SetDirectory(0);
        
        return histograms;
    }

public:
    // Create 2D projection with specified bin range
    void CreateProjection(const char* filename, int bin1, int bin2) {
        auto histograms = LoadHistograms(filename);
        if (!histograms.histBS) {
            std::cerr << "Failed to load histBS" << std::endl;
            return;
        }

        // Set range and create projection
        histograms.histBS->GetYaxis()->SetRange(bin1, bin2);
        auto hXY = std::unique_ptr<TH2D>((TH2D*)histograms.histBS->Project3D("zx"));
        
        // Create canvas and draw projection
        auto canvas = std::make_unique<TCanvas>("cProj", "Projection", 800, 600);
        hXY->Draw("colz");
    }

    // Read cluster data and create 2D visualizations
    void AnalyzeClusterData(const char* filename, Double_t zMin, Double_t zMax) {
        std::unique_ptr<TFile> file(new TFile(filename, "READ"));
        if (!file || file->IsZombie()) {
            std::cerr << "Error opening file: " << filename << std::endl;
            return;
        }

        // Create 2D graphs for discriminator and track counts
        auto grDisc = std::make_unique<TGraph2D>();
        auto grNtrk = std::make_unique<TGraph2D>();

        // Configure tree reader
        TTreeReader myReader("T", file.get());
        TTreeReaderValue<Double_t> x(myReader, "clusters.x");
        TTreeReaderValue<Double_t> y(myReader, "clusters.y");
        TTreeReaderValue<Double_t> z(myReader, "clusters.z");
        TTreeReaderValue<Double_t> nTracks(myReader, "clusters.tracks");
        TTreeReaderValue<Double_t> discr(myReader, "clusters.discr");

        // Process entries within z-range
        int pointIndex = 0;
        while (myReader.Next()) {
            if (*z < zMin || *z > zMax) continue;
            
            grDisc->SetPoint(pointIndex, *x, *y, *discr);
            grNtrk->SetPoint(pointIndex, *x, *y, *nTracks);
            pointIndex++;
        }

        // Create and configure canvases
        auto cTrck = std::make_unique<TCanvas>("cTrck", "Track Counts", 800, 600);
        auto cDisc = std::make_unique<TCanvas>("cDisc", "Discriminator", 800, 600);

        // Draw visualizations
        grNtrk->Draw("colz");
        cDisc->cd();
        grDisc->Draw("colz");
    }
};

// Wrapper functions for ROOT macro compatibility
void plotProjections(TString inFileName, int bin1, int bin2) {
    ProjectionAnalyzer analyzer;
    analyzer.CreateProjection(inFileName.Data(), bin1, bin2);
}

void ReadDataAndPlot(TString inFileName, Double_t zMin, Double_t zMax) {
    ProjectionAnalyzer analyzer;
    analyzer.AnalyzeClusterData(inFileName.Data(), zMin, zMax);
}

