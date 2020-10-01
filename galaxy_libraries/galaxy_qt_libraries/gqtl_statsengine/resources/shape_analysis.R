############################################################
##           GENERAL INFO ON SCRIPT                     ####
##                                                      ####
##  Title   : Shape_Analysis_Function 1.3.4             ####
##  Owner   : Galaxy                                    ####
##  Modified: 2016-02-23                                ####
##  Authors : A.ARCHIMBAUD / F.BERGERET / C.SOUAL       ####
##  Company : IPPON Innovation                          ####
##  E-mail  : aurore.archimbaud@ippon-innov.eu          ####
##            francois.bergeret@ippon-innov.eu          ####
##            carole.soual@ippon-innov.eu               ####
##------------------------------------------------------####
##                 ARCHITECTURE                         ####
##  0 : Globals, Path & Function                        ####
##  1 : Function to estimate weibull's parameters       ####
##  2 : Function to get AIC criterium of fitted distri  ####
##  3 : Function to get local maxima on a distribution  ####
##  4 : Function to clean data removing outliers        ####
##  5 : Function to identify a distribution             ####
##  6 : Function to detect outliers                     ####
##                                                      ####
############################################################

############################################################
## 0-R PACKAGES                                         ####
############################################################

#install.packages("diptest")


options(bitmapType='cairo')

#############################  GSShapeNormHistogram  ########################
# Draw histogram
########################################################################
GSShapeNormHistogram <- function(x, filepath, paramvect)
{
    x <- na.omit(x)
    size <- paramvect[[1]]

    if (paramvect[[2]] == "")
        mainTitle <- "Histogram"
    else
        mainTitle <- paramvect[[2]]

    png(filepath, height = size[1], width = size[2])
    par(mfrow= c(1, 2))
    hist(x,
         col = "blue",
         las = 1,
         prob = TRUE,
         nclass = 200,
         axes = FALSE,
         main = mainTitle)
    qqnorm(x)
    box()
    xnorm <- seq(0, 100, by = .1)

    dev.off()
    TRUE
}

# libraries
library(diptest)  # Dip Test

############################################################
## 1-Function to estimate Weibull's parameters          ####
############################################################
# Note : Weibull is considered as Left-Skewed distribution
# Param[in] x vector of data
# Return list(alpha,beta) where alpha & beta are resp. shape
#and scale parameters of a Weibull distribution estimated on
#x data

estim_weibull=function(x)
{
    x_sort=sort(x)
    n = length(x)
    F_i=(1:n)/(n+1)
    Y_i=log(log(1/(1-F_i)))

    A=n*sum(log(x_sort)*Y_i)
    B=sum(log(x_sort))*sum(Y_i)
    C=n*sum((log(x_sort)^2))
    D=(sum(log(x_sort))^2)

    alpha=(A-B)/(C-D)

    ybar=1/n*sum(log(x_sort))
    xbar=1/n*sum(Y_i)
    beta=exp(ybar-xbar/alpha)

    return(list(alpha,beta))
}


#########################################################################
## 2-Function to get AIC criterium of fitted distribution            ####
#########################################################################
# Note : update from package fitdistrplus
# Param[in] data vector of data
# Param[in] distr string of distribution name
# Return[out] list(estimate,loglik,aic,bic,n,data,distname) where
# estimate is vector of estimated parameters by matching moments
# loglik is log-likelihood of the fit of distributions
# aic and bic are quality criterium of the fit
# n is number of data
# data are initial dataset
# distname is string of distribution name fitted

fitdist = function (data, distr)
{

  distname <- distr
  ddistname <- paste("d", distname, sep = "")

  n <- length(data)

  # Calculate Estimates of theorical distribution parameters by "moments" method
  if (distname == "norm") {
    m <- mean(data)
    s <- sd(data)
    estimate <- c(mean = m, sd = s)
  }
  if (distname == "weibull") {
    est.w = estim_weibull(data)
    estimate <- c(est.w[[1]], est.w[[2]])
  }
  if (distname == "lnorm") {
    ddistnam = "dnorm"
    ml <- mean(log(data))
    sl <- sd(log(data))
    estimate <- c(meanlog = ml, sdlog = sl)
  }

  # Function to compute log-likelihood
  loglik <- function(par, fix.arg=NULL, obs, ddistnam) {
    sum(log(do.call(ddistnam, c(list(obs), as.list(par),as.list(fix.arg)))))
  }

  # Get log-likelihood value
  loglik <- loglik(par = estimate, obs = data, ddistnam = ddistname)

  # Calculate AICc and BIC : criterium of fit quality
  npar <- length(estimate)
  aic <- -2 * loglik + 2 * npar
  bic <- -2 * loglik + log(n) * npar

  # Construct result to return
  reslist <- list(estimate = estimate,  loglik = loglik, aic = aic,
                  bic = bic, n = n, data = data, distname = distname)

  return(reslist)
}

