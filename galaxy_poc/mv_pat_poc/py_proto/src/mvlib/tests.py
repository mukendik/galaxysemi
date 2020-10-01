#-------------------------------------------------------------------------------
# File name: recipe_tests.py
# Created:   2013-06-16
# Author:    Jerome Kodjabachian
#
# This code is part of the Multivariate Outlier Analysis (MOA)
# for Part Average Testing (PAT) prototype. 
#
# (C) 2013 by Galaxy Semiconductor Inc. All rights reserved.
#-------------------------------------------------------------------------------

## @defgroup tests Unit Test Module

import unittest
import recipe

## Tests for the Multivariate Recipe Module.
## @ingroup tests
class testPackageRecipe(unittest.TestCase):

    ## Tests for the Rule class.
    def testClassRule(self):
        rule1 = recipe.Rule()
        rule2 = recipe.Rule( {"name":        "No name",
                             "tests":       [],
                             "distance":    "near",
                             "type":        "multivariate",
                             "method":      "emp",
                             "enabled":     False } )
        self.assertEqual(rule1.getContents(), rule2.getContents())

    ## Tests for the Recipe class.
    def testClassRecipe(self):
        recipe1 = recipe.Recipe()
        recipe1.addRule( {"name":        "No name",
                                   "tests":      [],
                                   "distance":   "near",
                                   "type":        "multivariate",
                                   "method":      "emp",
                                   "enabled":     False } )
        recipe1.saveToJSON('_tmp')
        
        recipe2 = recipe.Recipe('_tmp')
        
        recipe3 = recipe.Recipe()
        recipe3.addRule( recipe.Rule() )

        self.assertEqual( len(recipe2.getRules()), len(recipe3.getRules()) )
        for rule in range(len(recipe2.getRules())):
            self.assertEqual( recipe2.getRules()[rule].getContents(), recipe3.getRules()[rule].getContents() )

        recipe4 = recipe.Recipe()
        self.assertRaises(recipe.RecipeError, recipe4.loadFromJSON, '_non_existing')

        # $$$$ add a test that saveToJSON return an exception when it cannot write (may use a readonly file)
        
import dataset
## Tests for the Input Manipulation Module.
## @ingroup tests
class testPackageDataset(unittest.TestCase):

    ## Tests for the Dataset class.
    def testClassDataset(self):
        inputset = dataset.Dataset( "../data/test/sample" )
        parts = inputset.findXY(18,25)
        self.assertEqual(1, len(parts))
        self.assertEqual('PID-575', parts[0])

        self.assertEqual(len(inputset.findIntegerInfo(' SBIN',1)), 2003)
        self.assertEqual(len(inputset.findIntegerInfo(' SBIN',2)), 18)
        self.assertEqual(len(inputset.findIntegerInfo(' SBIN',3)), 0)
        self.assertEqual(len(inputset.findIntegerInfo(' SBIN',4)), 1)
      
        otherInputset = dataset.Dataset( "../data/test/sample2" )
        parts = otherInputset.findXY(19,15)
        self.assertEqual(1, len(parts))
        self.assertEqual('PID-152', parts[0])

        inputset.extend( otherInputset )
        print inputset.getParts()


import grouper
import pprint

## Tests for the Group Formation Module.
## @ingroup tests
class testPackageGrouper(unittest.TestCase):

    ## Tests for the computeCorrMat function.
    def testFunctionPrintCorrMatrix(self):
        inputset = dataset.Dataset( "../data/test/sample" )
        r = recipe.Recipe("../data/test/sample")
        for rule in r.getRules():
            tests = rule.getContents()['tests']
            grouper.Grouper.printCorrMatrix( inputset, tests, minCorr=0.9 )
        #params = range(50,60)
        #tests = inputset.getTests()[params]

import table

## Tests for the Statistical Table Module.
## @ingroup tests
class testPackageTable(unittest.TestCase):

    ## Tests for the ChiToSigma class.
    def testClassChiToSigma(self):
        d = {}
        for dim in [1, 3, 50, 300]: 
            d[dim] = table.ChiToSigma( dim )
        for dim in [1, 3, 50, 300]: 
            self.assertAlmostEqual(table.chiToSigma( 0.0, dim )[0], d[dim].get( 0.0 ), places=2)
            self.assertAlmostEqual(table.chiToSigma( 3.0, dim )[0], d[dim].get( 3.0 ), places=2)
            self.assertAlmostEqual(table.chiToSigma( 7.0, dim )[0], d[dim].get( 7.0 ), places=2)
            self.assertAlmostEqual(table.chiToSigma( 15.0, dim )[0], d[dim].get( 15.0 ), places=2)
            self.assertAlmostEqual(table.chiToSigma( 100.0, dim )[0], d[dim].get( 100.0 ), places=2)

import result

## Tests for the Multivariate Result Module.
## @ingroup tests
class testPackageModel(unittest.TestCase):

    ## Tests for the Mahalanobis Model.
    def testMahalanobis(self):
        _dataset = dataset.Dataset( "../data/test/sample" )
        _recipe  = recipe.Recipe("../data/test/sample")
        _results = result.Result(_dataset.getParts(), _dataset.getTests() )
        for r in _recipe.getRules():
             i = _results.addResult( _dataset, r.getContents() )

        #_results.pickle( "../data/test/sample_expected" )
        _expectedResults = result.Result()
        _expectedResults.unpickle( "../data/test/sample_expected" )

        # compare results and expected results
        self.assertEqual( _results.nbRules(), _expectedResults.nbRules() )
        for i in range(_results.nbRules() ):
            self.assertEqual( len(_results.scores[i]), len(_expectedResults.scores[i]) )
            for j in range(len(_results.scores[i])):
                self.assertEqual( _results.scores[i][j],_expectedResults.scores[i][j])
        
        

if __name__ == "__main__":
    unittest.main()  
