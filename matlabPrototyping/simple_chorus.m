    function [y] = simple_chorus(x,fs)

    % parameters to vary 
    DMAX=0.030; %ms
    DMIN=0.005; %ms
    index=1:length(x);

    %noise to create varying delay
    noise_ref = rand([1,length(index)]); 
    [B,A]=butter(2,12*2/fs,'low'); 
    noise_ref = filter(B,A,noise_ref);

    const_ref = ones(size(index)); 

    %convert delay in ms to max delay in samples
    DMAXSamp=round(DMAX*fs);
    DMINSamp=round(DMIN*fs); 

    % create empty out vector
    y = zeros(length(x),1);

    % to avoid referencing of negative samples
    y(1:DMAXSamp)=x(1:DMAXSamp);

    % set amp suggested coefficient 
    depth=0.6;

    % for each sample
    for i = (DMAXSamp+1):length(x)

    ref=noise_ref(i); %abs of current sin val 0-1

    % generate delay 
    cur_delay=ceil(ref*DMAXSamp);

    if (cur_delay < DMINSamp)
        cur_delay = DMINSamp; 
    end

    % add delayed sample
    y(i) = (x(i)) + depth*(x(i-cur_delay));
    end
    end 
