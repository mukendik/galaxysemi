import math
import numpy as np 
import pylab 
import scipy.stats as stats

import mvlib.dataset as dataset

def shapiroWilk( measures, cut=0.01 ):
    measurements = np.ma.masked_invalid(measures).compressed()
    nbcut = int(len(measurements) * cut / 2.0 )
    measurements = sorted(measurements)[nbcut:len(measurements)-nbcut]
    if len(measurements) < 3:
        print 'not enough valid data to graph.'
        s = (float('nan'),float('nan'))
    else:
        try:
            s = stats.shapiro(measurements) 
        except ValueError:
            print "Value Error"
    return s

def sortTests( keptMeasures, keepThreshold = 1e-2 ):
    shaps = []
    for i in range(keptMeasures.shape[1]):
        measurements = keptMeasures.T[i]
        #if keptMeasures.T[i].min() > 0:
        #    measurements = map(lambda x:math.log(max(x,1e-100)), keptMeasures.T[i] )
        #measurements = map(lambda x:abs(x), keptMeasures.T[i] )
        #measurements = map(lambda x:1.0/x, keptMeasures.T[i] )
        pp = 0.00
        sw = shapiroWilk( measurements, pp )
        """
        for p in [0.001, 0.01, 0.02, 0.05]:
            sw2 = shapiroWilk( measurements, p )
            if sw[1] > sw2[1]:
                break
            sw = sw2
            pp = p
        """
        shaps.append((i, keptTests[i], pp, sw[0], sw[1]))

    shaps = [x for x in shaps if x[4]>keepThreshold]
    sshaps = sorted(shaps,key=lambda x:x[4])

    return map(lambda x:(sshaps[x][0],sshaps[x][2]),range(len(shaps)))

def normalTest( measures, cut=0.01, a=1, b=2, c=1 ):
    measurements = np.ma.masked_invalid(measures).compressed()
    nbcut = int(len(measurements) * cut / 2.0 )
    measurements = sorted(measurements)[nbcut:len(measurements)-nbcut]
    if len(measurements) < 3:
        print 'not enough valid data to graph.'
        return
    sp1 = pylab.subplot(a, b, c)
    try:
        stats.probplot(measurements, sparams = 100, dist="chi2", plot=pylab)
    except ValueError:
        print "Value Error"
    except ZeroDivisionError:
        print "Zero Division Error"
    try:
        print stats.shapiro(measurements)
    except ValueError:
        print "Value Error"
    sp2 = pylab.subplot(a, b, c+1)
    try:
        sp2.hist(measurements, 100)
    except ValueError:
        print "Value Error"
    except AttributeError:
        print "Attribute Error"
    pylab.title('Histogram')

# 1. PARSE COMMAND LINE OPTIONS

from optparse import OptionParser

usage = "usage: %prog [options] INPUTNAME"
version = "0.1.1"
parser = OptionParser(usage=usage, version=version)

parser.add_option("-k", "--keepthreshold", dest="keepThreshold",
                  default="1e-2", help="set minimum Shapiro-Wilk p-value to MIN_PVALUE",
                  metavar="MIN_PVALUE")

(options, args) = parser.parse_args()

if len(args) < 1 or not isinstance(args[0],str):
    print "No input file was specified!"
    exit()    

inputName = args[0]

# 2. LOAD INPUTS

import mvlib.dataset as dataset

inputset = dataset.Dataset(inputName)
print( 'Input dataset has ' + str(len(inputset.getParts())) + ' parts and ' + str(len(inputset.getTests())) + ' tests.' )

# 3. SORT TESTS

tests    = inputset.getTests()
transfos = ['normal']*len(tests)
keptMeasures, keptPartsIdx, keptTests = inputset.extract( tests, transfos, filterNans=False )
sites = keptMeasures.T[7] # T[5] # $$$$ Check for test name ?

order = sortTests( keptMeasures, float(options.keepThreshold) )
print 'Kept ' + str(len(order)) + ' tests.'


# 4. DISPLAY QQ-PLOTS

for i,p in order:

    f = pylab.figure()
    print i, p, keptTests[i]

    print shapiroWilk( keptMeasures.T[i], p )
    normalTest(keptMeasures.T[i], cut=p, a=2, b=2, c=1 )
    normalTest(keptMeasures.T[i][sites==0], a=4, b=4, c=9)
    normalTest(keptMeasures.T[i][sites==1], a=4, b=4, c=11)
    normalTest(keptMeasures.T[i][sites==2], a=4, b=4, c=13)
    normalTest(keptMeasures.T[i][sites==3], a=4, b=4, c=15)
    
    pylab.show(block=False)
    raw_input("Press Enter to close figures...")

    pylab.close(f)
