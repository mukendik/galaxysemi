#!/bin/bash
# ssh -L 5901:localhost:5900 -XC gexuser@dourdan.galaxysemi.com  -p 722 
# ssh -L 5901:localhost:5900 -XC root@dourdan.galaxysemi.com -p 722 

ssh -L 5901:localhost:5900 -XC licman@dourdan.galaxysemi.com -p 422

ultravncviewer localhost:5901
