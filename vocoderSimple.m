%take in speech
[speech,fs] = audioread('kill_humans.wav'); 
%take in pitch
FREQ = 300; 
T = 10000 * (1/FREQ); 
dt = 1/fs; 
t = 0:dt:T-dt; 
x = sawtooth(2*pi*FREQ*t); 

pitch = x(1:length(speech))'; 

out = zeros(size(speech)); 

WINSIZE = 1024; 

for i=1:WINSIZE:length(speech)-WINSIZE
    %ffts 
    Speech = fft(speech(i:i+WINSIZE-1),WINSIZE); 
    Pitch = fft(pitch(i:i+WINSIZE-1),WINSIZE); 

    Speech_re = real(Speech); 
    Speech_im = imag(Speech); 
    Pitch_re = real(Pitch); 
    Pitch_im = imag(Pitch); 

    Speech_mag = sqrt(Speech_re.^2 + Speech_im.^2); 
    Pitch_mag = sqrt(Pitch_re.^2 + Pitch_im.^2); 
    Pitch_phase = atan(Pitch_im ./ Pitch_re); 

    %multiply amplitudes, take phase of pitch
    Out_mag = Speech_mag.*Pitch_mag; 
    %Out_re = Out_mag .* cos(Pitch_phase); 
    %Out_im = Out_mag .* sin(Pitch_phase); 
    Out = Out_mag.*exp(Pitch_phase * 1i);
    
    %ifft
    out(i:i+WINSIZE-1) = ifft(Out); 
end 
%out