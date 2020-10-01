#-------------------------------------------------------------------------------
# File name: model.py
# Created:   2013-06-04
# Author:    Jerome Kodjabachian
#
# This code is part of the Multivariate Outlier Analysis (MOA)
# for Part Average Testing (PAT) prototype. 
#
# (C) 2013 by Galaxy Semiconductor Inc. All rights reserved.
#-------------------------------------------------------------------------------

## @defgroup model Multivariate Algorithm Module

import math

import numpy as np
import scipy.stats as stats
from sklearn.covariance import EmpiricalCovariance
from sklearn.decomposition import PCA

def mahalanobisContribs(cov, observations):
        precision = cov.get_precision()
        # compute mahalanobis distance contributions
        centered_obs = observations - cov.location_
        mahalanobis_contribs = np.dot(centered_obs, precision) * centered_obs

        return mahalanobis_contribs


import moaerror, table, dataset

## A function to compute the value of the distance threshold.
#
#  If the user specified one of 'near', medium' or 'far' for the
#  distance criterion, this is replaced by a corresponding value.
#  By default, the value for 'near' is 6.0, the value for 'medium'
#  is 9.0 and the value for 'far' is 12.0, but other values can be
#  specified using the nearDistance, medimuDistance and farDistance
#  optional arguments.
#
#  If the user specified a numerical value, then the threshold is forced
#  to this value.
def distanceValue( distance, nearDistance = 6.0, mediumDistance = 9.0, farDistance = 12.0 ):

    if isinstance(distance, basestring):
        if distance == 'near':
            value = nearDistance
        elif  distance == 'medium':
            value = mediumDistance
        elif  distance == 'far':
            value = farDistance
        else:
            raise MoaError( "Unknown distance criterion '" + distance + "' found in model." ) 
    else:
        try:
            value = float(distance)
            print( "Warning! Distance value forced to " + str(value) + " in model. Consider using 'near', 'medium' or 'far' instead." ) 
        except ValueError:
            raise MoaError( "Unknown distance type found in model." )

    return value

def delta(x,x0):
    if abs(x - x0).max() == 0:
        return 0.0
    else:
        return 1e100 # easier than float('inf') when data are exported to CSV
    
