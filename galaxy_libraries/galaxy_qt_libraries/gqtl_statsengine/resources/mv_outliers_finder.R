
####################  pvalueToChi  ####################
# compute chi from pvalue 
# pvalue: pvalue
# degf: degrees of freedom of chi distribution
######################################################
pvalueToChi <- function(pvalue, degf)
{
    chi <- sqrt(qchisq(1 - pvalue, degf))
}

####################  pvalueToSigma  ####################
# compute sigma from pvalue 
# pvalue: pvalue
######################################################
pvalueToSigma <- function(pvalue)
{
    sigma <- qnorm(1 - (pvalue/2))
}

####################  sigmaToChiApprox  ##############
# linear extrapolation for very small p-values
# sigma: sigma value
# degf: degrees of freedom of chi distribution
######################################################
sigmaToChiApprox <- function(sigma, degf)
{
    chi6p0 <- sigmaToChi(6.0, degf)
    chi7p5 <- sigmaToChi(7.5, degf)
    chi <- chi6p0 + (chi7p5 - chi6p0) * (sigma - 6.0) / 1.5
}

####################  sigmaToChi  ####################
# compute sigma to chi value
# sigma: sigma value
# degf: degrees of freedom of chi distribution
######################################################
sigmaToChi <- function(sigma, degf)
{
    # compute pvalue based on sigma value
    # sigma to pvalue : x2 because we suppose it's +/- sigma -
    pvalue <- pnorm(sigma, lower.tail = F) * 2
    # 6.38e-14 is an empirical value
    if (pvalue > 6.38e-14)
        chi <- pvalueToChi(pvalue, degf)
    else
        chi <- sigmaToChiApprox(sigma, degf)
    chi
}

####################  chiToSigmaApprox  ##############
# linear extrapolation for very small p-values
# chi: chi value
# degf: degrees of freedom of chi distribution
######################################################
chiToSigmaApprox <- function(chi, degf)
{
    chi6p0 <- sigmaToChi(6.0,degf)
    chi7p5 <- sigmaToChi(7.5,degf)
    sigma <- 6.0 + 1.5 * (chi-chi6p0) / (chi7p5-chi6p0)
}

####################  chiToSigma  ####################
# compute sigma
# chi: chi value
# degf: degrees of freedom of chi distribution
######################################################
chiToSigma <- function(chi, degf)
{
    # compute pvalue based on chi value
    pvalue <- 1 - pchisq(chi*chi, degf)
    # compute sigma kniwing pvalue
    # 6.38e-14 is an empirical value
    sigma <- numeric( length(pvalue) )
    
    ok <- pvalue > 6.38e-14 & !is.na(pvalue)
    sigma[ok] <- pvalueToSigma(pvalue[ok])
    sigma[!ok] <- chiToSigmaApprox(chi[!ok],degf)
    sigma
}

###################  findOutliers  ###################
# sigma: distance to define outliers
# x: matrix (parts X tests)
######################################################
findOutliers <- function(x, sigma, maxcomponent)
{
    # omit row (parts) containing na value
    m <- na.omit(x)
    if ((ncol(m) == 0) | (nrow(m) == 0))
        return (list())

    # keep track of removed rows
    narows <- attr(m, "na.action")
    # PCA computation
    xp <- prcomp(m, tol = sqrt(.Machine$double.eps))$x
    dimxp <- ncol(xp)
    if ((maxcomponent > 0) & (maxcomponent < dimxp))
    {
        xp <- xp[,1:maxcomponent]
        dimxp <- ncol(xp)
    }
    # estimated covariance matrix
    covxp <- cov(xp,use="na.or.complete")
    meanxp <- colMeans(xp)
    # Mahalanobis distances computation
    md2 <- mahalanobis(xp, meanxp, covxp)
    # compute sqrt of each distances
    md <- sqrt(md2)
    # check each point with the threshold
    # compute threeshold based on sigma : sigma to chi
    chi <- sigmaToChi(sigma, dimxp)
    # list indexes of part with a bigger chi than threshold
    # compute z-scores
    zscores <- chiToSigma(md, dimxp)
    # build vector of valid rows
    ok <- setdiff( 1:nrow(x), narows)
    # MDF
    mdf <- vector(length=nrow(x))
    mdf[] <- NA
    mdf[ok] <- md
    # MD2F
    md2f <- vector(length=nrow(x))
    md2f[] <- NA
    md2f[ok] <- md2
    # XPF
    xpf <- matrix(nrow=nrow(x),ncol=dimxp)
    xpf[] <- NA
    xpf[ok,] <- xp[]
    # ZSCORESF
    zscoresf <- vector(length=nrow(x))
    zscoresf[] <- NA
    zscoresf[ok] <- zscores
    # -1 to fetch with 0-n-1 C arrays
    outliers <- which(mdf > chi) - 1
    narows <- narows - 1

    return (list(o=outliers, c=chi, m=mdf, m2=md2f, pc=xpf, z=zscoresf, s=sigma, nar=narows, npc=dimxp))
}
