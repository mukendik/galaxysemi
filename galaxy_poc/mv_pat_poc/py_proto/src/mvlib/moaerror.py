#-------------------------------------------------------------------------------
# File name: moaerror.py
# Created:   2013-07-01
# Author:    Jerome Kodjabachian
#
# This code is part of the Multivariate Outlier Analysis (MOA)
# for Part Average Testing (PAT) prototype. 
#
# (C) 2013 by Galaxy Semiconductor Inc. All rights reserved.
#-------------------------------------------------------------------------------

## @defgroup utility Project Utility Module

## The class of exceptions raised by the MOA library.
## @ingroup utility
class MoaError( Exception ):

    # Create an exception (Constructor).
    def __init__(self, value):
        self.value = value
        
    # Convert the value of this exception to a printable string.
    def __str__(self):
        return repr(self.value)