## A class used to perform outlier detection.
## @ingroup model
#
#  Each instances of this class embeds an outlier detection
#  algorithm corresponding to a given type ('univariate' or
#  'multivariate') and method. Several methods are defined
#  for multivariate algorithms: 'emp' (Empirical Covariance),
#  'robust' (Robust Covariance), 'gmm' (Gaussian Mixture Model),
#  'svm' (ONe-Class Support Vector Machine).
#
#  IMPORTANT NOTE: The 'emp' method is the default and official
#  choice. The other multivariate algorithms have been implemented
#  for experimental purposes and must not be considered to constitute
#  sound solutions for multivariate PAT in the current state of art.
class Model:

    ## Create a new model (Constructor).
    def __init__( self, _type, _method = 'emp' ):
        if _type == 'univariate':
            self.algorithm     = AlgoUV()
        else: #     'multivariate'
            if   _method == 'svm':
                self.algorithm = AlgoSVM()
            elif _method == 'gmm':
                self.algorithm = AlgoGMM()
            elif _method == 'robust':
                self.algorithm = AlgoRobustCov()
            else: #         'emp'
                self.algorithm = AlgoEmpCov()                

    ## Return this model's state.
    def getState(self):
        return self.algorithm.state
    
    ## Apply the given rule to the given input set.
    #
    #  Fit the model using only good parts. Return main
    #  part scores and a list of outlier parts.
    #
    #  Return also transformed part scores (z-scores) and
    #  Principal Component contributions to the (main) score
    #  whenever applicable.
    def applyRule( self, inputset, rule, goods ):

        distance = distanceValue(rule['distance'])

        if rule['type'] == 'multivariate':
            n_components = rule['components']
        else:
            n_components = None

        testList    = map(lambda x:str(x['name']), rule['tests']) # get rid of JSON UTF encoding
        transfoList = map(lambda x:str(x['shape']), rule['tests']) # get rid of JSON UTF encoding
        #try:
        keptMeasures, keptPartsIdx, _ = inputset.extract( testList, transfoList )
        #except moaerror.MoaError:
        #    #print("Rule" + str(i+1) + " could not be applied - some tests may be missing in the input file.")
        #    print("Rule could not be applied - some tests may be missing in the input file.")
        #    return None, None, None, None

        if keptMeasures.shape[1] == 0:
            raise moaerror.MoaError("No test found for this rule")

        # fit model on good parts only
        badParts =  [j for j,p in enumerate(keptPartsIdx) if not p in goods]
        keptMeasuresFit, keptPartsIdxFit = dataset.removeParts(keptMeasures, keptPartsIdx, badParts)
        self.algorithm.fit( keptMeasuresFit, n_components )

        iterate = rule['type'] != 'multivariate' or ( rule['method'] != 'svm' and rule['method'] != 'gmm' )
        if iterate:

            while True:

                # compute scores
                newSc = self.algorithm.computeScores( keptMeasures )

                # compute outlier list
                newOutliers = self.algorithm.findOutliers( newSc, distance)

                # if no outlier, then stop
                if len(newOutliers) == 0:
                    break;

                # otherwise, remove outliers and iterate
                # $$$$ to avoid inconsistencies, recompute the outlier list at each iteration ?

                """
                # Keep only best outlier in order to remove outliers one by one
                bestScore = -1e100
                for outlier in newOutliers:
                    if newSc[outlier] > bestScore:
                        bestScore = newSc[outlier]
                        bestOutlier = outlier
                newOutliers = [bestOutlier]
                """
                keptMeasures, keptPartsIdx = dataset.removeParts(keptMeasures, keptPartsIdx, list(newOutliers));

                # (re)fit model on good parts only
                badParts =  [j for j,p in enumerate(keptPartsIdx) if not p in goods]
                keptMeasuresFit, keptPartsIdxFit = dataset.removeParts(keptMeasures, keptPartsIdx, badParts)
                self.algorithm.fit( keptMeasuresFit, n_components )

            keptMeasures, keptPartsIdx, _ = inputset.extract( testList, transfoList )

        # compute scores
        newSc = self.algorithm.computeScores( keptMeasures )
        scores = [float('nan')]*len(inputset.getParts())
        for k,kp in enumerate(keptPartsIdx):
            scores[kp] = newSc[k]

        # compute outlier list
        newOutliers = self.algorithm.findOutliers( newSc, distance)
        if len(newOutliers) > 0:
            outliers = [part for p,part in enumerate(inputset.getParts()) if p in map(lambda x:keptPartsIdx[x], list(newOutliers))]
        else:
            outliers = []

        zscores = [None]*len(inputset.getParts())
        scoreContribs = [None]*len(inputset.getParts())
        if rule['type'] == 'multivariate' and rule['method'] != 'svm' and rule['method'] != 'gmm':

            # Compute zscores  
            dim = self.getState()['dim']
            newZ = [table.tableManager.chiToSigma( x, dim) for x in newSc]
            zscores = [float('nan')]*len(inputset.getParts())
            for k,kp in enumerate(keptPartsIdx):
                zscores[kp] = newZ[k]

            #Compute PC contributions
            V = self.algorithm.computeMD2contributions( keptMeasures )
            if not V == None:
                scoreContribs = [float('nan')]*len(inputset.getParts())
                for k,kp in enumerate(keptPartsIdx):
                    scoreContribs[kp] = V[k]

        return scores, outliers, zscores, scoreContribs

## A class to implement a simple univariate outlier detection algorithm.
## @ingroup model
class AlgoUV:

    ## Create a new model (Constructor).
    def __init__( self ):

        self.type   = 'univariate'
        #self.method = 'emp'
        self.state  = {'mean':[],'std':[]}

    ## Fit this model using a given data matrix.
    def fit( self, X, n_components=None ):

        Xt = X.transpose() # 1 row by test, 1 column by part
        dim = Xt.shape[0] 
        self.state['mean'] = np.zeros(dim)
        self.state['std'] = np.zeros(dim)
        for d in range(dim): # compute stats by test
            self.state['mean'][d] = np.mean(Xt[d])
            self.state['std'][d] = np.std(Xt[d])

    ## Compute scores for a given data matrix.
    def computeScores( self, X ):

        scores = np.zeros(X.shape[0]) # all scores positive

        for n in range(X.shape[0]):
            row = X[n]
            for d in range(X.shape[1]):
                scores[n] = max(scores[n],abs((row[d]-self.state['mean'][d])/self.state['std'][d]))

        return scores

    ## Compute the score contributions of each Principal Component.
    def computeMD2contributions( self, X ):
        return None
                    
    ## Compute a list of outliers.
    def findOutliers( self, scores, distance ):
        
        self.state['threshold'] = distance   # distance is nbOfSigmas (positive)
        threshold = self.state['threshold']        

        return set([i for i in range(len(scores)) if scores[i]>threshold])

