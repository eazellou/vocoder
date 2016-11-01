%take in speech
[speech,fs] = audioread('kill_humans.wav'); 

%take in pitch
FREQ = 150; 
T = 10000 * (1/FREQ); 
dt = 1/fs; 
t = 0:dt:T-dt; 
x = sawtooth(2*pi*FREQ*t) + sawtooth(2*pi*2*FREQ*t) + sawtooth(2*pi*3/2*FREQ*t);

pitch = x(1:length(speech))'; 

out = zeros(size(speech)); 

WINSIZE = 1024; 
WINDOW = sqrt(triang(WINSIZE));

for i=1:WINSIZE/2:length(speech)-WINSIZE
    %ffts 
    Speech = fft(speech(i:i+WINSIZE-1).*WINDOW,WINSIZE); 
    Pitch = fft(pitch(i:i+WINSIZE-1).*WINDOW,WINSIZE); 

    Speech_re = real(Speech); 
    Speech_im = imag(Speech); 
    Pitch_re = real(Pitch); 
    Pitch_im = imag(Pitch); 

    Speech_mag = sqrt(Speech_re.^2 + Speech_im.^2); 
    Pitch_mag = sqrt(Pitch_re.^2 + Pitch_im.^2); 
    Pitch_phase = phase(Pitch);

    %multiply amplitudes, take phase of pitch
    Out_mag = Speech_mag.*Pitch_mag; 
    %Out_mag = Pitch_mag;
    Out = Out_mag.*exp(Pitch_phase * 1i);
    
    %ifft
    out(i:i+WINSIZE-1) = out(i:i+WINSIZE-1) + real(ifft(Out)).*WINDOW; 
end 

out = 0.5*out/max(abs(out));

aP = audioplayer(out,fs);
playblocking(aP);

%plot(out)

