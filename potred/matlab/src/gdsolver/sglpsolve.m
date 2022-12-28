function [x, y, s] = sglpsolve(c, A, b, G, g, x0, y0, maxit, quiet)
% Solve LP using penalty based method using a sharp objective
% min  c' * x
% s.t.  A * x  = b
%       G * x >= g
%           x >= 0

[meq, n] = size(A);
[mineq, n2] = size(G);

if meq == 0 || mineq == 0
    if meq > 0
        G = sparse(0, n);
        n2 = n;
    else
        A = sparse(0, n2);
        n = n2;
    end % End if
end % End if
      
assert( n == n2 );
ineqidx = meq+1:meq+mineq;
K = [A; G];
q = [b; g];

% [D, E, K] = pcscale(K, 1); 

m = meq + mineq;

if isempty(x0)
    x = ones(n, 1);
    y = zeros(m, 1);
else
    x = x0;
    y = y0;
end % End if

tau = 1;

pr = 1;
dr = 1;
pdr = 1;
aa = 1;

xwindow = zeros(n, 16);
ywindow = zeros(m, 16);
count = 1;
fbest = inf;
xbest = x0;
ybest = y0;
freq = 1000;

for i = 1:maxit
    
    % Subgradient
    Kx = K * x - q * tau;
    Kx(ineqidx) = min(Kx(ineqidx), 0);
    dx = K' * sign(Kx);
    KTy = K' * y - c * tau;
    KTy = max(KTy, 0);
    dy = K * sign(KTy);
    cpl = c' * x - q' * y;
    dx = pr * dx + pdr * c * sign(cpl);
    dy = dr * dy - pdr * q * sign(cpl);
    
    % Residual
    pres = sum(abs(Kx));
    dres = sum(abs(KTy));
    cres = abs(cpl);
    fval = pr * pres + dr * dres + pdr * cres;
    
    if fval < 1e-08
        break;
    end % End if
    
    if fval < fbest
        fbest = fval;
        xbest = x;
        ybest = y;
    end % End if
    
    % Polyak
    nrmdx = norm(dx);
    nrmdy = norm(dy);
    alpha = max(aa * fval, 1.5 * fbest) / (nrmdx^2 + nrmdy^2);
    
    % Update
    x = x - alpha * dx;
    y = y - alpha * dy;
    
    % Projection
    x = max(x, 0);
    y(ineqidx) = max(y(ineqidx), 0);
    
    xwindow(:, count) = x;
    ywindow(:, count) = y;
    if count == 32
        count = 0;
%         x = mean(xwindow, 2);
%         y = mean(ywindow, 2);
    end % End if
    count = count + 1;
    
    if mod(i, 10) == 1 && ~quiet
        fprintf("%4d %5.2e %5.2e %5.2e | %5.2e \n", i, pres, dres, cres, fval);
    end % End if

    if mod(i, freq) == 1
        aa = aa * 0.95;
    end % End if

end % End for

x = xbest;
y = ybest;
s = max(c - K' * y, 0);

end % End function