## An abstract class inherited by any class that implements a multivariate
#  outlier detection algorithm.
## @ingroup model
#
#  This class implements a preprocessor based on Principle Component Analysis.
#  The user specifies a desired number of components to keep.
class AlgoMV: # abstract

    ## Fit a PCA preprocessor.
    def fitPreprocessor( self, X, n_components = None ):

        # Print data rank
        #u, s, v = np.linalg.svd(X) # s, diagonal matrix
        #print np.sum(s > 1e-10), X.shape[1]

        if n_components == None or \
           abs(X.min(0) - X.max(0)).max() == 0:
            self.state['pca'] = None # no PCA
        else:
            self.state['pca'] = PCA(n_components=min(n_components,X.shape[1]))
            self.state['pca'].fit(X)

    ## Apply a PCA preprocessor.
    def applyPreprocessor( self, X ):

        if self.state['pca'] == None:
            Xp = X
        else:
            Xp = self.state['pca'].transform(X)

        return Xp

## A class to implement a multivariate outlier detection algorithm using Empirical
#  Covariance Estimation.
## @ingroup model
#
#  This is the official method proposed for PAT multivariate outlier analysis.
class AlgoEmpCov( AlgoMV ):

    ## Create a new model (Constructor).
    def __init__( self ):
        self.type   = "multivariate"
        self.method = "emp"
        self.state  = {'cov':None}

    ## Fit this model using a given data matrix.
    def fit( self, X, n_components=None ):

        # Preprocess data
        self.fitPreprocessor(X, n_components)
        Xp = self.applyPreprocessor(X)

        # Update model state
        self.state['cov'] = EmpiricalCovariance().fit(Xp)
        self.state['loc'] = self.state['cov'].location_
        self.state['dim'] = Xp.shape[1]

    ## Compute scores for a given data matrix.
    def computeScores( self, X ):

        Xp = self.applyPreprocessor( X )

        if abs(self.state['cov'].covariance_).max() == 0:
            #null covariance
            loc = self.state['loc']
            scores = [delta(X[i],loc) for i in range(X.shape[0])]
        else:
            MD2 = self.state['cov'].mahalanobis(Xp)
            scores = [math.sqrt(MD2[i]) for i in range(X.shape[0])]

        return scores

    ## Compute the score contributions of each Principal Component.
    def computeMD2contributions( self, X ):
        if self.state['pca'] != None:
            Xp = self.applyPreprocessor( X )        
            return mahalanobisContribs(self.state['cov'], Xp)
        return None
                    
    ## Compute a list of outliers.
    def findOutliers( self, scores, distance ):
        self.state['threshold'] = table.sigmaToChi( distance, self.state['dim'] )[0]    # distance is nbOfSigmas (unilateral)
        threshold = self.state['threshold']        
        return set([i for i in range(len(scores)) if scores[i]>threshold]) # $$$$ do we need a set or a list?

# Below, Alternative Multivariate Models (Experimental)

from sklearn import preprocessing, svm, mixture
from sklearn.covariance import MinCovDet

