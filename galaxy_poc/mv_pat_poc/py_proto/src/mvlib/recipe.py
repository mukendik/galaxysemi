#-------------------------------------------------------------------------------
# File name: recipe.py
# Created:   2013-06-04
# Author:    Jerome Kodjabachian
#
# This code is part of the Multivariate Outlier Analysis (MOA)
# for Part Average Testing (PAT) prototype. 
#
# (C) 2013 by Galaxy Semiconductor Inc. All rights reserved.
#-------------------------------------------------------------------------------

## @defgroup recipe Multivariate Recipe Module

import json

import moaerror

def readRecipeGroups( filename ):
    rec = Recipe( filename )
    groups = []
    for rule in rec.getRules():
        groups.append(rule.getContents()['tests'])
    return groups

def createRecipeFromGroups( groups, filename ):
    rec = Recipe()
    for n,g in enumerate(groups):
        rec.addRule( {"name": 'rule'+str(n+1),
                         "tests": map(lambda x:{"name":x['name'],"shape":x['shape']},g),
                         "distance": "near",
                         "type": "multivariate",
                         "components": 1000,                      
                         "method": "emp",
                         "enabled": True} )
    rec.saveToJSON(filename)

## A class to store rule descriptions.
## @ingroup recipe
#
# A rule instance merely contains a rule description and can be
# added to a recipe.
class Rule:
    
    ## Create a new rule (Constructor).
    #
    #  If a contents dictionary is provided it is used to set the rule,
    #  otherwise a rule with default contents is created.
    def __init__( self, contents = None ):
        if contents:
            self.contents = contents
        else:
            self.contents = {"name":        "No name",
                             "tests":       [],
                             "distance":    "near",
                             "type":        "multivariate",
                             "components":  None,
                             "method":      "emp",
                             "enabled":     False }

    ## Return the contents of this rule.
    def getContents( self ):
        return self.contents

    ## Set the contents of this rule.
    def setContents( self, contents ):
        self.contents = contents

## A class to store the description of a recipe.
## @ingroup recipe
#
#  A recipe is a collection of rule descriptions that can be
#  saved or loaded to/from a JSON file.
class Recipe:

    ## Create a new recipe (Constructor).
    #
    #  If a filename is provided it is used to load the recipe,
    #  otherwise a recipe with no rule is created.
    def __init__( self, filename=None ):
        self.rules = []
        if not filename == None:
            self.loadFromJSON(filename)

    ## Add a rule to this recipe.
    def addRule( self, rule ):
        self.rules.append( rule )

    ## Get the list of rules of this recipe.
    def getRules( self ):
        return self.rules

    ## Load this recipe to a JSON file with the given name.
    def loadFromJSON(self, filename):
        try:

            with open(filename + '.recipe') as f:
                d = json.load(f)

            if d['version'] == '1.0.0':
                self.rules = []
                for r in d['rules']:
                    self.addRule( Rule( r ) )
                return
            
            if d['version'] == '0.1.3':
                print "Warning: expecting version number 1.0.0 in recipe."
                self.rules = []
                for r in d['rules']:
                    self.addRule( Rule( r ) )
                return

            raise moaerror.MoaError( 'Wrong Recipe version (expecting 1.0.0).' )

        except IOError:
            raise moaerror.MoaError( 'Could not load recipe from file ' + filename + '.recipe')
            
    ## Save this recipe to a JSON file with the given name.
    def saveToJSON(self, filename):
        try:
            
            d = {'version':'1.0.0','rules':self.rules}
            with open(filename+'.recipe', 'w') as f:
                json.dump(d, f, indent=4)

        except IOError:
            raise moaerror.MoaError( 'Could not save recipe to file ' + filename + '.recipe')

