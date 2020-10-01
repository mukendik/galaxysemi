# load it with : source('E:/galaxy_repositories/galaxy_dev_v72/galaxy_poc/mv_pat_poc/R/mv_outliers_finder.R')


####################  sigmaToChi  ####################
# compute 
# sigma: sigma value
# degf: degrees of freedom of chi distribution
######################################################
sigmaToChi <- function(sigma, degf)
{
    # compute pvalue based on sigma value
    # sigma to pvalue : x2 because we suppose it's +/- sigma -
    pvalue = (1 - pnorm(sigma)) * 2
    # compute linked chi value
    chi = sqrt(qchisq(1-pvalue,degf))
}

####################  pvalueToSigma  ####################
# compute sigma from pvalue 
# chi: chi value
# degf: degrees of freedom of chi distribution
######################################################
pvalueToSigma <- function(pvalue)
{
    sigma = qnorm(1-(pvalue/2))
}

####################  chiToSigmaApprox  ##############
# linear extrapolation for very small p-values
# chi: chi value
# degf: degrees of freedom of chi distribution
######################################################
chiToSigmaApprox <- function(chi, degf)
{
    chi6p0 = sigmaToChi(6.0,degf)
    chi7p5 = sigmaToChi(7.5,degf)
    sigma = 6.0 + 1.5 * (chi-chi6p0) / (chi7p5-chi6p0)
}

####################  chiToSigma  ####################
# compute sigma
# chi: chi value
# degf: degrees of freedom of chi distribution
######################################################
chiToSigma <- function(chi, degf)
{
    # compute pvalue based on chi value
    pvalue = 1 - pchisq(chi*chi, degf)
    # compute sigma kniwing pvalue
    # 6.38e-14 is an empirical value
    sigma <- numeric( length(pvalue) )
    
    ok <- pvalue>6.38e-14 
    sigma[ok] <- pvalueToSigma(pvalue[ok])
    sigma[!ok] <- chiToSigmaApprox(chi[!ok],degf)
    # sigma = ifelse(pvalue>6.38e-14, pvalueToSigma(pvalue), )
    sigma
}

###################  findOutliers  ###################
# sigma: distance to define outliers
# x: matrix (parts X tests)
######################################################
findOutliers <- function(x, sigma)
{
#    print('findOutliers')
#    print(paste('x ', x))
    # omit na value
    x = na.exclude(x)
    png(file = "C:/Users/simon/Documents/myplot.png", bg = "transparent")
plot(1:10)
rect(1, 5, 3, 7, col = "white")
dev.off()
    print(paste('x rows: ', nrow(x)))
    print(paste('x cols: ', ncol(x)))

    # get number of tests
    dimx = ncol(x)
    # PCA computation
    # use prcomp because use svd (singular value decomposition) like the py lib
    # tranform of a matrix t(matrix)
    # with python proto Xp = (X - mean) * T(eigenvectors)
    # Xp stored match with 
    print('prcomp')
    xp = prcomp(x)$x
#    print(paste('xp ', xp))
    # estimated covariance matrix
    print('cov')
    covxp = cov(xp)
    print('colMeans')
    meanxp = colMeans(xp)
    # Mahalanobis distances computation
    print('maha')
    md2 = mahalanobis(xp, meanxp, covxp)
    # compute sqrt of each distances
    print('sqrt')
    md = sqrt(md2)
#    print(paste('md: ', md))
    # check each point with the threshold
    # compute threeshold based on sigma : sigma to chi
    chi = sigmaToChi(sigma, dimx)
#    print(paste('chi: ', chi))
    # list indexes of part with a bigger chi than threshold
    outliers = which(md > chi)
    # compute z-scores
    zscores = chiToSigma(md, dimx)
    print(paste('xp rows: ', nrow(xp)))
    print(paste('xp cols: ', ncol(xp)))
    
    return (list(o=outliers, c=chi, m=md, m2=md2, pc=xp, z=zscores))
}

# load data file
#ds1 <- read.csv("E:/en_cours/multivariate/R/formated_data/ds1.csv", header=TRUE, stringsAsFactors=FALSE)
# store result in rst
#rst = findOutliers(ds1[,29:30],6)


#mvoutlier_out_1 = findOutliers(mvoutlier_in_1[28:29,], 6)
#print(paste('outliers: ', mvoutlier_out_1$o))
#result2 <- mvoutlier_out_1$o
#result3 <- rst$m
