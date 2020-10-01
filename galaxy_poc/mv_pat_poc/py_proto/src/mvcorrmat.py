#-------------------------------------------------------------------------------
# File name: mvcorrmat.py
# Created:   2013-06-21
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
version = "0.1.0"
parser = OptionParser(usage=usage, version=version)

parser.add_option("-r", "--recipe", dest="recipeName",
                  default=None, help="Use recipe file RECIPENAME.recipe. By default, RECIPENAME is the same as INPUTNAME.",
                  metavar="RECIPENAME")

parser.add_option("-c", "--corrmin", dest="minCorr",
                  default=None, help="set minimum displayed correlation to CORRMIN. By default all values are displayed.",
                  metavar="CORRMIN")

parser.add_option("-p", "--printprob",action='store_true',default=False,
                  help="Print also p-values, in addition to correlation coefficients.")

(options, args) = parser.parse_args()

if len(args) < 1 or not isinstance(args[0],str):
    print "No input name was specified!"
    exit()

inputName = args[0]

# 2. LOAD RECIPE

if options.recipeName == None:
    options.recipeName = inputName
    
import mvlib.recipe as recipe

recipe = recipe.Recipe()
recipe.loadFromJSON(options.recipeName)

# 3. LOAD INPUTS
    
import mvlib.dataset as dataset

inputset = dataset.Dataset(inputName)
print( 'Input dataset has ' + str(len(inputset.getParts())) + ' parts and ' + str(len(inputset.getTests())) + ' tests.' )

# 4. PRINT CORRELATION MATRICES

import mvlib.grouper as grouper

if options.minCorr == None:
    minCorr = None
else:
    minCorr = float(options.minCorr)

print
for rule in recipe.getRules():
    print "Group " + rule.getContents()['name'] + ":"
    print
    tests = rule.getContents()['tests']
    grouper.Grouper.printCorrMatrix( inputset, tests, minCorr=minCorr, showProba=options.printprob )
    print
