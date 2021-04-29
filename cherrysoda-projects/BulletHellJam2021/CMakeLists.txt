set(CHERRYSODA_EXECUTABLE_SRC
  src/Program.cpp
  src/Program.h
  src/BulletHellTrial.cpp
  src/BulletHellTrial.h
  src/MainScene.cpp
  src/MainScene.h
)

set(CHERRYSODA_SHADER_SRC
  shaders/varying.def.sc
  shaders/vs_screenspacequad.sc
  shaders/fs_screenspacequad.sc
  shaders/vs_background.sc
  shaders/fs_background.sc
)

add_cherrysoda_executable(BulletHellJam2021)
