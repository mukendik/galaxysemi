from optparse import OptionParser

usage = "usage: %prog COMMAND [arguments]"
version = "1.0.0"
parser = OptionParser(usage=usage, version=version)

(options, args) = parser.parse_args()

if len(args) < 1:
    print "No command name was specified!"
    exit()

if args[0] == 'compute':
    import mvcompute
    mvcompute.main( args[1:] )
