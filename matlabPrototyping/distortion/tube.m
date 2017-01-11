function [y] = tube(x, gain, Q, dist, rh, rl, mix)
% y=tube(x, gain,Q, dist, rh, rl, mix)
% "Tube distortion" simulation, asymmetrical function
% x -       input
% gain -    the amount of distortion, >0->
% Q -       work point. Controls the linearity of the transfer
%           function for low input levels, more negative=more linear
% dist -    controls the distortion's character, a higher number gives
%           a harder distortion, >0
% rh        abs(rh)<i, but close to 1. Placement of poles in the HP filter
%           which removes the DC component
% rl-       O<rl<l. The pole placement in the LP filter used to simulate 
%           capacitances in a tube amplifier
% mix -     mix of original and distorted sound, 1=only distorted 
q=x*gain/max(abs(x)); %Normalization
if Q==0
    z=q./(l-exp(-dist*q));
    for i=1:length(q) %Test because of the
        if q(i)==Q %transfer function's 
            z(i)=1/dist; %O/O value inQ
        end; 
    end;
else
    z=(q-Q)./(1-exp(-dist*(q-Q)))+Q/(1-exp(dist*Q));
    for i=1:length(q) %Test because of the 
        if q(i)==Q    %transfer function's
            z(i)=1/dist+Q/(1-exp(dist*Q)); %O/O value inQ 
        end;
    end;
end;
y = mix*z*max(abs(x))/max(abs(z))+(1 - mix) * x;
y = y * max(abs(x))/max(abs(y));
y = filter([1 -2 1],[1 -2*rh rh^2],y);   %HP filter
y = filter([1 -rl],[1 -rl],y);           %LP filter

