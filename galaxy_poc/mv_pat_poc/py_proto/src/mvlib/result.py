#-------------------------------------------------------------------------------
# File name: result.py
# Created:   2013-06-04
# Author:    Jerome Kodjabachian
#
# This code is part of the Multivariate Outlier Analysis (MOA)
# for Part Average Testing (PAT) prototype. 
#
# (C) 2013 by Galaxy Semiconductor Inc. All rights reserved.
#-------------------------------------------------------------------------------

## @defgroup result Multivariate Result Module

import csv, pickle

import pylab as pl

import model, moaerror


import numpy as np
import scipy.stats as stats

def computeOutliers( recipe, inputset, outputName ):

    sbin = inputset.findInfos( 'SBIN' )
    hbin = inputset.findInfos( 'HBIN' )
    goodParts = [p for p,part in enumerate(inputset.getParts()) if int(sbin[p]) == 1]

    results = Result(inputset.getParts(), inputset.getTests(), goods=goodParts)
    results.setSbin( sbin )
    results.setHbin( hbin )

    for r in recipe.getRules():

        rule = r.getContents()
        i = results.addRule( rule )
        if rule['enabled']:
            results.addResult( inputset, i )
        results.printRuleSummary( i )
        #results.printScoreContributionSummary( i )

    # Assign all outliers to MVBIN
    results.computeMvbin()
    results.printMvbinSummary()

    try:
        results.saveScoresToCSV( outputName, inputset.getInfos(), inputset.getLogistics(), delimiter=',' )
        results.pickle( outputName )
    except IOError:
        raise moaerror.MoaError("Could not save results. File " + outputName + ".csv may be open.")

"""
def normalTest( measures, cut=0.01, a=1, b=2, c=1 ):
    # $$$$ can't use this if the sign of z-score is lost. Use Chi2 with dof=1 instead.
    measurements = np.ma.masked_invalid(measures).compressed()
    nbcut = int(len(measurements) * cut / 2.0 )
    measurements = sorted(measurements)[nbcut:len(measurements)-nbcut]
    if len(measurements) < 3:
        print 'not enough valid data to graph.'
        return
    sp1 = pl.subplot(a, b, c)
    try:
        stats.probplot(measurements, dist="norm", plot=pl)
    except ValueError:
        print "Value Error"
    except ZeroDivisionError:
        print "Zero Division Error"
    sp2 = pl.subplot(a, b, c+1)
    try:
        sp2.hist(np.square(measurements), 100, zorder=1)
        X = range(0,100)
        mi = min(np.square(measurements))
        ma = max(np.square(measurements))
        X = map(lambda x:mi+(ma-mi)*(float(x)+0.5)/100.0, X)
        K = float(len(measurements))*(ma-mi)/100.0
        Y = map(lambda x:K*stats.chi2.pdf(x,1), X)
        sp2.plot( X, Y, color='r', zorder=2)
    except ValueError:
        print "Value Error"
    except AttributeError:
        print "Attribute Error"
    pl.title('Histogram')
"""

def chiSquaredTest( measures, dim, cut=0.01, a=1, b=2, c=1 ):
    measurements = np.square(np.ma.masked_invalid(measures).compressed())
    nbcut = int(len(measurements) * cut / 2.0 )
    measurements = sorted(measurements)[nbcut:len(measurements)-nbcut]
    if len(measurements) < 3:
        print 'not enough valid data to graph.'
        return
    sp1 = pl.subplot(a, b, c)
    try:
        stats.probplot(measurements, sparams = dim, dist="chi2", plot=pl)
    except ValueError:
        print "Value Error"
    except ZeroDivisionError:
        print "Zero Division Error"
    sp2 = pl.subplot(a, b, c+1)
    try:
        sp2.hist(measurements, 100, zorder=1)
        X = range(0,100)
        mi = min(measurements)
        ma = max(measurements)
        X = map(lambda x:mi+(ma-mi)*(float(x)+0.5)/100.0, X)
        K = float(len(measurements))*(ma-mi)/100.0
        Y = map(lambda x:K*stats.chi2.pdf(x,dim), X)
        sp2.plot( X, Y, color='r', zorder=2)
    except ValueError:
        print "Value Error"
    except AttributeError:
        print "Attribute Error"
    pl.title('Histogram')


