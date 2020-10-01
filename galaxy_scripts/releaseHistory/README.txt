Example of use:

# Generate a V7.4 release history for several products:
$ node src/releaseHistory.js --force --families='["GEX","YM","PAT"]' --minor="V7.4" --releases='["V7.4.0","V7.4.RC1","V7.4.E","V7.4.D","V7.4.G","V7.4.B","V7.4.A"]' --output target/history_v74.htm

# Generate a V7.4 release history for YM:
$ node src/releaseHistory.js --force --families='["YM"]' --minor="V7.4" --releases='["V7.4.0","V7.4.RC1","V7.4.E","V7.4.D","V7.4.G","V7.4.B","V7.4.A"]' > target/_ym_history_v74.htm

$ cd galaxy_qa/releaseHistory
$ node src/releaseHistory.js ---families='["GEX"]' --minor="V7.5" --releases='["V7.5.0-alpha","V7.5.0-beta","V7.5.0-gamma"]' > target/_gexpro_history_v75
Version V7.5.0-alpha KO!
- GCORE-4875 KO!
- GCORE-4756 KO!
- GCORE-4291 KO!
- GCORE-4267 KO!
- GCORE-4261 KO!
- GCORE-4202 KO!
- GCORE-4192 KO!
- GCORE-3974 KO!
- GCORE-3610 KO!
- GCORE-3606 KO!
- GCORE-3605 KO!
- GCORE-3604 KO!
- GCORE-3603 KO!
- GCORE-3602 KO!
Version V7.5.0-beta KO!
- GCORE-5074 KO!
- GCORE-3283 KO!
[ERROR] - Fix releases before building Release History.

TODO:
* validation by comparisons

