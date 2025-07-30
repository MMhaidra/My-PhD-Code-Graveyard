// M.Mhaidra 14/07/2020
// Distribution of bitumen signal only


#ifndef __CINT__
#include "RooGlobalFunc.h"
#endif

#include "TCanvas.h"
#include "TH1D.h"
#include "TStyle.h"
#include "TF1.h"
#include "RooRealVar.h"
#include "RooDataHist.h"
#include "RooGaussian.h"
#include "RooAddPdf.h"
#include "RooPlot.h"
#include "RooFitResult.h"
#include <iostream>

using namespace RooFit;

// Helper functions
TH1D* CreateAndFillHistogram(const double* data, int size, const char* name, 
                            const char* title, int bins, double min, double max) {
    TH1D* hist = new TH1D(name, title, bins, min, max);
    for (int i = 1; i < size; i++) {
        hist->Fill(data[i]);
    }
    return hist;
}

void PrintHistogramStats(TH1D* hist) {
    std::cout << "Mean = " << hist->GetMean() 
              << "\nError Mean = " << hist->GetMeanError() << std::endl;
    std::cout << "Std Dev = " << hist->GetStdDev() 
              << "\nError Std Dev = " << hist->GetStdDevError() << std::endl;
}

RooFitResult* FitDistribution(TH1D* hist, RooRealVar& x, RooDataHist& dh, 
                            RooPlot* frame) {
    // Define fit parameters
    RooRealVar mean("#mu", "mean of gaussians", 7.5, 10.5);
    RooRealVar sigma("#sigma", "width of gaussians", 400.5, 480.4);
    RooGaussian signal("signal", "signal", x, mean, sigma);
    RooRealVar nsig("N_{Bkg}", "signal events", 0, 504);
    
    // Create and fit model
    RooAddPdf model("model", "model", RooArgList(signal), RooArgList(nsig));
    RooFitResult* result = model.fitTo(dh, Extended(kTRUE), Save());
    
    // Draw results
    model.paramOn(frame, Layout(0.7, 0.9, 0.8));
    model.plotOn(frame, Name("model"));
    
    return result;
}

