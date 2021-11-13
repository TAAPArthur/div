# Dumb Image Viewer (BETA)
It can view images... it can also do more, but you'll have to tell it to

## Install
```
make
make install
```

## Configuration
1. Create ~/.config/dim/config.c
2. Create a array called user_bindings of type Binding. Also define a method with signature of `void onStartup(){}` see [samples/no_op.c] for an example
3. Run `div -r` to recompile (run after every change to config.c)
4. `div` to actually launch

user_bindings will be used to for any custom bindings you may want. These will be processed before the default bindings and replace them if they conflict.
onStartup will be called on startup and can be used to do whatever you want.

## Goal
This project was born from frustrations with the development of [nxiv](https://github.com/nxiv/nxiv). The goal of this project is to have a simple, minimal image viewer that can easily be extended to any arbitrary usecase. So minimalism and extensibility are the main points of focus.

## Contributing
In line with the goal,
1. Anything should be possible via user config files. And related to this a lot of common ops (env var parsing, cli args, etc) aren't built in. This also means that everything should be able to be disabled from user config files.
2. Some useful functionality that can be done from the config file may be merged to `helpers/`. This is to just prevent duplication work
3. If your usecase can be achieved easier through patching than the config file (this includes use of external tools), consider sending a PR for a new flag
4. After the first version, default behavior shouldn't ever change. This means that your arbitrary usecase should continue to work on upgrades without you needing to do anything
5. Write functions in a modular way so people can reuse pieces of code

## Features
1. Custom keybindings
2. Grid view: see RC images in a RxC grid
3. View images left-to-right or right-to-left
4. Zooming and scaling
5. Under 1000 lines of code. Only other image I'm aware of that is comparable is [lel](https://git.codemadness.org/lel/files.html)
6. Modular design would make it easy to swap out X11/imlib backends

## Non Features
These will probably never be enabled by default, but should be possible to easily achieve. Could possibly make it into `helpers/`
1. Directory loading (easy to due from config.h;)
2. Archive support (extract to a temporary dir)
3. Support for arbitrary image formats (send a PR to the backend)

## TODO
1. Remove Xlib dependency. I wanted to use xcb because it is easier to use. However, imlib doesn't support it, so brought in -lX11-xcb -lX11 to avoid the issue. Ideally we wouldn't have this many dependencies.
2. Remove imlib2 dependency. Perhaps a bold move
2. Add env var loading and cli args to helpers.
