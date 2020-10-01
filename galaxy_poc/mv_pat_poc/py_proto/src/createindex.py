import os

import mvlib.dataset as dataset

inputDir = '../data/2013-07-26'
outputDir = 'test'

l = os.listdir(inputDir)
inputNameList = [d for d in l if os.path.splitext(d)[-1] == '.csv']

keptFiles = []
for inputname in inputNameList:

    try:
        nameParts = inputname.split('_')
        outputname = nameParts[6]+'-'+nameParts[7]+'-'+nameParts[4]
    except IndexError:
        print "Incorrect input file name " + inputname
    else:
        d = dataset.Descriptor( { "name": inputDir+'/'+inputname,
                    "ignoredRows1":40,
                    "ignoredRows2":5,
                    "ignoredCols":10,
                    "delimiter":","})

        d.saveToJSON( outputDir+ '/'+outputname+'.input')

        keptFiles.append(outputname)

l = dataset.InputList( _list = keptFiles )
l.saveToJSON( outputDir+'/inputs.list')
