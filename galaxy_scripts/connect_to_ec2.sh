#!/bin/bash

#ssh -L 5901:localhost:5900 -XC ubuntu@ec2-176-34-76-139.eu-west-1.compute.amazonaws.com -p 22 -i GexDev_Amazon_Key.pem

# -X : X11 forward
# -C : compress
# -i : 

#ssh -C ubuntu@ec2-176-34-76-139.eu-west-1.compute.amazonaws.com -p 22 -i GexDev_Amazon_Key.pem

# which pem :  GexDev_Amazon_Key.pem ? WTS-Key.pem ?
ssh -C ubuntu@ec2-174-129-99-120.compute-1.amazonaws.com -p 22 -i WTS-Key.pem


#ultravncviewer localhost:5901
