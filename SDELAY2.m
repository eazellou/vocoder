function [OutArray] = SDELAY2(InArray)
%This function takes an input matrix
% and adds an audio delay to it

Fs = 44100;         %sampling rate

DelayLength = .35;  %set delay length in seconds
Feedback = 0.7;    %feedback value

DelayArrLength = round(DelayLength * Fs);               %get length of array to hold delay
DelayArray = zeros(DelayArrLength,2);                   %array to hold the delay
BufferHolder = zeros(1,2);                              %allows to add on previous delay array values
NumDelay = round(length(InArray) / DelayArrLength) + 20;    %number of delays we'll be performing or something
                                                        %adding delays can vastly
                                                        %change the length of the
                                                        %OutArray, and will have the
                                                        %most affect on how long the
                                                        %processing takes.
                                                        
OutArray = zeros(DelayArrLength * NumDelay,2);     %array to hold output

PadArray = zeros(length(OutArray),2);

%make a new zero padded input array
for x = 1:1:length(InArray)
    PadArray(x,:) = InArray(x,:);
end

%do the delay the calculated number of times
for i = 1:1:NumDelay
    
    %this loop writes the delay
    for j = (DelayArrLength * (i - 1) + 1):1:DelayArrLength * i
        
        writeIndex = j - DelayArrLength * (i - 1); %index to keep track of where to write from
        BufferHolder(1,:) = DelayArray(writeIndex,:); %hold the values from the previous delay (feedback)
        
        %this is the delay, which only happens after the first (length of delay) seconds
        if i > 1
            DelayArray(writeIndex,:) = PadArray(j - DelayArrLength,:) + Feedback * BufferHolder(1,:);
        end
        
        OutArray(j,:) = DelayArray(writeIndex,:);
    end
end