
####################  findPredictor  ##################
## A function that returns candidate predictor variables for a given
#  dependent variable.
#  The candidate predictor variables are the parameter from a list
#  whose correlation with the dependent variable is above a given
#  minimum correlation threshold.
######################################################
findPredictors <- function(rmat, pmat, params, test, threshold)
{
    # dynamically allocate items is bad: better to allocate more than needed
    group <- lapply(params,
                    function(i)
                    {
                        if(test != i)
                        {
                            r <- rmat[test, i]
                            p <- pmat[test, i] 
                            if ((!is.na(p)) && (!is.na(r)) && (p < 0.01) && (abs(r) >= threshold))
                                return (i)
                        }
                    }
                 )

    Filter(Negate(is.null), group)
}

####################  findGroup  ##################
## Return the group index of the group containing param
# if exists, -1 otherwise
######################################################
findGroup <- function(groups, param)
{
    if (length(groups) > 0)
    {
        for(g in 1:length(groups))
        {
            if (param %in% groups[[g]])
                return (g)
        }
    }
    
    return (-1)
}

####################  mergeGroups  ##################
# Merge two list of groups in one list of groups
######################################################
mergeGroups <- function(group1, group2)
{
    for (i in 1:length(group2))
        group1[[length(group1) + 1]] <- group2[[i]]
    return (group1)
}

####################  computeCorrMat  ##################
# Calculates a Pearson correlation coefficient and 
# the p-value for testing non-correlation.
# replace corr.test from "psych" package
######################################################
computeCorrMat <- function(x)
{
    nParams <- ncol(x)
    rmat <- matrix(1, nParams, nParams, dimnames=list(colnames(x), colnames(x)))
    pmat <- matrix(1, nParams, nParams, dimnames=list(colnames(x), colnames(x)))
    # bench with rprof
    for(i in 1:(nParams-1)) 
    {
        for(j in (i+1):nParams)
        {
            corrMat <-  suppressWarnings( cor.test(x[,i],x[,j]) )
            # corrMat = withCallingHandlers( cor.test(x[,i],x[,j]), warning = function(w){})
            rmat[i,j] <- corrMat$estimate
            rmat[j,i] <- rmat[i,j]
            pmat[i,j] <- corrMat$p.value
            pmat[j,i] <- pmat[i,j]
        }
    }

    return (list(r=rmat, p=pmat))
} 


####################  offsetGroupIndex  ##################
# apply offset of -1 to group index
######################################################
offsetGroupIndex <- function(groups)
{
    for (i in 1:length(groups))
    {
        for (j in 1:length(groups[[i]]))
        {
            groups[[i]][[j]] <- as.integer(groups[[i]][[j]] - 1)
        }
    }
    return (groups)
}

####################  extractFrom  ###################
# Extract a sublist from a list starting from a given 
# index
# startPoint: index to start the sublist
# params: full list
######################################################
extractFrom <- function(startPoint, params)
{
    subParams <- lapply(params,
                    function(i)
                    {
                        if(i > startPoint)
                        {
                            return (i)
                        }
                    }
                 )

    Filter(Negate(is.null), subParams)
}

####################  removeItems  ###################
# remove item from a given list
# items: items to remove
# originalList: list containing items to remove
######################################################
removeItems <- function(items, originalList)
{
    newList <- lapply(originalList,
                    function(i)
                    {
                        if(!(i %in% items))
                        {
                            return (i)
                        }
                    }
                 )

    Filter(Negate(is.null), newList)
}

####################  groupBuilder  ##################
# build groups of tests based on pearson's correlation
# x: data matrice (nb parts X nb tests)
# threshold: correlation min threshold to group tests
######################################################
groupBuilder <- function(x, threshold)
{
    x <- na.omit(x)
    nParams <- ncol(x)
    # compute correlation and pvalue
    corrMat <- computeCorrMat(x)
    # p-value matrix in  corrMat$p
    # corr matrix in corrMat$r
    params <- 1:nParams
    groups <- list()
    i <- 1
    while (i <= length(params))
    {
        group <- findPredictors(corrMat$r, corrMat$p, extractFrom(i, params), params[[i]], threshold)
        if (length(group) > 0)
        {
            # append i to its predictors
            group <- append(group, params[[i]])
            # check if other groups contain i
            # if yes join with it
            groupIndex <- findGroup(groups, params[[i]])
            if (groupIndex != -1)
            {
                groups[[groupIndex]] <- unique(append(groups[[groupIndex]], group))
            }
            # else append this new group to group list
            else
            {
                if (length(groups) > 0)
                    groups[[length(groups) + 1]] <- group
                else
                    groups[[1]] <- group
            }
            params <- removeItems(group, params)
        }
        else
            i <- i + 1
    }

    return (offsetGroupIndex(groups))
}