## A class used to store the results of a multivariate analysis.
## @ingroup result
class Result:

    ## Create a result set (Constructor).
    def __init__( self, parts = [], tests = [], goods = None, bads = [] ):
        self.version  = '1.0.0'
        self.rules    = []
        self.mvbin    = []
        self.hbin     = [float('nan')]*len(parts)
        self.sbin     = [float('nan')]*len(parts)
        self.scores   = []
        self.scoreContribs  = []
        self.zscores  = []
        self.outliers = []
        self.models   = []
        self.parts    = parts
        self.tests    = tests
        if goods == None:
            self.goods = range(len(parts))
        else:
            self.goods = goods
        self.bads     = bads

    ## Applies the rule associated with the given entry to the given input set
    #  and fill the entry with the results.
    def addResult( self, inputset, i ):

        rule = self.rules[i]

        # create and store a model acording to the entry's rule
        mod = model.Model( rule['type'], rule.get('method', None) )
        #mod = model.ModelEmpCov()
        #mod = model.ModelRobustCov()
        #mod = model.ModelSVM()
        #mod = model.ModelGMM()
        #mod = model.ModelUV()
        self.setModel(i, mod)

        # compute and store the rules outputs
        try:
            scores, outliers, zscores, scoreContribs = mod.applyRule( inputset, rule, self.getGoods() )
        except moaerror.MoaError:
            print("Rule" + str(i+1) + " could not be applied - tests may be missing from the input file.")
        else:              
            self.setScores(i, scores )
            self.setOutliers( i, outliers )
            self.setZscores( i, zscores )
            self.setScoreContribs( i, scoreContribs )

    ## Return the number of result entries that have been filled.
    def nbRules( self ):
        return len(self.rules )

    ## Create an entry for the given rule in this result set.
    def addRule( self, rule ):
        self.rules.append( rule )
        self.scores.append( [float('nan')]*len(self.parts) )
        self.scoreContribs.append( [None]*len(self.parts) )
        self.zscores.append( [float('nan')]*len(self.parts) )
        self.outliers.append( [] )
        self.models.append( None )

        return len(self.rules)-1 # return the new entry index

    ## Set the (main) score for the given rule and part.
    def setScore( self, rule, part, score):
        self.scores[rule][part] = score
        
    ## Set the (main) score for the given rule.
    def setScores( self, rule, scores):
        self.scores[rule] = scores
        
    ## Set the Principal Component contributions to main score for the
    #  given rule and part.
    def setScoreContrib( self, rule, part, scoreContrib):
        self.scoreContribs[rule][part] = scoreContrib

    ## Set the Principal Component contributions to main score for the
    #  given rule.
    def setScoreContribs( self, rule, scoreContribs):
        self.scoreContribs[rule] = scoreContribs

    ## Set the z-score for the given rule and part.
    #
    #  This is used when the main score can be transformed
    #  into a Z-Score.
    def setZscore( self, rule, part, zscore):
        self.zscores[rule][part] = zscore

    ## Set the z-scores for the given rule.
    #
    #  This is used when the main score can be transformed
    #  into a Z-Score.
    def setZscores( self, rule, zscores):
        self.zscores[rule] = zscores

    ## Adds the given parts to the (global) list of bad parts.
    def extendBads( self, bads):
        self.bads.extend( bads )

    ## Set the list of outlier parts for the given rule.
    def setOutliers( self, rule, outliers):
        self.outliers[rule] = outliers

    ## Adds the given parts to the list of outlier parts for the given rule.
    def extendOutliers( self, rule, outliers):
        self.outliers[rule].extend( outliers )

    ## Set the list of outlier parts for the given rule.
    def setModel( self, rule, model):
        self.models[rule] = model

    ## Get the description of the given rule.
    def getRule( self, rule ):
        return self.rules[rule]

    ## Get the (main) scores for all rules.
    def getScores( self ):
        return self.scores
        
    ## Get the Principle Component contributions to main score for all rules.
    def getScoreContribs( self ):
        return self.scoreContribs

    ## Get the Z-Scores for all rules.
    def getZscores( self ):
        return self.zscores

    ## Get the list of outlier parts for the given rule.
    def getOutliers( self, rule ):
        return self.outliers[rule]

    ## Get the (global) list of good parts.
    def getGoods( self ):
        return self.goods

    ## Get the model for the given rule.
    def getModel( self, rule):
        return self.models[rule]

    ## Set the (global) list of parts' software bins.
    def setSbin( self, sbin):
        self.sbin = sbin

    ## Set the (global) list of parts' hardware bins.
    def setHbin( self, hbin):
        self.hbin = hbin

    ## Compute the multivariate bins for the given rules.
    #
    #  If no list of rules is specified, the multivariate
    #  bins are computed for all rules.
    def computeMvbin( self, rules = None):

        if rules == None:
            rules = range(self.nbRules())
            
        self.mvbin = [0]*len(self.parts)
        
        for rule in rules:
            outs = self.getOutliers(rule)

            for p,part in enumerate(self.parts):
                if part in outs:
                    self.mvbin[p] = rule+1 # The latest rule overrides all the previous ones

    ## Print the Principal Component contributions to main score.
    def printScoreContributions( self, name, V ):
        if V != None:
            MD2 = sum(V,1)
            s = name + ": MD2 score = %0.2f [\n" % MD2
            for j,v in sorted(enumerate(V),key=lambda x:x[1],reverse=True):
                percent = (100*v/MD2)
                if percent >= 0.05:
                    s = s + "\t\tPC"+str(j+1)+"=%0.1f%%\n" % percent
            s = s + '\t]'
            print s
        # could be just a function. No need for a method here

    ## Print the Principal Component contributions to main score for all outliers found by the given rule.
    def printScoreContributionSummary( self, rule ):
        outs = self.getOutliers(rule)
        for p,part in enumerate(self.parts):
            if part in outs:
                self.printScoreContributions( part, self.getScoreContribs()[rule][p] )

    ## Print a summary of this set o results.
    def printSummary( self ):
        print
        print '== RESULT SUMMARY ===================================='
        for rule in range(self.nbRules()):
            self.printRuleSummary(rule)
            self.printScoreContributionSummary(rule)
            print
        self.printMvbinSummary()
        print '======================================================'
        print
        
    ## Print a summary of the results for the given rule.
    def printRuleSummary( self, rule, verbose=False ):
        msg = 'Rule ' + \
               str(rule+1) + \
               ': ' + \
               str(len(self.getOutliers(rule))) + \
               ' outliers'
        if len(self.getOutliers(rule)) > 0:
            msg = msg + ' -> ' + self.getOutliers(rule)[0]
        for o in self.getOutliers(rule)[1:]:
            #msg = msg + ', ' + o[0]
            msg = msg + ', ' + o
        if verbose:
            msg = msg + \
               ' (' + \
               self.rules[rule]['type'] + \
               ', ' + \
               str(map(lambda x:x['name'],self.rules[rule]['tests'])) + \
               ')'
        print( msg )

    ## Compute a statistical summary of the multivariate analysis.
    def computeMvbinSummary( self, bs=[], uvbin=None ):
        N = len(self.mvbin)
        if len(bs) == 0:
            n = sum(map(lambda x:int(x>0),self.mvbin))
        else:
            n=[]
            for b in bs:
                n.append( sum(map(lambda x:int(x==b),self.mvbin)) )

        goods = [p for p in range(N) if int(self.sbin[p]) == 1]
        if uvbin:
            goods = [p for p in range(N) if p in goods and uvbin[p] == 0]
        N1 = len(goods)
        if len(bs) == 0:
            n1 = sum(map(lambda x:int(self.mvbin[x]>0),goods))
        else:
            n1 = []
            for b in bs:
                n1.append( sum(map(lambda x:int(self.mvbin[x]==b),goods)) )

        return N, n, N1, n1

    ## Print a statistical summary of the multivariate analysis.
    def printMvbinSummary( self ):
        N, n, N1, n1 = self.computeMvbinSummary( [] )
        percent = 100.0*n/N
        print( 'Overall number of parts analysed:\t' + str(N))
        print( 'Overall number of parts in MVBIN:\t' + str(n) + ' (' + '%(a).2f' % {'a':percent} +'%)')

        percent1 = 100.0*n1/N1
        print( 'Number of good parts analysed (SBIN 1):\t' + str(N1))
        print( 'Number of good parts in MVBIN (SBIN 1):\t' + str(n1) + ' (' + '%(a).2f' % {'a':percent1} +'%)')

    ## Print data relative to a given part.
    def printPidData( self, pid ):

        mvbin = [self.mvbin[p] for p,part in enumerate(self.parts) if part == pid] 
        assert( len(mvbin) == 1 )
        print( 'PID: ' + str(pid) + ', MVBIN: ' + str(mvbin[0]) )

    ## Display the score distribution graphically.
    #
    #  The scores of parts are plotted and a horizontal line detect shows the
    #  threshold used to identify outliers.
    def displayScore( self, rule, parts ):

        x = range(len(self.scores[rule]))
        y = self.scores[rule]

        outa = self.outliers[rule]
        outb = [(p,part) for p,part in enumerate(parts) if part in outa]
        out = [o for (o,p) in outb]
        textout = [p for (o,p) in outb]
        xout = map(lambda i:x[i], out)
        yout = map(lambda i:y[i], out)

        scoreThreshold = self.models[rule].getState()['threshold']

        pl.hlines( 0, 0, len(y), linestyles = "solid", color='k' )

        pl.scatter( x, y, color='b' )
        pl.scatter( xout, yout, color='r' )
        for t, txt in enumerate(textout):
            pl.annotate(txt, (xout[t],yout[t]))

        pl.hlines( scoreThreshold, 0, len(y), label ="threshold", linestyles = "dotted", color='r' )

        pl.ylabel("Score")
        pl.xlabel("Product part")
        pl.title( "Rule '" +self.rules[rule]['name']+"': Outlier scores distribution" )

    def displayScore2( self, rule ):

        chiSquaredTest( self.scores[rule], len(self.rules[rule]['tests']), cut=0.01, a=1, b=2, c=1 )

    ## Display the data distribution graphically, using some options.
    #
    #  For details, see the display function of the Dataset class in the
    #  Input Manipulation Module.
    def display( self, inputset, rules = None, separate = False, flatpairs = False, nolabels = False, projection = False, maxTests = 5 ):

        config = inputset.getDefaultDisplayConfig()
        config['separate']  = separate
        config['flatpairs'] = flatpairs
        config['nolabels']  = nolabels
        config['projection']  = projection

        if rules == None:
            rules = range(self.nbRules())
            
        for rule in rules:
            try:
                if self.getModel(rule) == None:
                    #print( "Rule '"+self.rules[rule]['name']+"': no results to show" )
                    continue
            except IndexError:
                raise moaerror.MoaError("Wrong group index in result")
            f = pl.figure()
            self.displayScore( rule, inputset.getParts() )
            f = pl.figure()
            self.displayScore2( rule )
            if config['projection']:
                inputset.printPCA( self.getRule(rule), self.getModel(rule).getState() ) 
            if len(self.rules[rule]['tests']) <= maxTests:            
                f = pl.figure()
                inputset.display( self.getRule(rule), self.getOutliers(rule), self.getModel(rule).getState(), config, self.bads)
            else:
                print( "Rule '"+self.rules[rule]['name']+"': " +str(len(self.rules[rule]['tests']))+" tests, data not graphed (more than " + str(maxTests) + " tests)" )
            pl.show(block=False)
        raw_input("Press Enter to close figures...")

    ## Save the scores to a human readable CSV file.     
    def saveScoresToCSV( self, filename, infos = [], logistics = [], delimiter = ',' ):
        
        with open(filename+'.csv', 'wb') as csvfile:

            writer = csv.writer(csvfile, delimiter=delimiter)

            row = ['GROUP','','','']
            for info in infos:
                row.append('')
            for i in range(len(self.scores)):
                row.append(str(map(lambda x:x['name'],self.rules[i]['tests'])))
            writer.writerow( row )
            row = ['DISTANCE','','','']
            for info in infos:
                row.append('')
            for i in range(len(self.scores)):
                row.append(str(self.rules[i]['distance']))
            writer.writerow( row )

            writer.writerow( [] )

            row = ['RULE','MVBIN', 'SBIN', 'HBIN']
            for info in infos:
                row.append(str(info))
            for i in range(len(self.scores)):
                row.append(str(i+1)+':'+self.rules[i]['name'])
            for i in range(len(self.scores)):
                row.append(str(i+1)+':'+self.rules[i]['name']+'_zscore')
            writer.writerow( row )

            for j,part in enumerate(self.parts):
                row = [str(part),str(self.mvbin[j]),str(self.sbin[j]),str(self.hbin[j])]
                for i in range(len(infos)):
                    row.append(logistics[part][i])
                for i in range(len(self.scores)):
                    row.append(str(self.scores[i][j]))
                for i in range(len(self.scores)):
                    row.append(str(self.zscores[i][j]))
                writer.writerow( row )            

    ## Save the results to a machine readable Binary file.     
    def pickle( self, resultName ):

        output = open(resultName+'.result', 'wb')

        pickle.dump(self.version,  output)
        pickle.dump(self.rules,    output)
        pickle.dump(self.mvbin,    output)
        pickle.dump(self.sbin,     output)
        pickle.dump(self.hbin,     output)
        pickle.dump(self.scores,   output)
        pickle.dump(self.scoreContribs,   output)
        pickle.dump(self.outliers, output)
        pickle.dump(self.models,   output)
        pickle.dump(self.parts,    output)
        pickle.dump(self.tests,    output)
        pickle.dump(self.goods,    output)
        pickle.dump(self.bads,     output)

        output.close()

    ## Read the results from a Binary file.     
    def unpickle( self, resultName ):
        
        pkl_file = open(resultName+'.result', 'rb')

        self.version  = pickle.load(pkl_file)
        
        if (self.version == '1.0.0'):
            self.rules    = pickle.load(pkl_file)
            self.mvbin    = pickle.load(pkl_file)
            self.sbin     = pickle.load(pkl_file)
            self.hbin     = pickle.load(pkl_file)
            self.scores   = pickle.load(pkl_file)
            self.scoreContribs   = pickle.load(pkl_file)
            self.outliers = pickle.load(pkl_file)
            self.models   = pickle.load(pkl_file)
            self.parts    = pickle.load(pkl_file)
            self.tests    = pickle.load(pkl_file)
            self.goods    = pickle.load(pkl_file)
            self.bads     = pickle.load(pkl_file)

        elif (self.version == '0.1.3'):
            print "Warning! Expecting version number 1.0.0 in result file!"
            self.rules    = pickle.load(pkl_file)
            self.mvbin    = pickle.load(pkl_file)
            self.sbin     = pickle.load(pkl_file)
            self.hbin     = pickle.load(pkl_file)
            self.scores   = pickle.load(pkl_file)
            self.scoreContribs   = pickle.load(pkl_file)
            self.outliers = pickle.load(pkl_file)
            self.models   = pickle.load(pkl_file)
            self.parts    = pickle.load(pkl_file)
            self.tests    = pickle.load(pkl_file)
            self.goods    = pickle.load(pkl_file)
            self.bads     = pickle.load(pkl_file)

        else:
            raise MoaError( "Wrong version in result file (expecting 0.1.3)" )
        
        pkl_file.close()
