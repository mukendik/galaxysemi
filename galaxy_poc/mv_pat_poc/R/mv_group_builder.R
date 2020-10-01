

####################  findPredictor  ##################
## A function that returns candidate predictor variables for a given
#  dependent variable.
#  The candidate predictor variables are the parameter from a list
#  whose correlation with the dependent variable is above a given
#  minimum correlation threshold.
######################################################
findPredictors2 <- function(rmat, pmat, params, test, threshold)
{
    # dynamically allocate is item is bad:  better
    group = list()
    for(i in params)
    {
        if(test != i)
        {
            r = rmat[test, i]
            p = pmat[test, i] 
            if ((!is.na(p)) && (!is.na(r)) && (p < 0.01) && (abs(r) >= threshold))
            {
                if (length(group) == 0)
                    group = list(c(p, r, test, i))
                else
                    group[[length(group) + 1]] = c(p, r, test, i)
            }
        }
    }
    return (group)
}

findPredictors1 <- function(rmat, pmat, params, test, threshold)
{
    # dynamically allocate items is bad: better to allocate more than needed
    ######### NEW
    group = lapply(params,
                    function(i)
                    {
                        if(test != i)
                        {
                            r = rmat[test, i]
                            p = pmat[test, i] 
                            if ((!is.na(p)) && (!is.na(r)) && (p < 0.01) && (abs(r) >= threshold))
                                return (c(p, r, test, i))
                        }
                    }
                 )

    
    Filter( Negate( is.null), group )
}

findPredictors <- function(rmat, pmat, params, test, threshold)
{
    # dynamically allocate items is bad: better to allocate more than needed
    ######### NEW
    group = lapply(params,
                    function(i)
                    {
                        if(test != i)
                        {
                            r = rmat[test, i]
                            p = pmat[test, i] 
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
findGroup1 <- function(groups, param)
{
    groupIndex=-1
    if (length(groups) > 0)
    {
        for(g in 1:length(groups))
        {
            for(h in 1:length(groups[g]))
            {
                #print(groups[g][h])
                if (groups[[g]][[h]][4] == param)
                {
                    return (g)
                }
            }
        }
    }
    
    return (groupIndex)
}

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
        group1[[length(group1) + 1]] = group2[[i]]
    return (group1)
}

####################  computeCorrMat  ##################
# Calculates a Pearson correlation coefficient and 
# the p-value for testing non-correlation.
# replace corr.test from "psych" package
######################################################
computeCorrMat <- function(x)
{
    nParams = ncol(x)
    rmat = matrix(1, nParams, nParams, dimnames=list(colnames(x), colnames(x)))
    pmat = matrix(1, nParams, nParams, dimnames=list(colnames(x), colnames(x)))
    # bench with rprof
    for(i in 1:(nParams-1)) 
    {
        for(j in (i+1):nParams)
        {
          
            corrMat =  suppressWarnings( cor.test(x[,i],x[,j]) )
            # corrMat = withCallingHandlers( cor.test(x[,i],x[,j]), warning = function(w){})
            rmat[i,j] = corrMat$estimate
            rmat[j,i] = rmat[i,j]
            pmat[i,j] = corrMat$p.value
            pmat[j,i] = pmat[i,j]
        }
    }

    return (list(r=rmat, p=pmat))
} 


####################  groupBuilder  ##################
# build groups of tests based on pearson's correlation
# x: data matrice (nb parts X nb tests)
# threshold: correlation min threshold to group tests
######################################################
groupBuilder <- function(x, threshold)
{
    print('groupBuilder')
    print(paste('rows: ', nrow(x)))
    print(paste('cols: ', ncol(x)))
    x = na.omit(x)
    nParams = ncol(x)
    # compute correlation and pvalue
    corrMat = computeCorrMat(x)
    # p-value matrix in  corrMat$p
    # corr matrix in corrMat$r
    params = 1:nParams
    groups = list()
    for(i in 1:(nParams-1))
    {
        group = findPredictors(corrMat$r, corrMat$p, seq(i+1,nParams), i, threshold)
        if (length(group) > 0)
        {
            group = append(group, i)
            # check if other groups contain i
            # if yes join with it
            groupIndex = findGroup(groups, i)
            if (groupIndex != -1)
            {
                groups[[groupIndex]] = unique(append(groups[[groupIndex]], group))
            }
            # else append this new group to group list
            else
            {
                if (length(groups) > 0)
                    groups[[length(groups) + 1]] = group
                else
                    groups[[1]] = group
            }
        }
    }
    
#    for (i in 1:length(groups))
#    {
#        nameList = numeric()
#        for(j in groups[[i]])
#        {
#            if (is.na(match(j[3], nameList)))
#                nameList = c(nameList, j[3])
#            if (is.na(match(j[4], nameList)))
#                nameList = c(nameList, j[4])
#        }
#        print(paste('group',i,':'))
#        #print(names(x[nameList]))
#        print(nameList)
#    }
    
    return (groups)
    #return (corrMat$r)
}

# Load formated csv file
#ds1 <- read.csv("E:/en_cours/multivariate/R/formated_data/ds1.csv", header=TRUE, stringsAsFactors=FALSE)
#rst <- groupBuilder(ds1[,2:30], 0.8)
#mvgroup_out_1 = groupBuilder(mvgroup_in_1[2:29,], 0.8)
#result2 <- 10

