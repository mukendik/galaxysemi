#-------------------------------------------------------------------------------
# File name: grouper.py
# Created:   2013-06-04
# Author:    Jerome Kodjabachian
#
# This code is part of the Multivariate Outlier Analysis (MOA)
# for Part Average Testing (PAT) prototype. 
#
# (C) 2013 by Galaxy Semiconductor Inc. All rights reserved.
#-------------------------------------------------------------------------------

## @defgroup grouper Group Formation Module

import dataset

from scipy.stats.stats import pearsonr

import math, warnings
import numpy as np

## A function to merge intersecting groups and remove empty groups.
#
#  Transformation information must be present.
def cleanGroupList( groups ):

    def code( group ):
        return map( lambda x:x['name'] + ':' + x['shape'], group )

    nbgroups = len(groups)
    for g in range(nbgroups-1):
        for h in range(g+1,nbgroups):
            if len(set(code(groups[g]))&set(code(groups[h]))) > 0:
                groups[h].extend([x for x in groups[g] if not (x['name']+':'+x['shape']) in code(groups[h]) ]) # no duplicate tests
                groups[g] = []

    groups = [g for g in groups if len(g)>0] # no empty groups
    return groups

## A function that returns a correlation matrix for a given
#  list of variables.
## @ingroup grouper
#
# Each cell in the returned 2D array contains 2 value: r, the
# correlation coefficient and p, the p-value for testing non
# correlation (p is the probability to observe a coefficient at
# least as high as r with uncorrelated data).
def computeCorrMat( measures, params ):

    N = len(params)
    corrMat =  np.ones((N,N))
    probMat =  np.zeros((N,N))

    with warnings.catch_warnings():

        warnings.simplefilter("ignore") # suppress warning issued on 'nan' correlations

        for i in range(N-1):
            for j in range(i+1,N):
                m = measures.T[[params[i],params[j]],:].T
                m = m[~np.isnan(m).any(axis=1)]
                if m.shape[0] == 0: # All entries have nans in either vector
                    r,p = float( 'nan' ),float( 'nan' )
                else:
                    r,p = pearsonr(m.T[0],m.T[1])
                corrMat[i][j] = corrMat[j][i] = r
                probMat[i][j] = probMat[j][i] = p

    return corrMat,probMat

## A function that returns candidate predictor variables for a given
#  dependent variable.
## @ingroup grouper
#
#  The candidate predictor variables are the parameter from a list
#  whose correlation with the dependent variable is above a given
#  minimum correlation threshold.
def findPredictors( rmat, pmat, params, dependent, minCorr=0.8 ):

    corrs = []
    """ before bug fix 
    for j in range(len(params)):
        if j == dependent:
            continue
        r = rmat[dependent][j]# may return nan if either variable is constant
        p = pmat[dependent][j]
        corrs.append( [ p, r, dependent, j ])"""
        
    for j in range(len(params)):
        if params[j] == dependent:
            continue
        r = rmat[dependent][params[j]]# may return nan if either variable is constant
        p = pmat[dependent][params[j]]
        corrs.append( [ p, r, dependent, params[j] ])
    
    #print corrs
    s = sorted( [x for x in corrs if x[0]<1e-2 and abs(x[1])>=minCorr], key=lambda x:abs(x[1]), reverse=True)
    # $$$$ is this sorting useful?

    return s

## A class to extract groups of correlated tests.
## @ingroup grouper
#
# Using a set of input data and given a correlation threshold, a grouper
# extracts groups of tests (columns) with high correlation. Several grouping
# methods can be defined. In this implementation, a test is added to a group
# if it has a correlation higher than the given threshold with at least one
# other test in the group.
class Grouper:

    ## Create a new grouper (Constructor).
    #
    #  A grouper computes and stores groups from a set of input data and a
    #  correlation threshold.
    #
    #  If the user does not specify a correlation threshold, a default value
    #  of 0.8 is used.
    def __init__( self, inputset, minCorr = 0.8, tests = None, transfos = None, grouping='loose'):

        if tests == None:
            tests = inputset.getTests()
            transfos = ['normal']*len(tests)
        keptMeasures, keptPartsIdx, keptTests = inputset.extract( tests, transfos, filterNans=False )
        print len(keptPartsIdx), len(keptTests)

        assert( len(tests) == len(keptTests) )
        params = range(len(tests)) # $$$$ ceci suppose que tous les tests sont conserves par l'extract?
                                   # garanti par le filterNans = False

        params2 = [t for t,test in enumerate(tests) if test in keptTests]
        r,p = computeCorrMat(keptMeasures, params2)
        #print r
        #print p
        self.groups = []
        for dependent in params:
            corrs = findPredictors( r,p, range(dependent+1,len(params)), dependent, minCorr=minCorr )

            group = set( map( lambda c:corrs[c][3], range(len(corrs)) ) )
            group.add(dependent)
            testGroup = set(map(lambda x:tests[x], list(group)))

            if grouping == 'loose':
                g = self.findGroup( tests[dependent] )
                if g >= 0:
                    self.groups[g] = self.groups[g].union( testGroup )
                else:
                    self.groups.append( testGroup )
            else:
                print 'Unknown grouping strategy in Grouper'
                # $$$$ may implement other grouping methods - for instance, all pairs
                # in a group have a correlation higher than threshold
            """
            elif grouping == 'full':
                g = self.findGroup( tests[dependent] ) ## $$$$ may find several
                if check all corrs:
                    self.groups[g] = self.groups[g].union( testGroup )
                else:
                    self.groups.append( testGroup )
            """
            # $$$$ should add transfos in groups
        
    ## Find a group that contains the given test and return its index or
    #  return -1 when no group is found.
    #
    #  A grouper is associated with a set of input data. If the user does not
    #  specify a correlation threshold, a default value of 0.8 is used.
    def findGroup( self, test ):
        for g, group in enumerate(self.groups):
            if test in group:
                return g # return the *first* group containing test
        return -1

    ## Get the groups computed by this grouper.
    def getGroups( self ):
        return self.groups

    ## Print the correlation matrix.
    @staticmethod
    def printCorrMatrix( inputset, testSelection, transfoSelection, minCorr = None, showProba = False ):
        tests = inputset.getTests()
        transfos = ['normal']*len(tests) # $$$$ transfos not managed
        params = [t for t,test in enumerate(tests) if test in testSelection]
        keptMeasures, keptPartsIdx, keptTests = inputset.extract( tests, transfos, filterNans=False )
        assert( len(tests) == len(keptTests) )
        r,p = computeCorrMat(keptMeasures, params)
        s = max( [ len(test) for test in testSelection ] )
        if showProba:
            for i in range(len(params)):
                st = '%(t)20s:' % {'t':tests[params[i]]}
                for j in range(0,i):
                    if minCorr==None or r[i][j] > minCorr:
                       st += ',' + '%(a).3f' % {'a':p[i][j]}
                    else:
                       st += ',  x  '
                for j in range(i,len(params)):
                    st += ',  -  '
                print st
            print
        for i in range(len(params)):
            st = eval( "'%(t)" + str(s) + "s:\\t' % " + "{'t':tests[params[i]]}" )
            #st = '%(t)20s:' % {'t':tests[params[i]]}
            for j in range(0,i):
                if minCorr==None or r[i][j] > minCorr:
                   st += ',' + '%(a).3f' % {'a':r[i][j]}
                else:
                   st += ',  x  ' # '   -  '
            for j in range(i,len(params)):
                st += ',  -  ' # '   -  '
            print st