############################################################
## 3-Function to get local maxima density on a distribution#
############################################################
# Param[in] x0 vector of data
# Return modes_006_list vector of x0 estimated values from
#density and matching highest modes of x0

localMaxima <- function(x0)
{
    #Get local maxima of density
    d = density(x0)
    x = d$y
    y <- diff(c(-.Machine$integer.max, x)) > 0L
    y <- cumsum(rle(y)$lengths)
    y <- y[seq.int(1L, length(y), 2L)]
    if (x[[1]] == x[[2]])
    {
        y <- y[-1]
    }
    lm = y
    lm.n = length(lm)

    # Method based on an 18-breaks histogram
    min=min(x0)
    max=max(x0)
    # Method based on an 18-breaks histogram
    classes=seq(from=min,to=max,by=(max-min)/18)
    H=hist(x0,breaks=classes,plot=F)
    H_eff=rbind(H$mids,H$counts) # mids and effectives

    #A mode is significant if there is at least one "empty" class between this mode [j-1] and the next one [j]
    modes_list = d$x[lm] #from local maxima

    nb1_0_ind = vector()
    nb2_0_ind = vector()
    nb_0_modes = vector()
    for (j in 2:lm.n)
    {
        m1 = modes_list[j-1]
        m2 = modes_list[j]
        #Get indexes (from histo 18 classes) of closest bounds for local maxima [j-1,j]
        b1 = max(which(H_eff[1,]<=m1),1)
        b2 = min(which(H_eff[1,]>=m2),18)

        #Get "empty" classes between the 2 local maxima
        #By computing the number of classes containing lower than half the minimum of the effectives of the local maxima
        empty_classes = which(H_eff[2,b1:b2]<min(H_eff[2,b1],H_eff[2,b2])/2)

        #Get index of the first "empty" class (reference is histo 18 classes)
        nb1_0_ind[j-1] = empty_classes[1] + b1 -1

        #Get index of the last "empty" class for the same local maxima (reference is histo 18 classes)
        if(length(empty_classes)==0)
        {
            nb2_0_ind[j-1] = NA
        }
        else
        {
            nb2_0_ind[j-1] = empty_classes[length(empty_classes)] + b1 - 1
        }

        #Get number of "empty" classes (reference is histo 18 classes)
        nb_0_modes[j-1]=length(empty_classes)
    }

    # Get indexes of modes with at least one "empty" class between this mode [j-1] and the next one [j]
    modes_ind = which(nb_0_modes!=0)
    # Get number of local maxima
    lm.n = length(modes_ind)

    modes_006_list = c(1)
    bimodal_split=NA
    if(lm.n>0)
    {
        #Get non missing index of the "empty" first classes
        nb1_0_ind =  nb1_0_ind[which(!is.na(nb1_0_ind))]

        #Get non missing index of the "empty" last classes
        nb2_0_ind =  nb2_0_ind[which(!is.na(nb2_0_ind))]

        #Get new mode values and effectives (initialize for the 1st mode)
        modes_list_count = sum(H_eff[2,1:(nb1_0_ind[1]-1)])
        modes_list_val = sum(H_eff[1,1:(nb1_0_ind[1]-1)])/length(1:(nb1_0_ind[1]-1))
        #Loop on each non missing index of "empty" class (which separates 2 modes)
        split_values = vector()
        for (k in 1:length(nb1_0_ind))
        {
            split_values[k] = (H_eff[1,nb1_0_ind[k]] + H_eff[1,nb2_0_ind[k]])/2

            #Test if last "empty" class
            if(k==length(nb1_0_ind))
            {
                mode_count  = sum(H_eff[2,(nb2_0_ind[k]+1):18])
                mode_val    = sum(H_eff[1,(nb2_0_ind[k]+1):18])/length((nb2_0_ind[k]+1):18)
            }
            else
            {
                mode_count = sum(H_eff[2,(nb2_0_ind[k]+1):(nb1_0_ind[k+1]-1)])
                mode_val   = sum(H_eff[1,(nb2_0_ind[k]+1):(nb1_0_ind[k+1]-1)]) /length((nb2_0_ind[k]+1):(nb1_0_ind[k+1]-1))
            }
            #Increment mode values and effectives in vectors
            modes_list_count = c(modes_list_count,mode_count)
            modes_list_val = c(modes_list_val,mode_val)
        }

        #Get effective in percentage (in fact ratio between 0 and 1)
        modes_list_count = modes_list_count/length(x0)
        lm.n = length(modes_list_val)

        #If there is still more than one mode
        #Check if it contains more than 6% of data
        if(lm.n>0)
        {
            #Get indexes of our new modes containing more than 6% of data
            nb_006 = which(modes_list_count > 0.06)

            if (length(nb_006) == 2) #If exactly 2 modes
            {
                bimodal_split = split_values[nb_006[1]]
            }

            #Keep the value of modes containing more than 6% of data
            modes_006_list = modes_list_val[nb_006]
        }
    }

    return(list(modes_006_list,bimodal_split))
}

