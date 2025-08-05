dev.off()

# ROC Macro: Loop over files and produce ROC curves with AUC and confidence intervals
# Produces 'info' plots containing ROC curve, file name, and AUC ± CI

# --- User parameters ---
output_pdf <- "/home/mmhaidra/SliceMethod/AUCresults/AUCwithErrors3cm.pdf"  # Output PDF file
input_file <- "/home/mmhaidra/SliceMethod/Roc3cmMedian.txt"                 # Input data file
# To process multiple files, see the commented section below

# --- Load required library ---
library(pROC)  # For ROC analysis

# --- Prepare PDF output ---
pdf(output_pdf)

# --- Read data ---
# If looping over files, replace input_file with file in a loop
mydata <- read.table(input_file)
print(mydata)  # Show loaded data for verification

# --- Compute ROC and confidence intervals ---
# Setting levels: control = 0, case = 1
# Setting direction: controls < cases
roc1 <- roc(mydata$V1, mydata$V2)
print(roc1)

# Compute confidence interval for AUC
ci_auc <- ci(roc1)
print(ci_auc)

# Compute confidence intervals for sensitivity at all thresholds
ci_sens <- ci.sp(roc1, sensitivities=seq(0, 1, .01))
print(ci_sens)

# --- Plot ROC curve and confidence intervals ---
plot.roc(roc1, legacy.axes=TRUE, main="", xlab="Purity", ylab="Efficiency", CI=TRUE, cex.lab=2, cex.axis=1.9)
# Overlay AUC confidence interval as a shaded region
plot(ci_auc, type="shape", col="lightblue")
# Overlay sensitivity confidence intervals (DeLong method)
plot(ci.sp(roc1, method="delong", sensitivities=seq(0, 1, .01)), type="shape")

# --- Annotate plot with AUC and CI ---
auc <- signif(100 * ci_auc[2], digits=4)
lower <- signif(100 * (ci_auc[2] - ci_auc[1]), digits=2)
upper <- signif(100 * (ci_auc[3] - ci_auc[2]), digits=2)
text(0.35, 0.2, paste("AUC =", auc, "+", upper, "-", lower, "%"), cex=1.8)

# --- (Optional) Format for LaTeX copy-paste ---
# text(0.3, 0.3, paste("\\tol{", auc, "\\%}{", upper, "}{", lower, "}"))

# --- (Optional) Loop over all files in a directory ---
# Uncomment and adapt the following to process all .txt files in a directory:
#
# filelist <- list.files(path="/path/to/dir", pattern=".txt$", full.names=TRUE)
# for (file in filelist) {
#   mydata <- read.table(file)
#   ... (repeat ROC analysis and plotting as above) ...
# }

# --- Close PDF device ---
