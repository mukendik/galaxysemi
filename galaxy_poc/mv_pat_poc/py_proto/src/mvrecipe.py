# 1. PARSE COMMAND LINE OPTIONS

from optparse import OptionParser

usage = "usage: %prog [options] INPUTLIST"
version = "1.0.0"
parser = OptionParser(usage=usage, version=version)

parser.add_option("-c", "--corrmin", dest="minCorr",
                  default="0.8", help="set minimum correlation to CORRMIN",
                  metavar="CORRMIN")

parser.add_option("-r", "--recipe", dest="recipeName",
                  default=None, help="create a new recipe named RECIPENAME.recipe. By default, RECIPENAME is the same as INPUTLIST.",
                  metavar="RECIPENAME")

parser.add_option("-d", "--directory", dest="rootDir",
                  default=".", help="Read inputs and write temporary recipes to directory ROOTDIR. By default, ROOTDIR is the current directory.",
                  metavar="ROOTDIR")

parser.add_option("-t", "--testlist", dest="testName",
                  default=None, help="specify the test selection TESTLIST to use for creating the new recipe. By default, all tests will be used.",
                  metavar="TESTLIST")

(options, args) = parser.parse_args()

if len(args) < 1 or not isinstance(args[0],str):
    print "No input file was specified!"
    exit()    

inputListName = args[0]

# 2. LOAD INPUTS AND CREATE GROUP-BASED RECIPE

import mvlib.dataset as dataset
import mvlib.grouper as grouper
import mvlib.recipe as recipe

inputList = dataset.InputList(options.rootDir)
inputList.loadFromJSON( inputListName + '.list' )

for inputName in inputList.getList():

    inputset = dataset.Dataset(inputName)
    print( 'Input dataset has ' + str(len(inputset.getParts())) + ' parts and ' + str(len(inputset.getTests())) + ' tests.' )

    print options.testName
    if options.testName == None:
        gper = grouper.Grouper(inputset, minCorr=float(options.minCorr))
    else:
        testlist, transfolist = dataset.loadCSVTests( options.testName+'.csv', delimiter=',' )
        gper = grouper.Grouper(inputset, tests=testlist, transfos=transfolist, minCorr=float(options.minCorr))

    kept = []
    for g, group in enumerate(gper.getGroups()):
        if len(group)>1:
            kept.append(g)
            print( 'GROUP ' + str(len(kept))+': ' + str(list(group)) )

    # recipe.createRecipeFromGroups( kept, inputName+'_tmp' )
    # $$$$ transformations are not passed by the grouper
    rec = recipe.Recipe()
    for n,g in enumerate(kept):
        rec.addRule( {"name": 'rule'+str(n+1),
                         "tests": map(lambda x:{"name":x,"shape":"normal"},list(gper.getGroups()[g])),
                         "distance": "near",
                         "type": "multivariate",
                         "components": 1000,
                         "method": "emp",
                         "enabled": True} )
    rec.saveToJSON(inputName+'_tmp')

if options.recipeName == None:
    options.recipeName = inputListName
    
groups = []
for inputName in inputList.getList():
    groups.extend( recipe.readRecipeGroups(inputName+'_tmp') )
groups = grouper.cleanGroupList( groups )
recipe.createRecipeFromGroups( groups, options.recipeName )
