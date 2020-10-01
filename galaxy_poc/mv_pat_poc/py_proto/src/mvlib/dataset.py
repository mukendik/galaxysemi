#-------------------------------------------------------------------------------
# File name: dataset.py
# Created:   2013-06-04
# Author:    Jerome Kodjabachian
#
# This code is part of the Multivariate Outlier Analysis (MOA)
# for Part Average Testing (PAT) prototype. 
#
# (C) 2013 by Galaxy Semiconductor Inc. All rights reserved.
#-------------------------------------------------------------------------------

## @defgroup dataset Input Manipulation Module

import csv, json, math

import numpy as np
import pylab as pl
from sklearn.decomposition import PCA

import moaerror

def printPcDefinitions( pcaComponents, varNames ):
    for c in range(len(varNames)):
        s = 'PC'+str(c+1)+':\n'
        # Sorted
        for i,nc in sorted(enumerate(zip(varNames,pcaComponents[c])),key=lambda x:abs(x[1][1]), reverse=True):
            s = s + '\t'+ nc[0] + ' =\t'+"%0.6f" % nc[1]+'\n'
#        # Unsorted
#        for i,name in enumerate(varNames):
#            s = s + '\t'+ name + ' =\t'+"%0.6f" % pcaComponents[c][i]+'\n'
        print s

def stringToNumber(x):
    if x == '' or x == ' ':
        x = 'nan'
    try:
        return float(x)
    except ValueError:
        print( "Warning: could not convert to float: '" + x + "'")
        return float('nan')

## A function to load a dataset from an input file with options.
## @ingroup dataset
#
# The loading options are the number of rows to
# ignore before (ignoredRows1) and after (ignoredRows2)
# the test matrix header row, the number of left columns
# that contain logistic information (ignoredCols) and the
# character used as field delimiter (delimiter).
#
# The logistic information and the test results are
# extracted and returned in separated tables. The labels
# of the rows (parts) and of the columns of the returned
# table columns (tests and infos) are also returned.
def loadCsvData( filename, ignoredRows1=0, ignoredRows2=0, ignoredCols=7, delimiter=';' ):
    with open(filename, 'r') as csvfile:
        for i in range(ignoredRows1):
            next(csvfile) # ignore header
        reader = csv.reader(csvfile, delimiter=delimiter)
        row = next(csvfile)
        a = row.strip().split(delimiter)
        row = next(csvfile)
        b = row.strip().split(delimiter)
        infos = np.array(a[:ignoredCols]) # $$$$ strip also infos (see tests)
        tests = np.array(map(lambda x:b[x].strip()+':'+a[x].strip(), range(ignoredCols,len(a))))            
        
        for i in range(ignoredRows2-1):
            next(csvfile) # ignore test info
        reader = csv.reader(csvfile, delimiter=delimiter)
        a = [ row.strip().split(delimiter) for row in csvfile]
        table_infos = dict((data[0],data[:ignoredCols]) for data in a)
        table_tests = dict((data[0],np.array(map(stringToNumber,data[ignoredCols:]))) for data in a)

    parts = sorted([key for key in table_tests.keys()])

    return parts, table_tests, tests, table_infos, infos

## A function to load a list of tests.
## @ingroup dataset
#
#  Load a list of tests from  a CSV file with three rows: test names,
#  test indices, test transformations ('normal', 'log', etc.).
def loadCSVTests( filename, delimiter=';' ):
    with open(filename, 'r') as csvfile:
        reader = csv.reader(csvfile, delimiter=delimiter)
        row = next(csvfile)
        a = row.strip().split(delimiter)
        row = next(csvfile)
        b = row.strip().split(delimiter)
        tests = np.array(map(lambda x:b[x].strip()+':'+a[x].strip(), range(len(a))))            
        row = next(csvfile)
        transfos = row.strip().split(delimiter)
        
    return tests, transfos

def removeParts( measures, parts, excludedIndices ):
    keptMeasures = np.delete(measures, excludedIndices, 0)
    keptParts = [ part for p,part in enumerate(parts) if not p in excludedIndices ]
    return keptMeasures, keptParts