############################################################
## 3-Function to clean data removing outliers           ####
############################################################
# Param[in] x vector of data
# Return x.bxp.3pct vector of x with some values removed if too
#far from main distribution

clean_data_for_distri <- function(x) {
  x_ini = x[!is.na(x)]
  n_ini=length(x_ini)

  # Get quantiles
  Q=quantile(x_ini)
  Q3=Q[4]
  Q1=Q[2]
  IQR=Q3-Q1

  # Outliers identification
  # We delete only if outside whiskers of boxplot
  limit_sup_box=Q3+3*IQR
  limit_inf_box=Q1-3*IQR

  #Subset x
  x1 = x_ini[which(x_ini < limit_inf_box)]
  x2 = x_ini[which(x_ini > limit_sup_box)]

  #Outliers of boxplot
  x.outlier.bxp = c(x1,x2)

  #Test if there are outliers outside boxplot
  if(length(x.outlier.bxp)>0){

    #Calculate distance from boxplot limits of each outlier
    x1.dist = limit_inf_box - x1
    x2.dist = x2 - limit_sup_box

    #Calculate rank of distance from boxplot limits of each outlier
    x1.dist.rk = order(order(x1.dist,decreasing = T))
    x2.dist.rk = order(order(x2.dist,decreasing = T))


    #Merge ranks in one vecor of boxplot outliers distance
    x.dist.rk = c(x1.dist.rk, x2.dist.rk)

    #Get 3% of outlier having the distance max
    #Get location of outliers
    loc.outlier.3pct = order(x.dist.rk,decreasing = F)[1:floor(0.03*n_ini)]
    loc.outlier.3pct = loc.outlier.3pct[!is.na(loc.outlier.3pct)]

    #Get outlier values
    x.outlier.bxp.3pct = x.outlier.bxp[loc.outlier.3pct]

    #Get final vector of values without outliers boxplot & 3pct
    x.bxp.3pct = c(x.outlier.bxp[-loc.outlier.3pct],
                   x_ini[which((x_ini >= limit_inf_box)&(x_ini <= limit_sup_box))])

  }else{

    #No outliers
    x.bxp.3pct = x_ini

  }

  return(x.bxp.3pct)
}

############################################################
## 4-Function to identify a distribution                ####
############################################################
# Note: main fonction, algorithm to identify the distribution
# Param[in] x0 vector of data
# Return distrib vector of 2 values containing the name of
# x0 distribution and the level confidence of the prediction

