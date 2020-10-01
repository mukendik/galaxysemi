
options(bitmapType='cairo')

#############################  GSNormHistogram  ########################
#
########################################################################
GSNormHistogram <- function(x, sigmavalue, filepath, paramvect)
{
    size <- paramvect[[1]]
    
    if (paramvect[[2]] == "")
        mainTitle <- "Z-Scores Histogram"
    else
        mainTitle <- paramvect[[2]]
    if (paramvect[[3]] == "")
        xTitle <- "z-score"
    else
        xTitle <- paramvect[[3]]
    if (paramvect[[4]] == "")
        yTitle <- ""
    else
        yTitle <- paramvect[[4]]

    png(filepath, height = size[1], width = size[2])
    hist(x, 
         col = "blue",
         las = 1,
         prob = TRUE,
         nclass = 200, 
         axes = FALSE, 
         main = mainTitle,
         xlab = xTitle,
         ylab = yTitle)
    box()
    xnorm <- seq(0, 100, by = .1)
    lines(xnorm, dnorm(xnorm)*2, lwd = 1, col = "red")
    abline(v = sigmavalue, lty = "dotted", col = "green", lwd = 3)
    text(sigmavalue + 0.5, 0.5, bquote(.(sigmavalue)*sigma))

    dev.off()
    TRUE
}

#############################  GSTrendChart  ###########################
#
########################################################################
GSTrendChart <- function(x, sigmavalue, filepath, paramvect)
{
    size <- paramvect[[1]]
    xlabels <- paramvect[[2]]
    if (paramvect[[3]] == "")
        mainTitle <- "Z-Scores Trend Chart"
    else
        mainTitle <- paramvect[[3]]
    if (paramvect[[4]] == "")
        xTitle <- "run ID"
    else
        xTitle <- paramvect[[4]]
    if (paramvect[[5]] == "")
        yTitle <- "z-score"
    else
        yTitle <- paramvect[[5]]

    # get limits
    xmin <- -length(x)*(1/10)
    xmax <- length(x) + length(x)*(1/10)
    ymin <- 0
    ymax <- max(x, na.rm = TRUE)
    ymax <- ymax + ymax*(15/100)
    xlim = c(xmin, xmax)
    ylim = c(ymin, ymax)

    png(filepath, height = size[1], width = size[2])
    plot(seq_along(x),
         x, 
         las = 1,
         col = ifelse(x<sigmavalue,"blue","red"),
         pch = ifelse(x<sigmavalue,20,19),
         main = mainTitle,
         xlab = xTitle,
         ylab = yTitle,
         xaxs="r",
         yaxs="r",
         xlim = xlim,
         ylim = ylim)
    index <- seq_along(x)
    outliers <- which(x > sigmavalue)
    if (length(outliers) > 0)
        text(index[outliers], x[outliers], xlabels[outliers], font=1, pos = 4)
    box()
    abline(h = sigmavalue, lty = "dashed", col = "green", lwd = 2)
    text(0.5, sigmavalue + 0.5, bquote(.(sigmavalue) * sigma))

    dev.off()
    TRUE
}

#############################  GSQQPlot  ###############################
#
########################################################################
GSQQPlot <- function(x, sigmavalue, filepath, paramvect)
{
    size <- paramvect[[1]]
    if (paramvect[[2]] == "")
        mainTitle <- "Normal Q-Q Plot"
    else
        mainTitle <- paramvect[[2]]
    if (paramvect[[3]] == "")
        xTitle <- "theoretical quantiles"
    else
        xTitle <- paramvect[[3]]
    if (paramvect[[4]] == "")
        yTitle <- "sample quantiles"
    else
        yTitle <- paramvect[[4]]
    
    png(filepath, height = size[1], width = size[2])
    # transparency doesn't work correctly on ubuntu 10.04 64 bits
    qqplot(qnorm(ppoints(length(x))),
           x,
           col = rgb(0, 0, 255, 255, max = 255),
           pch = 19,
           main = mainTitle,
           xlab = xTitle,
           ylab = yTitle)
    qqline(x, col = "green", lwd = 2)

    dev.off()
    TRUE
}

