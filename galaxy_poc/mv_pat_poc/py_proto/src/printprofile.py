import pstats

p = pstats.Stats('profile')

#p.strip_dirs().sort_stats(-1).print_stats()
p.strip_dirs().sort_stats('cumulative').print_stats()
#p.print_stats()

#p.sort_stats('cumulative').print_stats(10)
#p.sort_stats('time').print_stats()
#p.sort_stats('file').print_stats('__init__')
#p.sort_stats('name')
#p.sort_stats('time', 'cum').print_stats(.5, 'init')
#p.print_callers(.5, 'init')
p.print_callees()
#p.add('profile')
