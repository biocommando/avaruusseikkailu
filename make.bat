cd synth
call make.bat
cd ..
g++ main.cpp -Iallegro/include -Isynth allegro\lib\liballegro_monolith.dll.a synth/*.o