#-------------------------------------------------------------------------------
# File name: table.py
# Created:   2013-07-02
# Author:    Jerome Kodjabachian
#
# This code is part of the Multivariate Outlier Analysis (MOA)
# for Part Average Testing (PAT) prototype. 
#
# (C) 2013 by Galaxy Semiconductor Inc. All rights reserved.
#-------------------------------------------------------------------------------

import math, pickle, pickletools
import scipy.stats as stats

import moaerror

## @defgroup stats Statistical Table Module

## A function to convert a Chi value of given degree-of-freedom
#  into a p-value.
## @ingroup stats
def chiToPvalue( chi, dim ):
    return 1-stats.chi2.cdf(chi**2,dim)

## A function to convert a p-value into a Chi value of given
#  degree-of-freedom.
## @ingroup stats
def pvalueToChi( p, dim ):
    return math.sqrt(stats.chi2.ppf(1-p,dim))

## A function to convert a Mahalanobis Distance of given dimension
#  into a Z-Score.
## @ingroup stats
#
# The function assumes that the Mahalanobis distance follows a Chi
# squared distribution of degree-of-freedom equal to the dimension.
def chiToSigma( chi, dim ):
    p = chiToPvalue(chi,dim)
    if p > 6.38e-14:
        nbsig = pvalueToChi(p,1)
    else: # linear extrapolation for very small p-values
        chi6p0 = sigmaToChi(6.0,dim)[0]
        chi7p5 = sigmaToChi(7.5,dim)[0]
        nbsig = 6.0 + 1.5 * (chi-chi6p0) / (chi7p5-chi6p0)
    return nbsig, p

## A method to convert a Z-Score into a Mahalanobis Distance of
#  given dimension.
## @ingroup stats
#
# The function assumes that the Mahalanobis distance follows a Chi
# squared distribution of degree-of-freedom equal to the dimension.
def sigmaToChi( nbsig, dim ):
    p = chiToPvalue(nbsig,1)
    if p > 6.38e-14:
        chi = pvalueToChi(p,dim)
    else: # linear extrapolation for very small p-values
        chi6p0 = sigmaToChi(6.0,dim)[0]
        chi7p5 = sigmaToChi(7.5,dim)[0]
        chi = chi6p0 + (chi7p5-chi6p0) * (nbsig-6.0)/ 1.5   
    return chi, p

## A class of cached converters that transform a Mahalanobis Distance value
#  into a Z-Score value.
## @ingroup stats
#
#  This class uses a look-up table to optimize the computation time. The
#  table is stored as a file in a cache directory, which is required to exist.
class ChiToSigma:

    ## Create a converter for the given dimension (Constructor).
    #
    #  If a file with the required look-up table cannot be found in
    #  the cache directory, the table is computed and saved into the
    #  cache. So, it may take several seconds to create the first
    #  converter instance for a given dimension.
    def __init__( self, dim ):
        try:
            self.unpickle( dim )
        except IOError:
            print "Computing Table ... Starting (ChiToSigma, dim %d)" % dim
            self.version = '1.0.0'
            self.table = []
            self.dim = dim
            self.chi6p0 = sigmaToChi(6.0,dim)[0]
            self.chi7p5 = sigmaToChi(7.5,dim)[0]
            for chi in range(int(1000*self.chi7p5)+1):
                self.table.append(chiToSigma(chi/1000.,dim)[0])
            print "Computing Table ... Done"
            self.pickle( dim )

    ## Convert the given Mahalanobis Distance value into a Z-Score value.
    def get( self, chi ):
        if chi < self.chi7p5:
            return self.table[int(1000*chi)+1]
        else:
            return 6.0 + 1.5 * (chi-self.chi6p0) / (self.chi7p5-self.chi6p0)

    ## Save this converter as a binary file in the cache directory.
    def pickle( self, dim ):

        output = open('cache/'+str(dim)+'.cts', 'wb')

        pickle.dump(self.version,  output)
        pickle.dump(pickletools.optimize( pickle.dumps(self.table) ), output)
        pickle.dump(self.dim,      output)
        pickle.dump(self.chi6p0,   output)
        pickle.dump(self.chi7p5,   output)

        output.close()

    ## Load this converter from a binary file in the cache directory.
    def unpickle( self, dim ):
        
        pkl_file = open('cache/'+str(dim)+'.cts', 'rb')

        self.version  = pickle.load(pkl_file)

        if (self.version == '1.0.0'): # compressed representation for table
            self.table    = pickle.loads(pickle.load(pkl_file))
            self.dim      = pickle.load(pkl_file)
            self.chi6p0   = pickle.load(pkl_file)
            self.chi7p5   = pickle.load(pkl_file)
            
        elif (self.version == '0.1.0'):
            print "Warning! Expecting version number 1.0.0 in cached table file"
            self.table    = pickle.load(pkl_file)
            self.dim      = pickle.load(pkl_file)
            self.chi6p0   = pickle.load(pkl_file)
            self.chi7p5   = pickle.load(pkl_file)
            
        else:
            raise moaerror.MoaError( "Wrong version number found in cached table file (expecting 1.0.0)" )
        
        pkl_file.close()

