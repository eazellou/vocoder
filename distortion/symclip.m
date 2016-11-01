function y=symclip(x)
% y=symclip(x)
% "Overdrive" simulation with symmetrical clipping % x - input
N=length(x) ;
th=1/3; % threshold forsymmetrical soft clipping
        % by Schetzen Formula 
for i=1:1:N,
    if abs(x(i))< th, y(i)=2*x(i); end; 
    if abs(x(i))>=th
        if x(i) > 0, y(i)=(3-(2-x(i)*3) .^2)/3; end;
        if x(i)< 0, y(i)=-(3-(2-abs(x(i))*3).^2)/3; end;
    end;
    if abs(x(i))>2*th
        if x(i)> 0, y(i)=1;end; 
        if x(i)< 0, y(i)=-1;end;
    end;
end;
