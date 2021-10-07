rem g++ -c "D:\VST3 SDK\public.sdk\source\vst2.x\audioeffect.cpp" -DWIN32 -I"D:\VST3 SDK" -I"D:\VST3 SDK\plugininterfaces\source\vst2.x" -I"D:\VST3 SDK\public.sdk\source\vst2.x" -o audioeffect.o -Ofast
rem g++ -c "D:\VST3 SDK\public.sdk\source\vst2.x\audioeffectx.cpp" -DWIN32 -I"D:\VST3 SDK" -I"D:\VST3 SDK\plugininterfaces\source\vst2.x" -I"D:\VST3 SDK\public.sdk\source\vst2.x" -o audioeffectx.o -Ofast
rem g++ -c "D:\VST3 SDK\public.sdk\source\vst2.x\vstplugmain.cpp" -DWIN32 -I"D:\VST3 SDK" -I"D:\VST3 SDK\plugininterfaces\source\vst2.x" -I"D:\VST3 SDK\public.sdk\source\vst2.x" -o vstplugmain.o -Ofast
rem g++ -c "D:\VST3 SDK\vstgui.sf\vstgui\aeffguieditor.cpp" -DWIN32 -I"D:\VST3 SDK" -I"D:\VST3 SDK\plugininterfaces\source\vst2.x" -I"D:\VST3 SDK\public.sdk\source\vst2.x" -o aeffguieditor.o
rem g++ -c "D:\VST3 SDK\vstgui.sf\vstgui\vstgui.cpp" -DWIN32 -trigraphs -DWIN32 -I"D:\VST3 SDK" -I"D:\VST3 SDK\plugininterfaces\source\vst2.x" -I"D:\VST3 SDK\public.sdk\source\vst2.x" -o vstgui.o
rem g++ -c "D:\VST3 SDK\vstgui.sf\vstgui\vstcontrols.cpp" -DWIN32 -I"D:\VST3 SDK" -I"D:\VST3 SDK\plugininterfaces\source\vst2.x" -I"D:\VST3 SDK\public.sdk\source\vst2.x" -o vstcontrols.o

g++ -c *.cpp -DVST_GUI -DUNICODE -DWIN32 -DVST_TWEAKS -I"D:\VST3 SDK" -I"D:\VST3 SDK\vstgui.sf\vstgui" -I"D:\VST3 SDK\plugininterfaces\source\vst2.x" -I"D:\VST3 SDK\public.sdk\source\vst2.x" -Ofast
rem  -DVST_GUI
dllwrap  --output-def libSynthVst.def  --driver-name c++ ^
AdsrEnvelope.o   BasicDelay.o       Stdafx.o       vstgui.o ^
aeffguieditor.o  BasicOscillator.o  Synth.o        vstplugmain.o ^
audioeffect.o    EnvelopeStage.o    SynthVst.o ^
audioeffectx.o   kick_wav.o         vstcontrols.o  ^
-L. --add-stdcall-alias -lole32 -lkernel32 -lgdi32 -lgdiplus -luuid -luser32 -lshell32 -mwindows --no-export-all-symbols --def SynthVst.def ^
-o MidiTrackerSynth.dll -Ofast
xcopy MidiTrackerSynth.dll "C:\Program Files (x86)\VstPlugins\MidiTrackerSynth.dll" /Y