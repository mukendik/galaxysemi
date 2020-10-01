# 1. PARSE COMMAND LINE OPTIONS

from optparse import OptionParser

usage = "usage: %prog [options] INPUTNAME"
version = "1.0.0"
parser = OptionParser(usage=usage, version=version)

parser.add_option("-c", "--corrmin", dest="minCorr",
                  default="0.8", help="set minimum correlation to CORRMIN",
                  metavar="CORRMIN")

parser.add_option("-r", "--recipe", dest="recipeName",
                  default=None, help="create new recipe with name RECIPENAME.recipe. By default, RECIPENAME is the same as INPUTNAME.",
                  metavar="RECIPENAME")

parser.add_option("-t", "--testlist", dest="testName",
                  default=None, help="specify the test selection TESTLIST to use for creating the new recipe. By default, all tests will be used.",
                  metavar="TESTLIST")

(options, args) = parser.parse_args()

if len(args) < 1 or not isinstance(args[0],str):
    print "No input file was specified!"
    exit()    

inputName = args[0]

# 2. LOAD INPUTS

import mvlib.dataset as dataset

inputset = dataset.Dataset(inputName)
print( 'Input dataset has ' + str(len(inputset.getParts())) + ' parts and ' + str(len(inputset.getTests())) + ' tests.' )

# 3. COMPUTE CORRS

import mvlib.grouper as grouper

if options.testName == None:
    grouper = grouper.Grouper(inputset, minCorr=float(options.minCorr))
else:
    testlist, transfolist = dataset.loadCSVTests( options.testName+'.csv', delimiter=',' )
    grouper = grouper.Grouper(inputset, tests=testlist, transfos=transfolist, minCorr=float(options.minCorr))
# 3. DISPLAY RESULTS

kept = []
for g, group in enumerate(grouper.getGroups()):
    if len(group)>1:
        kept.append(g)
        print( 'GROUP ' + str(len(kept))+': ' + str(list(group)) )

#4. GENERATE NEW RECIPE
        
import mvlib.recipe as recipe

if options.recipeName == None:
    options.recipeName = inputName

# recipe.createRecipeFromGroups( kept, options.recipeName )
# $$$$ transformations are not passed by the grouper
recipe = recipe.Recipe()
for n,g in enumerate(kept):
    recipe.addRule( {"name": 'rule'+str(n+1),
                     "tests": map(lambda x:{"name":x,"shape":"normal"},list(grouper.getGroups()[g])),
                     "distance": "near",
                     "type": "multivariate",
                     "components": 1000,
                     "method": "emp",
                     "enabled": True} )

recipe.saveToJSON(options.recipeName)