void Bkg_distribution() {
    // Data configuration
    const int n = 504;
    // Signal data array definition (keeping original data)
    double Bkg_signal[n] = {-320.529, 635.004, 696.834, 655.967, 1157.28, 1011.81, 929.491, 111.094, 29.9806, -74.5506, -591.603, 15.296, -57.8849, 41.3396, -329.378, 423.852, -95.4676, -170.078, 51.1256, -86.7706, 165.415, 36.7601, 259.581, -376.175, -214.93, 251.514, 150.339, 360.382, -215.469, 1627.74, 729.615, 56.3501, 91.6887, 269.044, 224.805, -150.331, 667.457, 509.146, 504.813, 912.447, 663.628, 799.541, -159.39, 623.958, -349.546, -166.326, -174.528, -51.073, 186.639, -109.587, 94.5519, -562.376, -594.153, 291.821, 54.1488, 247.699, 320.529, -635.004, -696.834, -655.967, -1157.28, -1011.81, -929.491, 720.684, -593.258, -749.423, -1106.91, -1062.59, -1043.24, -874.571, -48.0965, -262.367, -488.497, -633.809, -977.795, -1064.19, -756.425, 112.675, 12.1675, -722.949, -703.899, -907.088, -884.747, -615.431, 311.331, 1332.81, 261.395, -357.762, -955.198, -744.75, -713.902, 33.666, 14.1113, -53.3838, 30.609, -170.279, -326.685, -124.563, 565.987, -69.9387, -999.219, -636.497, -1275.87, -1080.09, -759.735, 103.818, -988.037, -1167.61, -1191.91, -889.918, -973.832, -689.62, -111.094, -29.9806, 74.5506, 591.603, -15.296, 57.8849, -41.3396, -720.684, 593.258, 749.423, 1106.91, 1062.59, 1043.24, 874.571, -410.551, 204.032, 267.957, 461.897, 14.8261, -39.2663, 120.306, -143.567, 228.833, -395.881, 186.744, 172.597, 189.739, 297.706, -293.359, 1544.63, 895.685, 696.553, 27.5241, 296.558, 179.034, -143.653, 589.753, 397.424, 1087.49, 930.714, 743.663, 773.984, -33.3367, 978.396, -423.933, 226.11, -217.781, -45.1189, 131.275, -355.993, -244.377, -732.383, -117.793, 141.701, 110.347, 226.091, 329.378, -423.852, 95.4676, 170.078, -51.1256, 86.7706, -165.415, 48.0965, 262.367, 488.497, 633.809, 977.795, 1064.19, 756.425, 410.551, -204.032, -267.957, -461.897, -14.8261, 39.2663, -120.306, 522.391, -67.5466, -382.985, -55.7193, 165.03, 209.902, 154.601, 209.882, 1391.37, 616.854, 231.57, 63.7265, 332.043, 60.4324, 285.495, 57.6032, 462.987, 700.008, 815.292, 743.834, 638.979, 363.814, 213.738, -330.254, -27.1011, -268.272, -15.3, 2.93721, 8.63482, -399.173, -613.087, -522.908, 142.148, 122.038, 93.3167, -36.7601, -259.581, 376.175, 214.93, -251.514, -150.339, -360.382, -112.675, -12.1675, 722.949, 703.899, 907.088, 884.747, 615.431, 143.567, -228.833, 395.881, -186.744, -172.597, -189.739, -297.706, -522.391, 67.5466, 382.985, 55.7193, -165.03, -209.902, -154.601, 53.4788, 1816.86, 1090.97, 337.851, -171.279, 95.0962, -121.667, -207.365, 356.17, 636.445, 673.63, 667.589, 546.481, 459.666, 43.9804, -76.2183, -167.863, 53.0896, -441.381, -213.565, -150.566, -307.303, -247.978, -493.075, -433.297, -36.0075, -70.663, -69.4292, 215.469, -1627.74, -729.615, -56.3501, -91.6887, -269.044, -224.805, -311.331, -1332.81, -261.395, 357.762, 955.198, 744.75, 713.902, 293.359, -1544.63, -895.685, -696.553, -27.5241, -296.558, -179.034, -209.882, -1391.37, -616.854, -231.57, -63.7265, -332.043, -60.4324, -53.4788, -1816.86, -1090.97, -337.851, 171.279, -95.0962, 121.667, -242.593, -1035.33, -223.174, 381.111, 819.995, 434.148, 592.168, 168.561, -1395.46, -1022.74, -383.011, -305.357, -311.846, -36.8755, -357.588, -1611.04, -1167.12, -755.942, 125.099, -195.983, 36.316, 150.331, -667.457, -509.146, -504.813, -912.447, -663.628, -799.541, -33.666, -14.1113, 53.3838, -30.609, 170.279, 326.685, 124.563, 143.653, -589.753, -397.424, -1087.49, -930.714, -743.663, -773.984, -285.495, -57.6032, -462.987, -700.008, -815.292, -743.834, -638.979, 207.365, -356.17, -636.445, -673.63, -667.589, -546.481, -459.666, 242.593, 1035.33, 223.174, -381.111, -819.995, -434.148, -592.168, 133.166, 310.709, -982.602, -845.536, -1085.03, -772.127, -633.588, -379.937, -471.87, -1059.23, -1220.23, -648.612, -639.517, -555.84, 159.39, -623.958, 349.546, 166.326, 174.528, 51.073, -186.639, -565.987, 69.9387, 999.219, 636.497, 1275.87, 1080.09, 759.735, 33.3367, -978.396, 423.933, -226.11, 217.781, 45.1189, -131.275, -363.814, -213.738, 330.254, 27.1011, 268.272, 15.3, -2.93721, -43.9804, 76.2183, 167.863, -53.0896, 441.381, 213.565, 150.566, -168.561, 1395.46, 1022.74, 383.011, 305.357, 311.846, 36.8755, -133.166, -310.709, 982.602, 845.536, 1085.03, 772.127, 633.588, -453.601, -535.572, 192.852, -418.299, 337.178, 96.9956, 68.2465, 109.587, -94.5519, 562.376, 594.153, -291.821, -54.1488, -247.699, -103.818, 988.037, 1167.61, 1191.91, 889.918, 973.832, 689.62, 355.993, 244.377, 732.383, 117.793, -141.701, -110.347, -226.091, -8.63482, 399.173, 613.087, 522.908, -142.148, -122.038, -93.3167, 307.303, 247.978, 493.075, 433.297, 36.0075, 70.663, 69.4292, 357.588, 1611.04, 1167.12, 755.942, -125.099, 195.983, -36.316, 379.937, 471.87, 1059.23, 1220.23, 648.612, 639.517, 555.84, 453.601, 535.572, -192.852, 418.299, -337.178, -96.9956, -68.2465};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

TH1D *Bkg_signal_hist = new TH1D("Bkg_signal_hist","Bkg_signal_hist_bitumin",30,-2500,2500);

gStyle->SetOptFit(1);

    for (int i=1; i<n; i++)
    {
        Bkg_signal_hist->Fill(Bkg_signal[i]);

          }

    Bkg_signal_hist->SetXTitle("Bitumen signal");
    Bkg_signal_hist->SetYTitle("Entries");
    Bkg_signal_hist->SetLineColor(1);

std::cout << "Mean = "<< Bkg_signal_hist->GetMean() << "\n"<< "Error Mean = " << Bkg_signal_hist->GetMeanError() << std::endl;

std::cout << "Std Dev = "<< Bkg_signal_hist->GetStdDev() << "\n"<< "Error Std Dev = " << Bkg_signal_hist->GetStdDevError() << std::endl;

/*
// Fit distributions with a gaussian
    TF1 *f1 = new TF1("f1","gaus", -2000, 2000);
    //f1->SetParameters(Bkg_signal_hist->GetMaximum(), Bkg_signal_hist->GetMean(), Bkg_signal_hist->GetRMS());
    f1->SetParNames("Constant", "Mean", "Sigma");

    double par[3];
    f1->SetParameters(par);

    f1->SetLineColor(kBlue);

    TCanvas *c2 = new TCanvas("c2","Graph",200, 50, 1200, 600);

  //    Bkg_signal_hist->Fit(f1, "R+");

 //   std::cout << "Mean = " << f1->GetParameter(1) << "\n"<<  "Error on the Mean = "<< f1->GetParError(1) << "\n" << std::endl;

     c2->cd();

    Bkg_signal_hist->Draw("e1");

  //  f1->Draw("SAME");
  //  std::cout << std::endl; f1->Print();


    c2->SaveAs("Distribution_bitumen_signal_newdata.pdf","pdf");

 */

      RooRealVar x1("x1","Bitumen signal",-2000,2000) ;
      RooDataHist dh1("dh1","dataset",x1,Bkg_signal_hist);
      RooPlot* frame1 = x1.frame(Title("Bkg_signal_bitumin")) ;
      dh1.plotOn(frame1,Name("dh1")) ;
      RooRealVar mean11("#mu","mean of gaussians",7.5,10.5);
      RooRealVar sigma11("#sigma","width 1 of gaussians " ,400.5,480.4);


      RooGaussian sig11("sig11", "sig11", x1, mean11, sigma11);

      RooRealVar nsig1("N_{Bkg}","signal events",0,504);

      RooAddPdf all1("all1","model",RooArgList(sig11),RooArgList(nsig1));

      RooFitResult* r1 = all1.fitTo(dh1,Extended(kTRUE),Save()) ;
      all1.paramOn(frame1,Layout(0.7,0.9,0.8));
      all1.plotOn(frame1,Name("all1"));
      all1.plotOn(frame1);
      r1->Print();
/*
      RooPlot *residual1= x1.frame();
      RooHist *resHist1 = (RooHist*)frame1->pullHist();
      residual1->addObject(resHist1,"PZ");
      residual1->SetMinimum(-55.);
      residual1->SetMaximum(+55.);
      residual1->GetXaxis()->SetLabelSize(0.1);
      residual1->GetXaxis()->SetTitleOffset(0.8);
      residual1->GetXaxis()->SetTitleSize(0.15);
      residual1->GetYaxis()->SetLabelSize(0.1);
      residual1->GetYaxis()->SetNdivisions(110);

    TCanvas *c1 = new TCanvas("c1","Graph",1200,1200);
   // c1->Divide();
    c1->GetPad(1)->SetPad(0.,0.3,1.,1.);
    c1->GetPad(2)->SetPad(0.,0.,1.,0.3);
    c1->GetPad(1)->SetBottomMargin(0);
    c1->GetPad(2)->SetTopMargin(0);
    c1->GetPad(2)->SetBottomMargin(0.25);
    c1->GetPad(2)->SetFillColor(4000);
    c1->cd();
    r1->Draw();
    frame1->Draw();
   // c1->cd(2);
   // residual1->Draw();

    c1->SaveAs("Distribution_bitumen_signal_newdata_Roofit.pdf","pdf");

*/


       TCanvas *c1 = new TCanvas("c1","Graph",200, 50, 1200, 600);
       gPad->SetLeftMargin(0.15) ; frame1->GetYaxis()->SetTitleOffset(1.6) ; frame1->Draw() ;
       c1->SaveAs("Distribution_bitumen_signal_newdata_Roofit.pdf","pdf");

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

}

int main( int argc, char** argv )
{

    Bkg_distribution();

}

