# 1. PARSE COMMAND LINE OPTIONS

from optparse import OptionParser

usage = "usage: %prog [options] INPUTNAME"
version = "0.1.1"
parser = OptionParser(usage=usage, version=version)

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

# 3. COMPUTE GROUPS

if options.testName == None:
    testlist = inputset.getTests()
    transfolist = ['normal']*len(testlist)
else:
    testlist, transfolist = dataset.loadCSVTests( options.testName+'.csv', delimiter=',' )

groups = []
g = []
for n,tf in enumerate(zip(testlist,transfolist)):
    g.append(tf)
    if (n+1)%5 == 0:
        groups.append( g )
        g = []

#4. GENERATE NEW RECIPE
        
import mvlib.recipe as recipe

recipe = recipe.Recipe()
for n,g in enumerate(groups):
    recipe.addRule( {"name": 'rule'+str(n+1),
                     "tests": map(lambda x:{"name":x[0],"shape":x[1]},g),
                     "distance": "near",
                     "type": "multivariate",
                     "components": 1000,
                     "method": "emp",
                     "enabled": True} )
if options.recipeName == None:
    options.recipeName = inputName
recipe.saveToJSON(options.recipeName)
