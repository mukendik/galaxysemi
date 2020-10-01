#!/bin/bash
# ssh -L 5901:localhost:5900 -XC gexuser@dourdan.galaxysemi.com  -p 722 
# ssh -L 5901:localhost:5900 -XC root@dourdan.galaxysemi.com -p 722 

#8022 > 1.80 : neptune Windows

ssh -L 5901:localhost:5900 -XC gexprod@dourdan.galaxysemi.com -p 7722

#ultravncviewer1082.exe localhost:5901
