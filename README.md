## How to set up the project

download this repo,
bash:
```
git clone https://github.com/Adi12-dev/path-tracer.git
```

bash:
```
git clone --depth 1 https://github.com/raysan5/raylib.git raylib
cd raylib/src/
make PLATFORM=PLATFORM_DESKTOP GRAPHICS=GRAPHICS_API_OPENGL_43
```

After this a new file named libraylib.so will appear in raylib/src

Copy this file to the (this project dir)/libs/raylib/raylib4.3 replacing the old file.


## How to run the Ray Tracer

Go to this project's root directory.

### Make the run script executable:
**For linux:**\
bash:
```
chmod +x run.sh
```
you need to do this only once.

No need to make run.sh executable for Windows.

### Bash the run script:
bash:
```
./run.sh
```

do this every time you want to run this program.