## A class to store information about an input file.
## @ingroup dataset
#
# A descriptor contains the path to the input file and a set of loading options
# used to extract the information in the file, like number of header rows to
# ignore and number of columns containing logistic information, the field
# delimiter, etc. The input file is in CSV format and may have been obtained
# by translating a STDF file using Galaxy software.
class Descriptor:

    ## Create a new descriptor (Constructor).
    #
    #  If a contents dictionary is provided it is used,
    #  otherwise a descriptor with empty contents is created.    
    def __init__( self, contents=None ):
        if contents:
            self.contents = contents
        else:
            self.contents = {}

    ## Return the contents of this descriptor.
    def getContents( self ):
        return self.contents
        
    ## Load the contents of this descriptor from a descriptor file.
    #
    # The file is in JSON format. This method first checks the
    # descriptor version indicated in the file and then loads
    # the contents.
    def loadFromJSON(self, filename):
        with open(filename) as f:
            d = json.load(f)

        if d['version'] == '1.0.0':
            self.contents = d['descriptor']
            return
        
        if d['version'] == '0.1.0':
            print "Warning! Expecting descriptor version 1.0.0"
            self.contents = d['descriptor']
            return

        raise moaerror.MoaError( 'Wrong descriptor version (expecting 1.0.0).' )
        return #not reached

    ## Save the contents of this descriptor to a file.
    #
    # The file format is JSON.
    def saveToJSON(self, outputname):
        try:

            with open(outputname, 'w') as f:
                d = {"version": "1.0.0",
                     "descriptor": self.contents}        
                json.dump(d, f, indent=4)

        except IOError:
            raise moaerror.MoaError( 'Could not save descriptor to file ' + filename )
    
## A class to store a list of input file descriptor names.
## @ingroup dataset
#
# This class makes it easier to process groups of input dataset.
class InputList:

    ## Create a new input path list (Constructor).
    #
    #  If an input path list is provided it is used to initialize
    # this object, otherwise an empty input path list is created.
    # The user may specify a root directory that represent a relative
    # or absolute path to add before each input path in the list.
    def __init__( self, _rootDir='.', _list=None ):

        if _list:
            self.list    = _list
        else:
            self.list = []

        self.rootDir = _rootDir
    
    ## Return the list of inputs.
    def getList( self ):
        return map(lambda x:self.rootDir+'/'+x,self.list)


    ## Load the contents of this list from an input path list file.
    #
    # The file is in JSON format. This method first checks the
    # input path list version indicated in the file and then loads
    # the contents.
    def loadFromJSON(self, filename):
        with open(filename) as f:
            l = json.load(f)

        if l['version'] == '1.0.0':
            self.list = l['list']
            return
        
        raise moaerror.MoaError( 'Wrong file list version (expecting 1.0.0).' )
        return #not reached

    ## Save the contents of this list to a file.
    #
    # The file format is JSON.
    def saveToJSON(self, outputname):
        try:

            with open(outputname, 'w') as f:
                d = {"version": "1.0.0",
                     "list": self.list}        
                json.dump(d, f, indent=4)

        except IOError:
            raise moaerror.MoaError( 'Could not save path list to file ' + filename )

