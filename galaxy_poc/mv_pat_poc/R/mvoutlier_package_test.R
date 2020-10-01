
# install package
install.packages("mvoutlier")
# load package
library(mvoutlier)
# run detection on data matrix
rst = uni.plot(input_matrix)
# in rst$outliers full list of dies, true if outlier, false otherwise
# in rst$md list of mahalanobis distances for each die to explain outliers