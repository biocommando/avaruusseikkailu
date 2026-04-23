set GPP=g++ -static -c
rem set LINK=-L. --add-stdcall-alias -lole32 -lkernel32 -lgdi32 -lgdiplus -luuid -luser32 -lshell32 -mwindows --no-export-all-symbols
set VSTGUIFLAGS=  -DWIN32 -trigraphs -DWINDOWS -w
set VSTGUILINK=aeffguieditor.o vstcontrols.o vstgui.o timer.o

rem %GPP% D:\code\c\fst\src\FstAudioEffect.cpp -DWINDOWS_GUI %FSTFLAGS% -Ofast

%GPP% "D:\code\c\clapwrap\VSTGui\*.cpp" %VSTGUIFLAGS%

%GPP%  -I../../clap/include -I../../clapwrap -I../../clapwrap/VSTGui -I"D:\code\c\clapwrap\VSTGui" ^
    ../../clapwrap/clapwrap.cpp ^
    *.cpp ..\..\wav_handler\wav_handler.c .\cimpl\wt_sample_reader.c ^
    -DUNICODE -DWIN32 -DVST_GUI -DVST_TWEAKS -DAUTO_GLUE -DPLUGIN_DEBUG_LOG -Ofast

g++ -shared ^
AdsrEnvelope.o BasicDelay.o BasicOscillator.o Synth.o EnvelopeStage.o SynthVst.o kick_wav.o ^
clapwrap.o wav_handler.o wt_sample_reader.o %VSTGUILINK% ^
-L. -lole32 -lkernel32 -lgdi32 -lgdiplus -luuid -luser32 -lshell32 ^
-o MidiTrackerSynth.clap -Ofast

xcopy MidiTrackerSynth.clap "C:\Program Files\Common Files\CLAP" /Y
