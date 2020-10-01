#-------------------------------------------------------------------------------
# File name: mvgraph.py
# Created:   2013-06-04
# Author:    Jerome Kodjabachian
#
# This code is part of the Multivariate Outlier Analysis (MOA)
# for Part Average Testing (PAT) prototype. 
#
# (C) 2013 by Galaxy Semiconductor Inc. All rights reserved.
#-------------------------------------------------------------------------------

# 1. PARSE COMMAND LINE OPTIONS

from optparse import OptionParser

usage = "usage: %prog [options] INPUTNAME"
version = "1.0.0"
parser = OptionParser(usage=usage, version=version)
parser.add_option("-o", "--output", dest="outputName",
                  default=None, help="Read results from file OUTPUTNAME.result. By default, OUTPUTNAME is the same as INPUTNAME.",
                  metavar="OUTPUTNAME")

parser.add_option("-g", "--group", dest="ruleIndex",
                  default="0", help="Display only group number GROUPINDEX",
                  metavar="GROUPINDEX")

parser.add_option("-s", "--singlefigure",action='store_true',default=False,
                  help="Display all test pairs in a single figures")

parser.add_option("-a", "--allpairs",action='store_true',default=False,
                  help="Display consecutive test pairs (not all pairs)")

parser.add_option("-n", "--nolabels",action='store_true',default=False,
                  help="Do not display text labels in the figures")

parser.add_option("-p", "--projection",action='store_true',default=False,
                  help="Display pca components instead of tests")

parser.add_option("-m", "--maxtests",dest="maxTests",
                  default="20", help="Do not display data for groups with more than MAXTESTS tests. Default value is 20.",
                  metavar="MAXTESTS")

(options, args) = parser.parse_args()

if len(args) < 1 or not isinstance(args[0],str):
    print "No input file was specified!"
    exit()    

inputName = args[0]

# 2. LOAD INPUTS

import mvlib.dataset as dataset

inputset = dataset.Dataset(inputName)

#inputset2 = dataset.Dataset('sample2')
#inputset.extend( inputset2 )
#print( 'Input dataset has ' + str(len(inputset.getParts())) + ' parts and ' + str(len(inputset.getTests())) + ' tests.' )

# 3. LOAD RESULTS

import mvlib.result as result

results = result.Result()

if options.outputName == None:
    options.outputName = inputName
results.unpickle( options.outputName ) 

# 4. DISPLAY RESULTS

"""
for v in results.scoreContribs[4]:
    if v != None:
        s = ''
        for x in v:
            s = s + '%f, ' % x
        print s
exit()
"""

results.printSummary()

i = int( options.ruleIndex )
maxTests = int( options.maxTests )
if i > 0:
    results.display( inputset,  [i-1], separate=not options.singlefigure, flatpairs=not options.allpairs, nolabels=options.nolabels, projection = options.projection, maxTests = maxTests )
else:
    results.display( inputset, separate=not options.singlefigure, flatpairs=not options.allpairs, nolabels=options.nolabels, projection = options.projection, maxTests = maxTests  )