shape_analysis=function(x0, distinctValues0)
{
    # Create object to be returned by the function
    distrib=vector()

    # Check if x is categorical or constant
    if (distinctValues0==1)
    {
        distrib[1]="Constant"
        distrib[2]=5
    }
    else
    {
      # Clean data from extreme outliers
      x = clean_data_for_distri(x0)

      distinctValues = length(unique(x))
      if(distinctValues<=5)
      {
        distrib[1]="Categorical"
        distrib[2]=5
      }
      else
      {
        #x is not categorical nor constant
        #Initialization
        lm.n=1
        dip_n2 = 0
        distri_KS=""
        #If less than 2% of distinct values : multimodal test
        if ((distinctValues <= (0.02 * length(x))) & (distinctValues < 50))
        {
            distri_KS="Multimodal"
        }
        else
        {
            # Dip Test : check if distribution is unimodal or not
            dip_n2 = suppressMessages(dip.test(x)$p.value) # hide message about asymptotic value with suppressMessages
            if(dip_n2<0.05)  # if not unimodal
            {
                # Check the number of modes thanks to the density function
                lm=localMaxima(x) # the modes
                lm.n=length(lm[[1]]) # the number of modes
                lm.bs=lm[[2]]
            }

            if ((dip_n2<0.05)&(lm.n>=2))
            {
                # Distribution is Multimodal-Other if there is more than 2 modes
                distri_KS="Multimodal-Other"
                distri_KS_pval=dip_n2
                # Check for BIMODAL/ MULTIMODAL / CLAMPED
                min=min(x)
                max=max(x)
                # Method based on an 18-breaks histogram
                classes=seq(from=min,to=max,by=(max-min)/18)
                H=hist(x,breaks=classes,plot=F)
                H_eff=rbind(H$mids,H$counts) # mids and effectives
                # Check if CLAMPED
                nb_0=sum(H_eff[2,]==0)# check ig=f there is at least one empty class

                if (nb_0==0)
                { # if no empty class
                    max_eff=max(H_eff[2,]) # the maximum size
                    clamped=0

                    if(H_eff[2,1]>max_eff/2)
                    {
                        distri_KS="LeftClamped"
                        distri_KS_pval=H_eff[2,1]/max_eff
                        clamped=clamped+1
                    }
                    if (H_eff[2,ncol(H_eff)]>max_eff/2)
                    {
                        distri_KS="RightClamped"
                        distri_KS_pval=H_eff[2,ncol(H_eff)]/max_eff
                        clamped=clamped+1
                    }
                    if (clamped==2)
                    {
                        distri_KS="DualClamped"
                        distri_KS_pval=(H_eff[2,1]/max_eff+H_eff[2,ncol(H_eff)]/max_eff)/2
                    }
                }

                # Check for MULTIMODALITY / BIMODALITY
                if(lm.n>=2 & distri_KS=="Multimodal-Other" )# for not taking account for CLAMPED
                {
                    distri_KS="Multimodal"
                    distri_KS_pval=dip_n2
                    if (lm.n==2 ) # if only two modes : bimodal
                    {
                        distri_KS="Bimodal"
                        distri_KS_pval=dip_n2
                    }
                }
            }
        }#End If less than 2% of distinct values : multimodal test

        #Check unimodal or global distribution
        if ((dip_n2>=0.05)||(lm.n<2)||(distri_KS=="Multimodal"))
        {
            # Get estimation of parameters for normal distribution
            mu=mean(x)
            sigma=sd(x)

            # KS Test for Normality
            ks_n_T = suppressWarnings(ks.test(x,"pnorm",m=mu,s=sigma))
            ks_n = ks_n_T$p.value
            ks_n_S = ks_n_T$statistic

            # Get if normality is significant
            if(ks_n>0.01)
            {
                if (distri_KS=="Multimodal")
                {
                    distri_KS="Multimodal-Normal"
                }
                else
                {
                    distri_KS="Normal"
                }
                distri_KS_pval=ks_n
            }
            else
            {
                # Test if data are positive
                pos_data=1
                # if data are negative translate the vector by min(x)
                if(sum(x<=0)!=0)
                {
                    min_x=min(x)
                    x_trans = x - min_x + 1
                    pos_data=0
                }

                # KS test : Log Normal
                ks_ln_T = NA
                ks_ln = NA

                if(pos_data==1)
                {
                    ks_ln_T = suppressWarnings(ks.test(log(x),"pnorm",m=mean(log(x)),s=sd(log(x)) ))
                    ks_ln = ks_ln_T$p.value
                    ks_ln_S = ks_ln_T$statistic
                }else
                {
                    ks_ln_T = suppressWarnings(ks.test(log(x_trans),"pnorm",m=mean(log(x_trans)),s=sd(log(x_trans)) ))
                    ks_ln = ks_ln_T$p.value
                    ks_ln_S = ks_ln_T$statistic
                }

                # Estimation of Weibull parameters
                if (pos_data==1)
                {
                    estim=estim_weibull(x)
                }
                else
                {
                    estim=estim_weibull(x_trans)
                }
                alpha=estim[[1]]
                beta=estim[[2]]


                # KS test : Weibull
                ks_w_T = NA
                ks_w = NA

                if(pos_data==1)
                {
                    ks_w_T = suppressWarnings( ks.test(x,"pweibull", shape=alpha,scale=beta))
                    ks_w = ks_w_T$p.value
                    ks_w_S = ks_w_T$statistic
                }else
                {
                    ks_w_T = suppressWarnings(ks.test(x_trans,"pweibull", shape=alpha,scale=beta))
                    ks_w = ks_w_T$p.value
                    ks_w_S = ks_w_T$statistic
                }

                # Get AIC criterion in case of all p.value of KS test reject distribution
                # fit (confidence level == 1)
                if((ks_w <= 0.001) && (ks_n <= 0.001) && (ks_ln <= 0.001) ){
                  if(pos_data==1){
                    fdw = try(fitdist(x,"weibull"))
                    fdn = try(fitdist(x,"norm"))
                    fdln = try(fitdist(x,"lnorm"))
                  }else
                  {
                    fdw = try(fitdist(x_trans,"weibull"))
                    fdn = try(fitdist(x_trans,"norm"))
                    fdln = try(fitdist(x_trans,"lnorm") )
                  }
                  if(!(inherits(fdw, "try-error")) && (!inherits(fdn, "try-error")) &&
                       (!inherits(fdln, "try-error")))
                  {
                    ks_w_S = fdw$aic
                    ks_n_S = fdn$aic
                    ks_ln_S = fdln$aic
                  }
                }

                # Results : Lognormal or Weibull (Right-Skewed or Left-Skewed)
                distri_names=c("Normal","Right-Skewed","Left-Skewed")
                pvalues=c(ks_n,ks_ln,ks_w)
                pvalue_max=max(pvalues)
                if(sum(pvalues==pvalue_max)>1)
                {
                    stats=c(ks_n_S,ks_ln_S,ks_w_S)
                    stats_min=min(stats)
                    distri_KS_uni=distri_names[which(stats==stats_min)]
                    if(distri_KS=="Multimodal")
                    {
                        distri_KS=paste("Multimodal-",distri_KS_uni[1],sep="")
                    }else
                    {
                        distri_KS=distri_KS_uni[1]
                    }
                    distri_KS_pval=pvalue_max
                }else
                {
                    distri_KS_uni=distri_names[which(pvalues==pvalue_max)]
                    if(distri_KS=="Multimodal")
                    {
                        distri_KS=paste("Multimodal-",distri_KS_uni,sep="")
                    }else
                    {
                        distri_KS=distri_KS_uni
                    }
                    distri_KS_pval=pvalue_max
                }
                #Test if Symmetric or Multimodal-Symmetric (Other)
                if(distri_KS == "Left-Skewed" || distri_KS == "Multimodal-Left-Skewed")
                {
                    if(alpha < 10){
                      distri_KS = "Other"
                    }
                }
            }
        }
        distrib[1]=distri_KS
        distrib[3]=NA
        # Compute level confidence
        #In case of Bimodal distribution (dip test pvalue)
        if(distri_KS=="Bimodal")
        {
            distrib[2]=1+(distri_KS_pval<10^(-3))+(distri_KS_pval<10^(-4))+(distri_KS_pval<10^(-5))+(distri_KS_pval<10^(-6))
            distrib[3]=lm.bs #Get bimodal split value
        #In case of Clamped distribution
        }
        else if(distri_KS%in%c("LeftClamped","RightClamped","DualClamped"))
        {
            distrib[2]=(distri_KS_pval>0.5)+(distri_KS_pval>0.6)+(distri_KS_pval>0.7)+(distri_KS_pval>0.8)+(distri_KS_pval>0.9)

          #In case of Normal, Left-Skewed, Right-Skewed, Multimodal or Other distribution (kolmogorov test pvalue)
        }
        else
        {
            distrib[2]=(distri_KS_pval>=0)+(distri_KS_pval>0.001)+(distri_KS_pval>0.01)+(distri_KS_pval>0.05)+(distri_KS_pval>0.1)
        }

    }#End if x is not categorical nor constant

    }#End if x is not constant

  # Returns name of identified distribution and its level confidence
  return(distrib)
}

