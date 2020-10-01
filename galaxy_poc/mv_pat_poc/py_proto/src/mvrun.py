#-------------------------------------------------------------------------------
# File name: mvstats.py
# Created:   2013-07-06
# Author:    Jerome Kodjabachian
#
# This code is part of the Multivariate Outlier Analysis (MOA)
# for Part Average Testing (PAT) prototype. 
#
# (C) 2013 by Galaxy Semiconductor Inc. All rights reserved.
#-------------------------------------------------------------------------------

from optparse import OptionParser

import mvlib.moaerror as moaerror

usage = "usage: %prog [options] INPUTLIST"
version = "1.0.0"
parser = OptionParser(usage=usage, version=version)

parser.add_option("-r", "--recipe", dest="recipeName",
                  help="Use recipe file RECIPENAME.recipe.",
                  metavar="RECIPENAME")

parser.add_option("-o", "--output", dest="statName",
                  default=None, help="Write stats to file STATNAME.csv. By default, STATNAME is <INPUTLIST>_<RECIPENAME>.",
                  metavar="STATNAME")

parser.add_option("-d", "--directory", dest="rootDir",
                  default=".", help="Read inputs and write outputs to directory ROOTDIR. By default, ROOTDIR is the current directory.",
                  metavar="ROOTDIR")

parser.add_option("-c", "--chained", dest="chainedRecipe",
                  default=None, help="Discount outliers found by CHAINEDRECIPE from the stats. By default, there is no chained recipe.",
                  metavar="CHAINEDRECIPE")

(options, args) = parser.parse_args()

if len(args) < 1 or not isinstance(args[0],str):
    print "No input list was specified!"
    exit()

inputListName = args[0]

# 2. LOAD RECIPE

if options.recipeName == None:
    options.recipeName = inputListName
    
import mvlib.recipe as recipe

rec = recipe.Recipe()
rec.loadFromJSON(options.recipeName)

import csv, os
_, recipeShortName = os.path.split(options.recipeName)

# 3. LOAD INPUTS
    
import mvlib.dataset as dataset

inputList = dataset.InputList(options.rootDir)
inputList.loadFromJSON( inputListName + '.list' )

# 4. COMPUTE AND SAVE OUTLIERS
import mvlib.result as result

for inputName in inputList.getList():
    inputset = dataset.Dataset(inputName)
    print( 'Input dataset has ' + str(len(inputset.getParts())) + ' parts and ' + str(len(inputset.getTests())) + ' tests.' )

    result.computeOutliers( rec, inputset, inputName + '_' + recipeShortName )

if options.statName == None:
    options.statName = inputListName + '_' + recipeShortName

nbrules = 200 # Maximum number of rules in the stat report - $$$$ this could be improved

with open(options.statName+'.csv', 'wb') as csvfile:

    writer = csv.writer(csvfile, delimiter=',')

    row = ['FILE','NB_PROD','NB_OUT']
    for i in range(nbrules):
        row.append('Rule'+str(i+1))
    row.append('NB_PROD_BIN1')
    row.append('NB_OUT_BIN1')
    for i in range(nbrules):
        row.append('Rule'+str(i+1))
    writer.writerow( row )
        
    N_tot = 0
    n_tot = [0]*nbrules
    N1_tot = 0
    n1_tot = [0]*nbrules

    for inputName in inputList.getList():

        # 1. LOAD RESULTS

        results = result.Result()
        results.unpickle( inputName + '_' + recipeShortName ) 

        uvbins = None
        if options.chainedRecipe:
            _, shortName = os.path.split(options.chainedRecipe)
            
            results2 = result.Result()
            results2.unpickle( inputName + '_' + shortName )
            uvbins = results2.mvbin

        # 2. SAVE STATS
        N, n, N1, n1 = results.computeMvbinSummary( range(1,nbrules+1), uvbins )
        N_tot = N_tot+N
        N1_tot = N1_tot+N1
        for i in range(nbrules):
            n_tot[i] = n_tot[i] + n[i]
            n1_tot[i] = n1_tot[i] + n1[i]

        row = [inputName + '_' + recipeShortName,N,sum(n)]
        for i,s in enumerate(n):
            row.append(s)
        row.append(N1)
        row.append(sum(n1))
        for i,s in enumerate(n1):
            row.append(s)
        writer.writerow( row )

    row = []
    writer.writerow( row )

    row = ['TOTAL',N_tot,sum(n_tot)]
    for i,s in enumerate(n_tot):
        row.append(s)
    row.append(N1_tot)
    row.append(sum(n1_tot))
    for i,s in enumerate(n1_tot):
        row.append(s)
    writer.writerow( row )

    row = ['PERCENT',100.0,100.0*sum(n_tot)/N_tot]
    for i,s in enumerate(n_tot):
        row.append(100.0*s/N_tot)
    row.append(100.0)
    row.append(100.0*sum(n1_tot)/N1_tot)
    for i,s in enumerate(n1_tot):
        row.append(100.0*s/N1_tot)
    writer.writerow( row )
