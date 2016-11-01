%take in speech
[speech,fs] = audioread('kill_humans.wav'); 

%take in pitch
FREQ = 150; 
T = 10000 * (1/FREQ); 
dt = 1/fs; 
t = 0:dt:T-dt; 
x = sawtooth(2*pi*FREQ*t) + sawtooth(2*pi*2*FREQ*t) + sawtooth(2*pi*3/2*FREQ*t);

% make noise
whiteNoise = rand(size(speech));

pitch = x(1:length(speech))'; 

out = zeros(size(speech)); 

WINSIZE = 1024; 
WINDOW = sqrt(triang(WINSIZE));

considerNoise = true;
noiseThreshold = 120;

for i=1:WINSIZE/2:length(speech)-WINSIZE
    
    currentSpeech = speech(i:i+WINSIZE-1);
    shouldUseNoise = noiseZeroCrossings(currentSpeech, noiseThreshold);
    
    %ffts 
    Speech = fft(currentSpeech.*WINDOW,WINSIZE); 
    Pitch = fft(pitch(i:i+WINSIZE-1).*WINDOW,WINSIZE); 
    White = fft(whiteNoise(i:i+WINSIZE-1).*WINDOW,WINSIZE);
    
    if considerNoise && shouldUseNoise
        Carrier = White;
    else
        Carrier = Pitch;
    end

    Speech_re = real(Speech); 
    Speech_im = imag(Speech); 
    Carrier_re = real(Carrier); 
    Carrier_im = imag(Carrier); 

    Speech_mag = sqrt(Speech_re.^2 + Speech_im.^2); 
    Carrier_mag = sqrt(Carrier_re.^2 + Carrier_im.^2); 
    Carrier_phase = phase(Carrier);

    %multiply amplitudes, take phase of pitch
    Out_mag = Speech_mag.*Carrier_mag; 
    %Out_mag = Pitch_mag;
    Out = Out_mag.*exp(Carrier_phase * 1i);
    
    %ifft
    out(i:i+WINSIZE-1) = out(i:i+WINSIZE-1) + real(ifft(Out)).*WINDOW; 
end 

out = 0.5*out/max(abs(out));

aP = audioplayer(out,fs);
playblocking(aP);

%plot(out)

