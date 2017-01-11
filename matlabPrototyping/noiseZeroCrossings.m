function [ shouldUseNoise ] = noiseZeroCrossings( signal, threshold )
% numZeroCrossings signal should be replaced with noise if numZeroCrossings
% exceeds threshold
    
    crossingCount = 0;

    for i = 2:length(signal)
        if sign(signal(i - 1)) ~= sign(signal(i))
            if(signal(i) ~= 0)
                crossingCount = crossingCount + 1;
            end
        end
    end

    if crossingCount > threshold
        shouldUseNoise = true;
    else
        shouldUseNoise = false;
    end
end