## A class to store the information contained in an input file.
## @ingroup dataset
#
# The data itself is separated into (i) logistics information
# and (ii) test results.
class Dataset:

    ## Create and load a new dataset (Constructor).
    #
    #  A descriptor must be provided to set the loading options.    
    def __init__( self, inputName ):
        desc = Descriptor()    
        desc.loadFromJSON( inputName + '.input' )
        self.parts, self.measures, self.tests, self.logistics, self.infos = \
                    loadCsvData( desc.getContents()['name'],
                                 ignoredRows1=desc.getContents()['ignoredRows1'],
                                 ignoredRows2=desc.getContents()['ignoredRows2'],
                                 ignoredCols=desc.getContents()['ignoredCols'],
                                 delimiter=str(desc.getContents()['delimiter']) )

    ## Extends a dataset with the content of another dataset.
    #
    # Any test that does not exist in the second dataset will
    # be filled with nan (Not-A-Number) values. Any test that
    # does not exist in the first dataset will be ignored. 
    def extend( self, otherSet, prefix='other_' ):
        # This works, but simpler to use Gex for large merges
        otherTests = otherSet.getTests()
        otherMeasures = otherSet.getMeasures()
        otherInfos = otherSet.getInfos()
        otherLogistics = otherSet.getLogistics()
        for p in otherSet.getParts():
            key = prefix+p
            value = []
            for t in self.tests:
                indices = [i for i,test in enumerate( otherTests ) if test == t ]
                assert( len(indices) < 2 )
                if len(indices) == 0:
                    val = float('nan')
                else:
                    val = otherMeasures[p][indices[0]]
                value.append(val)
            self.measures[key] = np.array(value) 
            key = prefix+p
            value = []
            for f in self.infos:
                indices = [i for i,info in enumerate( otherInfos ) if info == f ]
                assert( len(indices) < 2 )
                if len(indices) == 0:
                    val = ''
                else:
                    val = otherLogistics[p][indices[0]]
                value.append(val)
            self.logistics[key] = value 
        self.parts = sorted([key for key in self.measures.keys()])

    ## Extract a measurement sub-matrix from this dataset.
    #
    # This will extract a matrix of measures for the tests listed in
    # testList. The raw values read from the input file can be transformed
    # according to contents of transfoList ('normal', i.e. no transformation,
    # 'log', etc.). If the filterNans option is set to True, any row of the
    # matrix to-be-returned that contains nan (Not-A-Number) values is deleted
    # and the resulting list of kept parts is returned.
    def extract( self, testList, transfoList, filterNans=True ):

        if len(testList)>0:
            paramList = [t for t,test in enumerate(self.tests) if test in testList]
            if len(paramList) < len(testList):
                print "Warning! The following tests could not be found in the input file:"
                print [test for test in testList if test not in self.tests]
                #raise moaerror.MoaError("Some extraction tests could not be found")

        else:
            paramList = range(200) #[t for t,test in enumerate(self.tests)]
            print "Warning! Taking 200 parameters only."
            # $$$ May want to remove parameters with more than x% missing data

        keptMeasures, keptParts = self.extractMatrix( paramList, transfoList, filterNans=filterNans )
        keptTests = self.tests[paramList]

        return keptMeasures, keptParts, keptTests

    def extractMatrix( self, params, transfos, filterNans = False ):
        x = []
        keptParts = []
        for [i,part] in enumerate(self.parts):
            row = []
            discardPart = False;
            for p,param in enumerate(params):
                value = self.measures[part][param]
                if filterNans and math.isnan(value):
                    discardPart = True
                    break
                else:
                    if transfos[p] == 'normal': # no transformation
                        row.append(value)
                    elif transfos[p] == 'log': # log transformation
                        row.append(math.log(value))
                    elif transfos[p] == 'logabs': # log abs transformation
                        row.append(math.log(abs(value)))
                    elif transfos[p] == 'sqrt': # sqrt transformation
                        row.append(math.sqrt(value))
                    elif transfos[p] == 'sqrtabs': # sqrt abs transformation
                        row.append(math.sqrt(abs(value)))
                    else:
                        raise moaerror.MoaError( "Unknown shape specified" ) 
            if not discardPart:
                x.append(row)
                keptParts.append(i)
        return np.array(x), keptParts

    ## Return any part found to have given x-y coordinates.
    #
    #  This method checks the values of the 'DIE_X' and 'DIE_Y' logistic
    #  properties.
    def findXY( self, x, y ):
        xinfo = [i for i,inf in enumerate(self.infos) if inf == 'DIE_X']
        yinfo = [i for i,inf in enumerate(self.infos) if inf == 'DIE_Y']
        results = [ part for part in self.parts if (x in map(lambda a:int(self.logistics[part][a]),xinfo)) and (y in map(lambda a:int(self.logistics[part][a]),yinfo))]
        return results

    ## Return the indices of parts for which a given logistic property (key)
    #  has a given integer value (value).
    def findIntegerInfo( self, key, value ):
        info = [i for i,inf in enumerate(self.infos) if inf == key]
        results = [ p for p,part in enumerate(self.parts) \
                    if (value in map(lambda a:int(self.logistics[part][a]),info))]
        return results

    ## Return the list of values found for the a given logistic property (key).
    def findInfos( self, key ):
        info = [i for i,inf in enumerate(self.infos) if inf.strip() == key.strip()]
        if len(info) == 0:
            results = [ float( 'nan' ) ] * len(self.parts)
        else:
            assert( len(info) == 1 )
            results = [self.logistics[part][info[0]] for part in self.parts ]

        return results

    ## Return a default display option configuration.
    def getDefaultDisplayConfig( self ):
        return {'separate':     False,
                'flatpairs':    False,
                'nolabels':     False,
                'projection':   False}

    def printPCA( self, rule, model=None ):

        # expects model version 0.1.1
        testList = map(lambda x:x['name'], rule['tests'])
        transfoList = map(lambda x:x['shape'], rule['tests'])
        measures, parts, tests = self.extract( testList, transfoList )

        if model['pca'] == None:
            pca = PCA(n_components=measures.shape[1])
            pca.fit(measures)
            # $$$ should remove bad parts and outliers
            print "Warning: Quick PCA analysis done for display only (no data cleaning). You may want to force PCA preprocessing by setting 'components' to 1000 in your recipe and recomputing results."
        else:
            pca = model['pca']

        X = pca.transform( measures ).transpose()
        printPcDefinitions( pca.components_, tests )
        
    ## Display a rule's results, using a given display configuration.
    #
    #  If the rule has just one test, a simple plot is drawn and
    #  the lower and higher limits are shown. If the rule has more
    #  than one test, tests are displayed by pairs. For each pair, a
    #  bivariate plot is drawn and, if the rule uses a covariance
    #  model, the projection of an ellipsoid is shown.
    #
    #  The user can specify various options: 'separate': plot each
    #  pair of variables in a separate figure; 'flatpairs': plot only
    #  pairs of successive parameters (in contrast to all possible
    #  pairs); 'nolabels': do not display text labels on the plots;
    #  'projection': plot the Principal Component variables instead
    #  of the original parameters.
    def display( self,
                 rule,
                 outliers=[],
                 model=None,
                 config=None,
                 bada=[] ):

        if config == None:
            config = self.getDefaultDisplayConfig()

        # expects model version 0.1.1
        first = True
        testList = map(lambda x:x['name'], rule['tests'])
        transfoList = map(lambda x:x['shape'], rule['tests'])
        measures, parts, tests = self.extract( testList, transfoList )

        if config['projection'] == True:
            if model['pca'] == None:
                pca = PCA(n_components=measures.shape[1])
                pca.fit(measures)
                # $$$ should remove bad parts and outliers
                print "Warning: Quick PCA analysis done for display only (no data cleaning). You may want to force PCA preprocessing by setting 'components' to 1000 in your recipe and recomputing results."
            else:
                pca = model['pca']

            X = pca.transform( measures ).transpose()

            #printPcDefinitions( pca.components_, tests )
            tests = map(lambda x:'PC'+str(x+1), range(X.shape[0]))

        else:
            X = measures.transpose()
        outa = outliers
        outb = [(o,self.parts[p]) for o,p in enumerate(parts) if self.parts[p] in outa]
        out = [o for (o,p) in outb]
        outs = [p for (o,p) in outb]
        badb = [(o,self.parts[p]) for o,p in enumerate(parts) if self.parts[p] in bada]
        bad = [o for o,p in badb]
        bads = [p for o,p in badb]
        dim = X.shape[0]
        is_univariate = rule['type'] == 'univariate'
        nbOfSigmas = model['threshold']
        
        if dim == 1:
            pl.scatter(range(len(X[0])),X[0],marker='.', c='b')
            pl.scatter(out,X[0][out],marker='o', c='r')
            pl.scatter(bad,X[0][bad],marker='o', c='g')
            yL = [model['mean'][0]-nbOfSigmas*model['std'][0],
                  model['mean'][0]-nbOfSigmas*model['std'][0]]
            yU = [model['mean'][0]+nbOfSigmas*model['std'][0],
                  model['mean'][0]+nbOfSigmas*model['std'][0]]
            x = [pl.xlim()[0],pl.xlim()[1]]
            pl.plot( x, yL, c='r' )
            pl.plot( x, yU, c='r' )
            #pl.ylabel(tests[0])
            pl.title(tests[0])
        elif not config['flatpairs']:
            for i in range(0,dim):
                for j in range(i+1,dim):
                    if config['separate']:
                        if first:
                            first = False
                        else:
                            if dim > 11:
                                pl.show(block=False)
                                raw_input("Press Enter to close figures...")
                                pl.close()
                            f = pl.figure()
                    else:
                        splot = pl.subplot(dim-1, dim-1, i*(dim-1)+j)
                    pl.scatter(X[j],X[i],marker='.', c='b')
                    pl.scatter(X[j][out],X[i][out],marker='o', c='r')
                    if not config['nolabels']:
                        for t, txt in enumerate(outs):
                            pl.annotate(txt, (X[j][out[t]],X[i][out[t]]))
                    pl.scatter(X[j][bad],X[i][bad],marker='o', c='g')
                    if not config['nolabels']:
                        for t, txt in enumerate(bads):
                            pl.annotate(txt, (X[j][bad[t]],X[i][bad[t]]))
                    if is_univariate and model:
                        # Draw a rectangle to show the uv limits
                        x = [model['mean'][j]-nbOfSigmas*model['std'][j],
                             model['mean'][j]+nbOfSigmas*model['std'][j],
                             model['mean'][j]+nbOfSigmas*model['std'][j],
                             model['mean'][j]-nbOfSigmas*model['std'][j],
                             model['mean'][j]-nbOfSigmas*model['std'][j]]
                        y = [model['mean'][i]-nbOfSigmas*model['std'][i],
                             model['mean'][i]-nbOfSigmas*model['std'][i],
                             model['mean'][i]+nbOfSigmas*model['std'][i],
                             model['mean'][i]+nbOfSigmas*model['std'][i],
                             model['mean'][i]-nbOfSigmas*model['std'][i]]
                        pl.plot( x, y )
                    elif rule['method'] <> 'svm':
                        # Draw an ellipse to show the mv limits
                        xlim = pl.xlim()
                        ylim = pl.ylim()
                        xx, yy = np.meshgrid(np.linspace(xlim[0], xlim[1], 100),
                                                     np.linspace(ylim[0], ylim[1], 100))
                        zz = np.c_[xx.ravel(),yy.ravel()]
                        list1 = range(dim)
                        list2 = [j,i]
                        for d in range(dim):
                            if d <> i and d <> j:
                                list2.append(d)
                        for d in range(dim):
                            if d <> i and d <> j:
                                zz = np.append(zz, np.array([(model['loc'][d]*np.ones(100*100))]).T, 1)
                        zz[:,list2] = zz[:,list1]
                        cov = model['cov'].mahalanobis(zz)
                        cov = cov.reshape(xx.shape)
                        try:
                            contour = pl.contour(xx, yy, np.sqrt(cov), [0,model['threshold']], cmap=pl.cm.hot_r,linestyles='dashed')
                            pl.clabel(contour, fontsize=10, inline=1)
                        except ValueError:
                            print 'Warning: Could not draw ellipse (value error)!'
                        pl.scatter(X[j],X[i],marker='.', c='b')
                        pl.scatter(X[j][out],X[i][out],marker='o', c='r')
                        if not config['nolabels']:
                            for t, txt in enumerate(outs):
                                pl.annotate(txt, (X[j][out[t]],X[i][out[t]]))
                        pl.scatter(X[j][bad],X[i][bad],marker='o', c='g')
                        if not config['nolabels']:
                            for t, txt in enumerate(bads):
                                pl.annotate(txt, (X[j][bad[t]],X[i][bad[t]]))
                        pl.xlim(xlim)
                        pl.ylim(ylim)
                    pl.xlabel(tests[j])
                    pl.ylabel(tests[i])
            #pl.title( map(lambda x:tests[x],range(len(tests))) )
        else:
            size = math.ceil(math.sqrt(dim))
            for i in range(0,dim-1):
                j = i+1
                if config['separate']:
                    if first:
                        first = False
                    else:
                        if dim > 60:
                            pl.show(block=False)
                            raw_input("Press Enter to close figures...")
                            pl.close()

                        f = pl.figure()
                else:
                    splot = pl.subplot(size, size, i)
                pl.scatter(X[j],X[i],marker='.', c='b')
                pl.scatter(X[j][out],X[i][out],marker='o', c='r')
                if not config['nolabels']:
                    for t, txt in enumerate(outs):
                        pl.annotate(txt, (X[j][out[t]],X[i][out[t]]))
                pl.scatter(X[j][bad],X[i][bad],marker='o', c='g')
                if not config['nolabels']:
                    for t, txt in enumerate(bads):
                        pl.annotate(txt, (X[j][bad[t]],X[i][bad[t]]))
                if is_univariate and model:
                    # Draw a rectangle to show the uv limits
                    x = [model['mean'][j]-nbOfSigmas*model['std'][j],
                        model['mean'][j]+nbOfSigmas*model['std'][j],
                        model['mean'][j]+nbOfSigmas*model['std'][j],
                        model['mean'][j]-nbOfSigmas*model['std'][j],
                        model['mean'][j]-nbOfSigmas*model['std'][j]]
                    y = [model['mean'][i]-nbOfSigmas*model['std'][i],
                        model['mean'][i]-nbOfSigmas*model['std'][i],
                        model['mean'][i]+nbOfSigmas*model['std'][i],
                        model['mean'][i]+nbOfSigmas*model['std'][i],
                        model['mean'][i]-nbOfSigmas*model['std'][i]]
                    pl.plot( x, y )
                elif rule['method'] <> 'svm' and rule['method'] <> 'gmm':
                    # Draw an ellipse to show the mv limits
                    xlim = pl.xlim()
                    ylim = pl.ylim()
                    xx, yy = np.meshgrid(np.linspace(xlim[0], xlim[1], 100),
                                                     np.linspace(ylim[0], ylim[1], 100))
                    zz = np.c_[xx.ravel(),yy.ravel()]
                    list1 = range(dim)
                    list2 = [j,i]
                    for d in range(dim):
                        if d <> i and d <> j:
                            list2.append(d)
                    for d in range(dim):
                        if d <> i and d <> j:
                            zz = np.append(zz, np.array([(model['loc'][d]*np.ones(100*100))]).T, 1)
                    zz[:,list2] = zz[:,list1]
                    cov = model['cov'].mahalanobis(zz)
                    cov = cov.reshape(xx.shape)
                    try:
                        contour = pl.contour(xx, yy, np.sqrt(cov), [0,model['threshold']], cmap=pl.cm.hot_r,linestyles='dashed')
                        pl.clabel(contour, fontsize=10, inline=1)
                    except ValueError:
                        print 'Warning: Could not draw ellipse (value error)!'
                    pl.scatter(X[j],X[i],marker='.', c='b')
                    pl.scatter(X[j][out],X[i][out],marker='o', c='r')
                    if not config['nolabels']:
                        for t, txt in enumerate(outs):
                            pl.annotate(txt, (X[j][out[t]],X[i][out[t]]))
                    pl.scatter(X[j][bad],X[i][bad],marker='o', c='g')
                    if not config['nolabels']:
                        for t, txt in enumerate(bads):
                            pl.annotate(txt, (X[j][bad[t]],X[i][bad[t]]))
                    pl.xlim(xlim)
                    pl.ylim(ylim)
                pl.xlabel(tests[j])
                pl.ylabel(tests[i])
        #pl.title( map(lambda x:tests[x],range(len(tests))) )
        
    def display2( self, testSelection, bads=[], separate=False, nolabels = False ):

        first = True
        measures, parts, tests = self.extract( testSelection )

        X = measures.transpose()
        bad = [o for o,p in enumerate(parts) if self.parts[p] in bads]
        dim = X.shape[0]
        if dim == 1:
            pl.plot(X[0],marker='.', c='b')
            pl.scatter(bad,X[0][bad],marker='o', c='r')
            x = [pl.xlim()[0],pl.xlim()[1]]
            pl.title(tests[0])
        else:
            for i in range(0,dim):
                for j in range(i+1,dim):
                    if separate:
                        if first:
                            first = False
                        else:
                            f = pl.figure()
                    else:
                        splot = pl.subplot(dim-1, dim-1, i*(dim-1)+j)
                    pl.scatter(X[j],X[i],marker='.', c='b')
                    pl.scatter(X[j][bad],X[i][bad],marker='o', c='r')
                    if not nolabels:
                        for t, txt in enumerate(bads):
                            pl.annotate(txt, (X[j][bad[t]],X[i][bad[t]]))
                    pl.xlabel(tests[j])
                    pl.ylabel(tests[i])

    ## Return the list of part labels of this dataset.
    def getParts( self ):
        return self.parts

    ## Return the table of measures of this dataset.
    def getMeasures( self ):
        return self.measures

    ## Return the list of test labels of this dataset.
    def getTests( self ):
        return self.tests
    
    ## Return the table of logistic information of this dataset.
    def getLogistics( self ):
        return self.logistics

    ## Return the list of logistic information labels of this dataset.
    def getInfos( self ):
        return self.infos
    
