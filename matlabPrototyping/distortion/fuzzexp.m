function y = fuzzexp(x, gain, mix) 
% y=fuzzexp(x, gain, mix)
% Distortion based on an exponential function % x - input
% gain - amount of distortion, >0->
% mix - mix of original and distorted sound, 1=only distorted

% f(x) = (x / abs(x)) * (1 - e^(x^2 / abs(x)))

q = x * gain / max(abs(x));

z = sign(-q).*(1-exp(sign(-q).*q));

y = mix * z * max(abs(x)) / max(abs(z)) + (1-mix) * x;

y = y * max(abs(x)) / max(abs(y));

end