############################################################
## 5-Function to detect outliers                        ####
############################################################
# Param[in] x0 vector of data
# Param[in] distri the name of x0 distribution
# Return outlier vector of 6 values containing for 2 methods
#the lower and the upper limits to detect outliers and the
#pct of detected outliers for each method
#Note: you can uncomment other calculation methods if needed

outlier_detection=function(x0, distri){

  x_ini = x0[!is.na(x0)]
  n_ini=length(x_ini)

  #Values by default
    #outlier.m1 = c(NA,NA,0)
    #outlier.m2 = c(NA,NA,0)
    outlier.m3 = c(NA,NA,0)
    #outlier.m4 = c(NA,NA,0)
    outlier.m5 = c(NA,NA,0)

  if(distri != "Constant" & distri != "Categorical"){

    # Get quantiles
    Q=quantile(x_ini)
    med = Q[3]
    Q3=Q[4]
    Q1=Q[2]
    IQR=Q3-Q1

    if(distri == "Left-Skewed" || distri == "Multimodal-Left-Skewed"){
      #method 1
      #outlier.lowerlimit = mean(x_ini) - 6*sd(x_ini)
      #outlier.upperlimit = mean(x_ini) + 4.5*sd(x_ini)
      #outlier.pct        = (sum(x_ini<outlier.lowerlimit)+sum(x_ini>outlier.upperlimit))*100/n_ini
      #outlier.m1         = c(outlier.lowerlimit,outlier.upperlimit,outlier.pct)

      #method 2
      #outlier.lowerlimit = med - 6*(Q3-Q1)/1.35
      #outlier.upperlimit = med + 4.5*(Q3-Q1)/1.35
      #outlier.pct        = (sum(x_ini<outlier.lowerlimit)+sum(x_ini>outlier.upperlimit))*100/n_ini
      #outlier.m2         = c(outlier.lowerlimit,outlier.upperlimit,outlier.pct)

      #method 3
      outlier.lowerlimit = med - 6*(med-Q1)/0.67
      outlier.upperlimit = med + 4.5*(Q3-med)/0.67
      outlier.pct        = (sum(x_ini<outlier.lowerlimit)+sum(x_ini>outlier.upperlimit))*100/n_ini
      outlier.m3         = c(outlier.lowerlimit,outlier.upperlimit,outlier.pct)

      #method 4
      #outlier.lowerlimit = med - 6*(med - quantile(x_ini,0.01))/2.33
      #outlier.upperlimit = med + 4.5*(quantile(x_ini,0.99) - med)/2.33
      #outlier.pct        = (sum(x_ini<outlier.lowerlimit)+sum(x_ini>outlier.upperlimit))*100/n_ini
      #outlier.m4         = c(outlier.lowerlimit,outlier.upperlimit,outlier.pct)

      #method 5
      outlier.lowerlimit = med - 6*(med - quantile(x_ini,0.05))/1.645
      outlier.upperlimit = med + 4.5*(quantile(x_ini,0.95) - med)/1.645
      outlier.pct        = (sum(x_ini<outlier.lowerlimit)+sum(x_ini>outlier.upperlimit))*100/n_ini
      outlier.m5         = c(outlier.lowerlimit,outlier.upperlimit,outlier.pct)

    } else if (distri == "Right-Skewed" || distri == "Multimodal-Right-Skewed"){

      #method 1
      #outlier.lowerlimit = mean(x_ini) - 4.5*sd(x_ini)
      #outlier.upperlimit = mean(x_ini) + 6*sd(x_ini)
      #outlier.pct        = (sum(x_ini<outlier.lowerlimit)+sum(x_ini>outlier.upperlimit))*100/n_ini
      #outlier.m1         = c(outlier.lowerlimit,outlier.upperlimit,outlier.pct)

      #method 2
      #outlier.lowerlimit = med - 4.5*(Q3-Q1)/1.35
      #outlier.upperlimit = med + 6*(Q3-Q1)/1.35
      #outlier.pct        = (sum(x_ini<outlier.lowerlimit)+sum(x_ini>outlier.upperlimit))*100/n_ini
      #outlier.m2         = c(outlier.lowerlimit,outlier.upperlimit,outlier.pct)

      #method 3
      outlier.lowerlimit = med - 4.5*(med-Q1)/0.67
      outlier.upperlimit = med + 6*(Q3-med)/0.67
      outlier.pct        = (sum(x_ini<outlier.lowerlimit)+sum(x_ini>outlier.upperlimit))*100/n_ini
      outlier.m3         = c(outlier.lowerlimit,outlier.upperlimit,outlier.pct)

      #method 4
      #outlier.lowerlimit = med - 4.5*(med - quantile(x_ini,0.01))/2.33
      #outlier.upperlimit = med + 6*(quantile(x_ini,0.99) - med)/2.33
      #outlier.pct        = (sum(x_ini<outlier.lowerlimit)+sum(x_ini>outlier.upperlimit))*100/n_ini
      #outlier.m4         = c(outlier.lowerlimit,outlier.upperlimit,outlier.pct)

      #method 5
      outlier.lowerlimit = med - 4.5*(med - quantile(x_ini,0.05))/1.645
      outlier.upperlimit = med + 6*(quantile(x_ini,0.95) - med)/1.645
      outlier.pct        = (sum(x_ini<outlier.lowerlimit)+sum(x_ini>outlier.upperlimit))*100/n_ini
      outlier.m5         = c(outlier.lowerlimit,outlier.upperlimit,outlier.pct)

    } else {

      #method 1
      #outlier.lowerlimit = mean(x_ini) - 6*sd(x_ini)
      #outlier.upperlimit = mean(x_ini) + 6*sd(x_ini)
      #outlier.pct        = (sum(x_ini<outlier.lowerlimit)+sum(x_ini>outlier.upperlimit))*100/n_ini
      #outlier.m1         = c(outlier.lowerlimit,outlier.upperlimit,outlier.pct)

      #method 2
      #outlier.lowerlimit = med - 6*(Q3-Q1)/1.35
      #outlier.upperlimit = med + 6*(Q3-Q1)/1.35
      #outlier.pct        = (sum(x_ini<outlier.lowerlimit)+sum(x_ini>outlier.upperlimit))*100/n_ini
      #outlier.m2         = c(outlier.lowerlimit,outlier.upperlimit,outlier.pct)

      #method 3
      outlier.lowerlimit = med - 6*(med-Q1)/0.67
      outlier.upperlimit = med + 6*(Q3-med)/0.67
      outlier.pct        = (sum(x_ini<outlier.lowerlimit)+sum(x_ini>outlier.upperlimit))*100/n_ini
      outlier.m3         = c(outlier.lowerlimit,outlier.upperlimit,outlier.pct)

      #method 4
      #outlier.lowerlimit = med - 6*(med - quantile(x_ini,0.01))/2.33
      #outlier.upperlimit = med + 6*(quantile(x_ini,0.99) - med)/2.33
      #outlier.pct        = (sum(x_ini<outlier.lowerlimit)+sum(x_ini>outlier.upperlimit))*100/n_ini
      #outlier.m4         = c(outlier.lowerlimit,outlier.upperlimit,outlier.pct)

      #method 5
      outlier.lowerlimit = med - 6*(med - quantile(x_ini,0.05))/1.645
      outlier.upperlimit = med + 6*(quantile(x_ini,0.95) - med)/1.645
      outlier.pct        = (sum(x_ini<outlier.lowerlimit)+sum(x_ini>outlier.upperlimit))*100/n_ini
      outlier.m5         = c(outlier.lowerlimit,outlier.upperlimit,outlier.pct)

    }

  }

  #Return list of limits and percentage of detected outliers
  outlier = c(outlier.m3,outlier.m5)
  return(outlier)
}


