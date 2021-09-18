g++ -c *.cpp -DWIN32 -DNO_SOFTCLIP -I"D:\VST3 SDK" -I"D:\VST3 SDK\vstgui.sf\vstgui" -I"D:\VST3 SDK\plugininterfaces\source\vst2.x" -I"D:\VST3 SDK\public.sdk\source\vst2.x" -Ofast

dllwrap  --output-def libSynthVst.def  --driver-name c++ ^
*.o ^
-L. --add-stdcall-alias -lole32 -lkernel32 -lgdi32 -lgdiplus -luuid -luser32 -lshell32 -mwindows --no-export-all-symbols --def SynthVst.def ^
-o MidiTrackerSynth.dll -Ofast