#############################  GSCorrelationChart  #####################
#
########################################################################
GSCorrelationChart <- function(x, filepath, paramvect)
{
    size <- paramvect[[1]]
    xlabels <- paramvect[[2]]
    selectedcols <- paramvect[[3]]
    # apply offset to match with 1-to-n array
    col1 <- selectedcols[1]+1
    col2 <- selectedcols[2]+1
    outliers <- paramvect[[4]]
    if (paramvect[[5]] == "")
        mainTitle <- "Correlation Chart"
    else
        mainTitle <- paramvect[[5]]
    if (paramvect[[6]] == "")
        xTitle <- ""
    else
        xTitle <- paramvect[[6]]
    if (paramvect[[7]] == "")
        yTitle <- ""
    else
        yTitle <- paramvect[[7]]
    chisigma <- paramvect[[8]]
    chivalue <- chisigma[1]
    sigmavalue <- chisigma[2]

    png(filepath, height = size[1], width = size[2])
    gridsize <- 250
    marginrate <- (20/100)
    
    # get limits
    xmin <- min(x[,col1], na.rm = TRUE)
    xmax <- max(x[,col1], na.rm = TRUE)
    ymin <- min(x[,col2], na.rm = TRUE)
    ymax <- max(x[,col2], na.rm = TRUE)
    # x scope
    xscope <- abs(xmax - xmin)
    yscope <- abs(ymax - ymin)
    xmin <- xmin - marginrate * xscope
    xmax <- xmax + marginrate * xscope
    ymin <- ymin - marginrate * yscope
    ymax <- ymax + marginrate * yscope

    # graphics options
    xlab = xTitle
    ylab = yTitle
    xlim = c(xmin, xmax)
    ylim = c(ymin, ymax)

    if ((chivalue != 0) & (sigmavalue != 0))
    {
        # Draw an ellipse to show the mv limits
        # build grid
        seqx <- seq(xmin, xmax, length.out = gridsize)
        seqy <- seq(ymin, ymax, length.out = gridsize)
        xx <- rep(seqx, gridsize)
        yy <- rep(seqy, each = gridsize)
        zzflat <- cbind(xx,yy)
        # compute Mahalanobis distances for this grid
        xp <- na.omit(cbind(x[,col1], x[,col2]))
        covxp <- cov(xp, use = "na.or.complete")
        meanxp <- colMeans(xp)
        mdflat <- sqrt(mahalanobis(zzflat, meanxp, covxp))
        md <- matrix(mdflat, ncol = gridsize)
        # draw ellipse font
        image(seqx,
              seqy,
              md,
              col = terrain.colors(25),
              main = mainTitle,
              xlab = xlab,
              ylab = ylab,
              xlim = xlim,
              ylim = ylim)
        # draw limit
        contour(seqx,
                seqy,
                md,
                add = TRUE, 
                col = "red", 
                lwd = 2, 
                levels = c(chivalue),
                labels = c(sigmavalue),
                labcex = 1.2,
                lty = "dashed",
                vfont = c("sans serif", "plain"))
        # transparency doesn't work correctly on ubuntu 10.04 64 bits
        points(x[,col1],
               x[,col2],
               col = rgb(0, 0, 255, 255, max = 255),
               pch = 20,
               las = 1)
    }
    else
    {
        plot(x[,col1],
             x[,col2],
             col = rgb(0, 0, 255, 255, max = 255),
             pch = 20,
             las = 1,
             main = mainTitle,
             xlab = xTitle,
             ylab = yTitle,
             xlim = xlim,
             ylim = ylim)
    }
    if (length(outliers) > 0)
    {
        # draw outliers
        points(x[outliers+1,col1], x[outliers+1,col2], col = "red", pch = 20)
        # draw outliers labels
        text(x[outliers+1,col1], x[outliers+1,col2] , xlabels[outliers + 1], font = 1, pos = 4 )
    }

    dev.off()
    TRUE
}
