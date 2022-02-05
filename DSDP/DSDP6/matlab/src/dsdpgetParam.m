function [dsdpParam] = dsdpgetParam()
% Get parameters for DSDP

maxiter          = 100;
tol              = 1e-06;
sigma            = 0.1;
alpha            = 0.7;
pObj             = 1e+05;
rho              = 3.0;
initmethod       = "fro"; % "eigs", "linesearch", "IdS"
initbeta         = 1.0; % 1.0; % 10.0
reuse            = 0;
ncorr            = 0;
ncorrdual        = 0;
corrdelta        = 10;
corralpha        = 0.6;
mu               = 1e+06;
tau              = 1.0;
kappa            = 1.0;
dualInfeasalpha  = 0.75;
stepsizestrategy = "eigs"; % linesearch
maxpfeasiter     = 30;
dualcorrstep     = "eigs"; % "linesearch"; % eigs
phase1candmu     = [0.1, 0.4, 0.7, 1.0];
ndash            = 90;
presolve         = true;
normalizer       = 1e+20;
pInfeasIter      = 50;
pInfeasAlpha     = 0.75;
pInfeasSuspicion = 1e+08;

% Max iteration
dsdpParam{1} = maxiter;

% Tolerance
dsdpParam{2} = tol;

% Sigma
dsdpParam{3} = sigma;

% Alpha
dsdpParam{4} = alpha;

% Initial pObj
dsdpParam{5} = pObj;

% Rho
dsdpParam{6} = rho;

% Initialization
dsdpParam{7} = initmethod; % "eigs", "linesearch", "IdS"
dsdpParam{10} = initbeta;

% Reuse
dsdpParam{8} = reuse;

% Corrector step
dsdpParam{9} = ncorr;
dsdpParam{27} = ncorrdual;

% Initial mu
dsdpParam{11} = mu;

% Initial tau and kappa
dsdpParam{12} = tau; % tau
dsdpParam{13} = kappa; % kappa

% Alpha for dual infeasibility elimination
dsdpParam{14} = dualInfeasalpha;

% Strategy of getting stepsize
dsdpParam{15} = stepsizestrategy; % "linesearch"

% Corrector alpha
dsdpParam{16} = corralpha;

% Primal feasibility proof
dsdpParam{17} = maxpfeasiter;
dsdpParam{18} = dualcorrstep;
dsdpParam{19} = phase1candmu;
dsdpParam{24} = pInfeasIter;
dsdpParam{25} = pInfeasAlpha;
dsdpParam{26} = pInfeasSuspicion;

% Logging dash
dsdpParam{20} = ndash;

% Presolver
dsdpParam{21} = presolve;

% Corrector delta
dsdpParam{22} = corrdelta;
dsdpParam{23} = normalizer;


end % End function