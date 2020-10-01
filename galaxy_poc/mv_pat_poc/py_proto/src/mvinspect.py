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
version = "0.1.0"
parser = OptionParser(usage=usage, version=version)

parser.add_option("-r", "--recipe", dest="recipeName",
                  default=None, help="Display the recipe named RECIPENAME",
                  metavar="RECIPENAME")

parser.add_option("-g", "--group", dest="ruleIndex",
                  default="0", help="Display only group number GROUPINDEX",
                  metavar="GROUPINDEX")

parser.add_option("-s", "--separate",action='store_true',default=False,
                  help="Display all test pairs in separate figures")

parser.add_option("-n", "--nolabels",action='store_true',default=False,
                  help="Do not display text labels in the figures")

parser.add_option("-m", "--maxtests",dest="maxTests",
                  default="5", help="Do not display data for groups with more than MAXTESTS tests.",
                  metavar="MAXTESTS")

parser.add_option("-x", "--xdie", dest="xDie",
                  default="1", help="DIE_X is the x coordinate of the product of interest",
                  metavar="DIE_X")

parser.add_option("-y", "--ydie", dest="yDie",
                  default="1", help="DIE_Y is the y coordinate of the product of interest",
                  metavar="DIE_Y")


(options, args) = parser.parse_args()

if len(args) < 1 or not isinstance(args[0],str):
    print "No input file was specified!"
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

# 4. DISPLAY DATA

import pylab as pl

def displayRule( rule, maxTests, bads, separate, nolabels ):
    tests = rule.getContents()['tests']
    if len(tests) <= maxTests:
        f = pl.figure()
        inputset.display2( tests, bads, separate=options.separate, nolabels=options.nolabels )
    else:
        print( "Rule '"+rule.getContents()['name']+"': data not graphed (more than " + str(maxTests) + " tests)" )
    

i = int( options.ruleIndex )
maxTests = int( options.maxTests )

bads = []
pid = inputset.findXY( int(options.xDie), int(options.yDie))
bads.extend( pid )
bads = pid
if i > 0:
    displayRule(recipe.getRules()[i-1], maxTests, bads, separate=options.separate, nolabels=options.nolabels)
else:
    for rule in recipe.getRules():
        displayRule(rule, maxTests, bads, separate=options.separate, nolabels=options.nolabels)

pl.show(block=False)
raw_input("Press Enter to close figures...")

    