## A class to implement an experimental multivariate outlier detection algorithm using GMM.
## @ingroup model
class AlgoGMM( AlgoMV ):

    ## Create a new model (Constructor).
    def __init__( self ):
        self.type     = "multivariate"
        self.method   = "gmm"
        self.state = {'gmm':mixture.GMM(n_components=2, covariance_type="full")} # $$$$ need to whiten the data                

    ## Fit this model using a given data matrix.
    def fit( self, X, n_components=None ):

        self.fitPreprocessor(X, n_components)
        Xp = self.applyPreprocessor(X)

        self.state['scaler'] = preprocessing.StandardScaler().fit(Xp)
        X_scaled = self.state['scaler'].transform(Xp)
        self.state['gmm'].fit(X_scaled)

    ## Compute scores for a given data matrix.
    def computeScores( self, X ):

        Xp = self.applyPreprocessor( X )
        X_scaled = self.state['scaler'].transform(Xp)
        scores = self.state['gmm'].score(X_scaled)

        return scores

    ## Compute the score contributions of each Principal Component.
    def computeMD2contributions( self, X ):
        return None
                    
    ## Compute a list of outliers.
    def findOutliers( self, scores, distance ):

        self.state['threshold'] = stats.scoreatpercentile(scores, 99.5)  # distance is opposite of percentile
        threshold = self.state['threshold']        

        return set([i for i in range(len(scores)) if scores[i]>threshold])

## A class to implement an experimental multivariate outlier detection algorithm using One Class SVM.
## @ingroup model
class AlgoSVM( AlgoMV ):

    ## Create a new model (Constructor).
    def __init__( self ):
        self.type     = "multivariate"
        self.method   = "svm"
        self.state = {'svm':svm.OneClassSVM(nu=0.05, kernel="rbf", gamma=0.1)}

    ## Fit this model using a given data matrix.
    def fit( self, X, n_components=None ):

        self.fitPreprocessor(X, n_components)
        Xp = self.applyPreprocessor(X)

        self.state['scaler'] = preprocessing.StandardScaler().fit(Xp)
        X_scaled = self.state['scaler'].transform(Xp)
        self.state['svm'].fit(X_scaled)

    ## Compute scores for a given data matrix.
    def computeScores( self, X ):

        Xp = self.applyPreprocessor( X )
        X_scaled = self.state['scaler'].transform(Xp)
        scores = -self.state['svm'].decision_function(X_scaled).ravel()

        return scores

    ## Compute the score contributions of each Principal Component.
    def computeMD2contributions( self, X ):
        return None
                    
    ## Compute a list of outliers.
    def findOutliers( self, scores, distance ):
        
        self.state['threshold'] = stats.scoreatpercentile(scores, 99.5)  # distance is opposite of percentile
        threshold = self.state['threshold']        

        return set([i for i in range(len(scores)) if scores[i]>threshold])

## A class to implement an experimental multivariate outlier detection algorithm using Robust
#  Covariance Estimation.
## @ingroup model
class AlgoRobustCov( AlgoMV ):

    ## Create a new model (Constructor).
    def __init__( self ):
        self.type     = "multivariate"
        self.method   = "robust"
        self.state    = {'cov':None}

    ## Fit this model using a given data matrix.
    def fit( self, X, n_components=None ):

        self.fitPreprocessor(X, n_components)
        Xp = self.applyPreprocessor(X)

        self.state['cov'] = MinCovDet().fit(Xp)
        self.state['loc'] = self.state['cov'].location_
        self.state['dim'] = Xp.shape[1]

    ## Compute scores for a given data matrix.
    def computeScores( self, X ):

        Xp = self.applyPreprocessor( X )

        if abs(self.state['cov'].covariance_).max() == 0:
            #null covariance
            loc = self.state['loc']
            scores = [delta(X[i],loc) for i in range(X.shape[0])]
        else:
            MD2 = self.state['cov'].mahalanobis(Xp)
            scores = [math.sqrt(MD2[i]) for i in range(X.shape[0])]

        return scores

    ## Compute the score contributions of each Principal Component.
    def computeMD2contributions( self, X ):

        if self.state['pca'] != None:
            Xp = self.applyPreprocessor( X )        
            return mahalanobisContribs(self.state['cov'], Xp)
        
        return None
                    
    ## Compute a list of outliers.
    def findOutliers( self, scores, distance ):

        self.state['threshold'] = table.sigmaToChi( distance, self.state['dim'] )[0]    # distance is nbOfSigmas (unilateral)
        threshold = self.state['threshold']
        
        return set([i for i in range(len(scores)) if scores[i]>threshold])

