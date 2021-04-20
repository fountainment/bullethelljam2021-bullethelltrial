"C:/Program Files (x86)/Steam/steamapps/common/Aseprite/Aseprite.exe" -b pixel_bullet.aseprite --save-as textures/bullet_{tag}{tagframe00}.png
"C:/Program Files (x86)/Steam/steamapps/common/Aseprite/Aseprite.exe" -b pixel_cover.aseprite --save-as textures/cover_{tag}{tagframe00}.png
crunch.exe assets/atlases/atlas textures/ -j -p -u -t -s2048 -p8
