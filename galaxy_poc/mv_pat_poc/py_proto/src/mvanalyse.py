#-------------------------------------------------------------------------------
# File name: mvanalyse.py
# Created:   2013-06-19
# Author:    Jerome Kodjabachian
#
# This code is part of the Multivariate Outlier Analysis (MOA)
# for Part Average Testing (PAT) prototype. 
#
# (C) 2013 by Galaxy Semiconductor Inc. All rights reserved.
#-------------------------------------------------------------------------------

from optparse import OptionParser

usage = "usage: %prog [options] INPUTNAME"
version = "0.1.0"
parser = OptionParser(usage=usage, version=version)
parser.add_option("-o", "--output", dest="outputName",
                  default=None, help="Read results from file OUTPUTNAME.result. By default, OUTPUTNAME is the same as INPUTNAME.",
                  metavar="OUTPUTNAME")
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

# 2. LOAD INPUTS

import mvlib.dataset as dataset

inputset = dataset.Dataset(inputName)

# 3. LOAD RESULTS

import mvlib.result as result

results = result.Result()

if options.outputName == None:
    options.outputName = inputName
results.unpickle( options.outputName ) 

# 4. SHOW PID DATA

# $$$$ a simpler implementation would extract a line from the CSV result file
# but this implementation make it possible to make computation and graphs in
# relation to the product at dieX, dieY

pid = inputset.findXY( int(options.xDie), int(options.yDie))
print 'At: x=' + options.xDie + ', =' + options.yDie + ': ['
for p in pid:
    results.printPidData( p )
print ']'