## A class of cached converters that transform a Z-Score value into a
#  Mahalanobis Distance value.
## @ingroup stats
#
#  This class uses a look-up table to optimize the computation time. The
#  table is stored as a file in a cache directory, which is required to exist.
class SigmaToChi:

    ## Create a converter for the given dimension (Constructor).
    #
    #  If a file with the required look-up table cannot be found in
    #  the cache directory, the table is computed and saved into the
    #  cache. So, it may take several seconds to create the first
    #  converter instance for a given dimension.
    def __init__( self, dim ):
        try:
            self.unpickle( dim )
        except IOError:
            print "Computing Table ... Starting (SigmaToChi, dim %d)" % dim
            self.version = '1.0.0'
            self.table = []
            self.dim = dim
            self.chi6p0 = sigmaToChi(6.0,dim)[0]
            self.chi7p5 = sigmaToChi(7.5,dim)[0]
            for nbsigma in range(int(1000*7.5)+1):
                self.table.append(sigmaToChi(nbsigma/1000.,dim)[0])
            print "Computing Table ... Done"
            self.pickle( dim )

    ## Convert the given Z-Score value into a Mahalanobis Distance value.
    def get( self, nbsigma ):
        if nbsigma < 7.5:
            return self.table[int(1000*nbsigma)+1]
        else:
            return self.chi6p0 + (self.chi7p5-self.chi6p0) * (nbsig-6.0) / 1.5

    ## Save this converter as a binary file in the cache directory.
    def pickle( self, dim ):

        output = open('cache/'+str(dim)+'.stc', 'wb')

        pickle.dump(self.version,  output)
        pickle.dump(pickletools.optimize( pickle.dumps(self.table) ), output)
        pickle.dump(self.dim,      output)
        pickle.dump(self.chi6p0,   output)
        pickle.dump(self.chi7p5,   output)

        output.close()

    ## Load this converter from a binary file in the cache directory.
    def unpickle( self, dim ):
        
        pkl_file = open('cache/'+str(dim)+'.stc', 'rb')

        self.version  = pickle.load(pkl_file)
        
        if (self.version == '1.0.0'): # compressed representation for table
            self.table    = pickle.loads(pickle.load(pkl_file))
            self.dim      = pickle.load(pkl_file)
            self.chi6p0   = pickle.load(pkl_file)
            self.chi7p5   = pickle.load(pkl_file)

        elif (self.version == '0.1.0'):
            print "Warning! Expecting version number 1.0.0 in cached table file"
            self.table    = pickle.load(pkl_file)
            self.dim      = pickle.load(pkl_file)
            self.chi6p0   = pickle.load(pkl_file)
            self.chi7p5   = pickle.load(pkl_file)

        else:
            raise moaerror.MoaError( "Wrong version number in cached table file (expecting 1.0.0)" )
        
        pkl_file.close()

## A class that simplifies the use of cached converters.
## @ingroup stats
#
#  Users of the cached converter classes do not need to
#  manipulate these classes directly. Instead they can use
#  the methods provided in this class.
class TableManager:

    ## Create a cached converter manager (Constructor).
    def __init__( self ):
        self.stcInstances = {}
        self.ctsInstances = {}

    ## Converts a Z-Score value into a Mahalanobis Distance value
    #  of given dimension.
    def sigmaToChi( self, nbsigma, dim ):
        if not dim in self.stcInstances.keys():
            self.stcInstances[dim] = SigmaToChi( dim )
        return self.stcInstances[dim].get( nbsigma )

    ## Converts a Mahalanobis Distance value of given dimension
    #  into a Z-Score value.
    def chiToSigma( self, chi, dim ):
        if not dim in self.ctsInstances.keys():
            self.ctsInstances[dim] = ChiToSigma( dim )
        return self.ctsInstances[dim].get( chi )
            
## A global table manager.
tableManager = TableManager() # $$$$ should implement a singleton pattern
