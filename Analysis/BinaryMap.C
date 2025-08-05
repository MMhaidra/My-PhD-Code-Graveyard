// M.Mhaidra 21/10/2021 //


#include "TCanvas.h"
#include "TTree.h"
#include "TH3D.h"
#include "TH3F.h"
#include "TH2D.h"
#include "TH1D.h"
#include "TFile.h"
#include "TStyle.h"
#include <cmath>
#include <vector>

struct CylinderRegion {
    double xMin;
    double xMax;
    double threshold;
    bool isCenter;
    bool isUpper;
};

// Helper functions
TH3F* LoadHistogram(const char* filepath, const char* histname) {
    TFile* file = new TFile(filepath, "READ");
    TH3F* hist = (TH3F*)file->Get(histname);
    if (hist) hist->SetDirectory(0);
    file->Close();
    delete file;
    return hist;
}

bool IsInsideCylinder(double y, double z, double centerY, double centerZ, double radius) {
    return (pow(y - centerY, 2.0) + pow(z - centerZ, 2.0) < pow(radius, 2.0));
}

void ProcessRegion(TH3F* hist3D, TH3F* diffHist, const CylinderRegion& region, 
                  double NbinsYCenter, double NbinsZCenter, double dY, double dZ) {
    for (int i = hist3D->GetXaxis()->FindBin(region.xMin); 
         i < hist3D->GetXaxis()->FindBin(region.xMax); i++) {
        for (int j = hist3D->GetYaxis()->FindBin(-1000); 
             j < hist3D->GetYaxis()->FindBin(1000); j++) {
            for (int k = hist3D->GetZaxis()->FindBin(-500); 
                 k < hist3D->GetZaxis()->FindBin(500); k++) {
                
                double y = (j - NbinsYCenter) * dY;
                double z = (k - NbinsZCenter) * dZ;
                
                if (!IsInsideCylinder(y, z, 0, 0, 240)) continue;
                
                // Check z-range conditions based on region type
                if (region.isCenter) {
                    if (z > -100 && z <= 100) continue;
                } else if (region.isUpper) {
                    if (z <= 100) continue;
                } else {
                    if (z >= -100) continue;
                }
                
                if (hist3D->GetBinContent(i,j,k) >= region.threshold) {
                    diffHist->SetBinContent(i,j,k, 1.);
                }
            }
        }
    }
}

void BinaryMap() {
    // Load input histogram
    TH3F* hist_3D = LoadHistogram("/home/mmhaidra/SliceMethod/largedrum_4L_2Cubes_dense_Central_3cmVoxel_Nov2021.discriminator.root", 
                                 "histMedianMetric");
    if (!hist_3D) return;

    // Create output histogram
    TH3F* h3_diff = (TH3F*)hist_3D->Clone("h3_diff");
    h3_diff->Reset();

    // Setup parameters
    double NbinsXCenter = h3_diff->GetNbinsX()/2;
    double NbinsYCenter = h3_diff->GetNbinsY()/2;
    double NbinsZCenter = h3_diff->GetNbinsZ()/2;
    double dX = 30, dY = 30, dZ = 30;

    // Define regions with their parameters
    std::vector<CylinderRegion> regions = {
        {-400, -300, 11.17, true,  false}, // Central regions
        {-300, -200, 11.19, true,  false},
        {-200, -100, 11.19, true,  false},
        {-100,    0, 11.35, true,  false},
        {   0,  100, 11.35, true,  false},
        { 100,  200, 11.19, true,  false},
        { 200,  300, 11.19, true,  false},
        { 300,  400, 11.15, true,  false},
        
        {-400, -300, 11.50, false, true},  // Upper regions
        {-300, -200, 11.56, false, true},
        {-200, -100, 11.53, false, true},
        {-100,    0, 11.66, false, true},
        {   0,  100, 11.66, false, true},
        { 100,  200, 11.56, false, true},
        { 200,  300, 11.56, false, true},
        { 300,  400, 11.54, false, true},
        
        {-400, -300, 11.25, false, false}, // Lower regions
        {-300, -200, 11.47, false, false},
        {-200, -100, 11.35, false, false},
        {-100,    0, 11.51, false, false},
        {   0,  100, 11.55, false, false},
        { 100,  200, 11.41, false, false},
        { 200,  300, 11.45, false, false},
        { 300,  400, 11.33, false, false},
    };

    // Process all regions
    for (const auto& region : regions) {
        ProcessRegion(hist_3D, h3_diff, region, NbinsYCenter, NbinsZCenter, dY, dZ);
    }

    // Save results
    TFile* outputFile = new TFile("BinaryMap_4L_2Cubes_Central_3cmVoxel.root", "RECREATE");
    h3_diff->Write();
    outputFile->Close();

    // Cleanup
    delete hist_3D;
    delete h3_diff;
    delete outputFile;
}

int main(int argc, char** argv) {
    BinaryMap();
    return 0;
}

