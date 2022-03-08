# Dumb Image Viewer
It can view images... it can also do more, but you'll have to tell it to. Think of it as a library to create your own image view that happens to have useable defaults.

## Install
```
make
make install
```

The following are the dependant libs
` -lxcb -lxcb-keysyms -lxcb-icccm -lxcb-image -limgloader`
Note that [libimageloader](https://github.com/TAAPArthur/libimageloader) is just a wrapper around other image loaders.

## Configuration
1. Create ~/.config/dim/config.c
2. Create a array called user_bindings of type Binding. Also define a method with signature of `void onStartup(){}` see [samples/no_op.c] for an example
3. Run `div --recompile` to recompile (run after every change to config.c)
4. `div` to actually launch

user_bindings will be used to for any custom bindings you may want. These will be processed before the default bindings and replace them if they conflict.
onStartup will be called on startup and can be used to do whatever you want.

## Goal
This project was born from frustrations with the development of [nxiv](https://github.com/nxiv/nxiv). The goal of this project is to have a simple, minimal image viewer that can easily be extended to any arbitrary usecase. So minimalism and extensibility are the main points of focus.

## Contributing
In line with the goal,
1. Anything should be possible via user config files. Relatedly all default behavior is controlled by defaults.c which is basically a default config file.
2. Some useful functionality that can be done from the config file may be merged to either defaults.c or to functions.c. This is to just prevent duplication work
3. If your usecase can be achieved easier through patching than the config file (this includes use of external tools), consider sending a PR for a new flag
4. Write functions in a modular way so people can reuse pieces of code

## Features
1. Custom keybindings
2. Grid view: see RC images in a RxC grid (which is a generalization of a dual page view)
3. View images left-to-right or right-to-left
4. Zooming and (basic) scaling
5. Under 1000 lines of code. Only other image I'm aware of that is comparable is [lel](https://git.codemadness.org/lel/files.html) and [meh](https://github.com/jhawthorn/meh) both of which are far less customizable.
6. Modular design would make it easy to swap out X11/imlib backends
7. Ability to load from zip files, directories, pipes and urls thanks to the flexible backend
8. Can react to any arbitrary X events

## Things (trivially) possible from config file
1. Panning across images
2. Mouse bindings and key release
3. Spawning external commands
4. Reloading image of disk change (ie via inotify)
5. Loading args from the environment or config file
6. Custom cli options

## TODO
* Consider adding example configs
* Be consistent with variable naming

## FAQ
### The image is distorted/ The image is distorted while zooming
Change the scaling function. By default a really cheap, fast scaling function is used but it doesn't produce the best images. You can roll your own or use a 3rd party one. I recommend [stb_image_resize.h](https://github.com/nothings/stb/blob/master/stb_image_resize.h). Another option is to just disable zooming (state.zoom=1) and/or disable scaling (state.scale_mode=SCALE_NORMAL) which will just use the raw image.
