#-------------------------------------------------------------------------------
# File name: mvcompute.py
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

import mvlib.moaerror as moaerror

usage = "usage: %prog [options] INPUTNAME"
version = "1.0.0"
parser = OptionParser(usage=usage, version=version)
parser.add_option("-r", "--recipe", dest="recipeName",
                    default=None, help="Use recipe file RECIPENAME.recipe. By default, RECIPENAME is the same as INPUTNAME.",
                    metavar="RECIPENAME")
parser.add_option("-o", "--output", dest="outputName",
                    default=None, help="Write results to file OUTPUTNAME.result. By default, OUTPUTNAME is the same as INPUTNAME.",
                    metavar="OUTPUTNAME")

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

# 4. COMPUTE AND SAVE OUTLIERS

if options.outputName == None:
    options.outputName = inputName

import mvlib.result as result

result.computeOutliers( recipe, inputset, options.outputName )
