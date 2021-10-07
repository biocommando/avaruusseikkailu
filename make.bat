cd synth
call make.bat
cd ..
g++ main.cpp -Iallegro/include -Isynth allegro\lib\liballegro_monolith.dll.a ^
synth/AdsrEnvelope.o   synth/BasicDelay.o synth/BasicOscillator.o  synth/Synth.o ^
synth/EnvelopeStage.o  synth/kick_wav.o