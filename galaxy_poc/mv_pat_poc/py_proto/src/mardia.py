#from matlab
"""
% Check number of input arguements
if (nargin < 1) || (nargin > 2)
    error('Requires one or two input arguments.')
end

% Define default ALPHA if only one input is provided
if nargin == 1, 
    alpha = 0.05;
end

% Check for validity of ALPHA
if ~isscalar(alpha) || alpha>0.5 || alpha <0
    error('Input ALPHA must be a scalar between 0 and 0.5.')
end

[n,p] = size(X);

% Check for validity of X
if p < 2 || ~isnumeric(X)
    error('Input X must be a numeric array with at least 2 columns.')
end


difT = [];
for	j = 1:p
   difT = [difT,(X(:,j) - mean(X(:,j)))];
end;

S = cov(X);                     % Variance-covariance matrix
D = difT * inv(S) * difT';      % Mahalanobis' distances matrix
b1p = (sum(sum(D.^3))) / n^2;   % Multivariate skewness coefficient
b2p = trace(D.^2) / n;          % Multivariate kurtosis coefficient

k = ((p+1)*(n+1)*(n+3)) / ...
    (n*(((n+1)*(p+1))-6));      % Small sample correction
v = (p*(p+1)*(p+2)) / 6;        % Degrees of freedom
g1c = (n*b1p*k) / 6;            % Skewness test statistic corrected for small sample (approximates to a chi-square distribution)
g1 = (n*b1p) / 6;               % Skewness test statistic (approximates to a chi-square distribution)
P1 = 1 - chi2cdf(g1,v);         % Significance value of skewness
P1c = 1 - chi2cdf(g1c,v);       % Significance value of skewness corrected for small sample

g2 = (b2p-(p*(p+2))) / ...
    (sqrt((8*p*(p+2))/n));      % Kurtosis test statistic (approximates to a unit-normal distribution)
P2 = 1-normcdf(abs(g2));        % Significance value of kurtosis
"""

import math
import numpy as np
import scipy.stats as stats
from sklearn.utils.extmath import pinvh

X = np.random.multivariate_normal([5.,10.,15.], [[1.,1./2.,1./3.],[1./2.,1.,1./2.],[1./3.,1./2.,1.]], 2000)
alpha = 0.05

n,p = X.shape
print n,p

Xc = np.array(X.T)
for j in range(p):
   Xc[j] = Xc[j]-Xc[j].mean()
Xc = Xc.T

#cov = np.cov(X.T, bias=1)
cov = np.dot(Xc.T, Xc) / Xc.shape[0]
#MD2 = np.dot(Xc, pinvh(cov)) * Xc
MD2 = np.dot(np.dot(Xc, pinvh(cov)), Xc.T)

Skew = sum(sum(np.power(MD2,3))) / n**2

k = float((p+1)*(n+1)*(n+3)) / float(n*(((n+1)*(p+1))-6)) # Small sample correction
v = float(p*(p+1)*(p+2)) / 6.0                         # Degrees of freedom

g1c = float(n*Skew*k) / 6.0            # Skewness test statistic corrected for small sample (approximates to a chi-square distribution)
p1c = 1.0 - stats.chi2.cdf(g1c,v) # Significance value of skewness corrected for small sample

g1 = float(n*Skew) / 6.0             # Skewness test statistic (approximates to a chi-square distribution)
p1 = 1.0 - stats.chi2.cdf(g1,v) # Significance value of skewness

print Skew, p1c, p1

Kurt = np.trace(np.power(MD2,2)) / n

g2 = float(Kurt-(p*(p+2))) / (math.sqrt(float(8*p*(p+2))/float(n))) # Kurtosis test statistic (approximates to a unit-normal distribution)
p2 = 1.0-stats.norm.cdf(abs(g2)) # Significance value of kurtosis

print Kurt, p2
