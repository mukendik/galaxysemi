Multivariate Outlier Analysis Prototype version 0.9.0
=====================================================

To test this early distribution of the Multivariate Outlier Prototype,
the corresponding Python 2.7 distribution must be installed on a Windows
platform (see python-dist-moa.zip for more details).

Extract the contents of this archive anywhere, go to the work directory
and run the console.bat sript to open a DOS command window.

To create a recipe, type    > MV_GROUP sample28

To apply the recipe, type   > MV_COMPUTE sample28

To graph some results, type > MV_GRAPH sample28 -g 6
                            > MV_GRAPH sample28 -g 6 -p

The MV_RECIPE and MV_RUN make it possible to run a batch of MV_GROUP
or MV_COMPUTE commands on a list of inputs; for instance:

> MV_RECIPE samples -r sample_recipe

> MV_COMPUTE samples -r sample_recipe

Type the name of the MV command plus -h for more details on the available
options.

Note: The first time a certain group dimension is encountered in MV_COMPUTE
a table of conversion from Mahalanobis Distance to Zscore is created and
cached. The next time, the cached table will be used to speed up the
computation significantly.
