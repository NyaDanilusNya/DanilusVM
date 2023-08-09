# DanilusVM

A small virtual machine written in C

### About

Toy virtual machine, written in C, uses Lua 5.2 and SDL (maybe someday in the future the virtual machine will emulate the processor, RAM, VRAM)

### Requirements

#### Debian-like

```
make
libsdl2-2.0-0
libsdl2-dev
liblua5.2-0
liblua5.2-dev
clang
```

#### Arch-like

```
make
sdl2
lua52
clang
```

If you are using a Debian family operating system, you can use this command:

```shell
$ sudo apt install make libsdl2-2.0-0 libsdl2-dev liblua5.2-0 liblua5.2-dev clang
```

If you are using an Arch family operating system, you can use this command:

```shell
$ sudo pacman -S make sdl2 lua52 clang
```

### Build 'n' Run

To compile, use the following command:

```shell
$ make build
```

The compiled program will be in `./bin/main`

The files of your operating system must be on the path `./FS/*` (relative to the `./main` file). The entry point is the `init.lua` file (you can change file system path in `vm.cfg`)

To run the VM, enter the program directory (`./bin` by default) and run `./main`, or use:

```shell
$ make run
```

Also you can use this command to compile and run:

```shell
$ make
```

### Config file

The VM configuration is done via the `vm.cfg` file.

At the moment you can configure the CPU speed (`cpu_clock`), the amount of RAM (`ram_size`), the time after which the system will be killed in case of a dead loop, screen resolution(`window_w` and `window_h`), as well as the path to the file system of VM.

### TODO:

- [ ] Module system of devices (in progress)

- [x] VRAM and textures

- [ ] Networking

- [ ] In the distant future: assembler, emulating CPU, RAM, mini C-lang

### API

`gpu` - an interface for drawing graphics on the screen

`gpu.setcolor(number color_rgb) -> nil` - sets the drawing color (`0xRRGGBB`)

`gpu.getcolor() -> number` - returns the current color

`gpu.getpixel(number x, number y) -> number`- returns the color of the specified pixel

`gpu.clear() -> nil` - clears the screen by filling it with the currently set color

`gpu.fill(number x, number y, number w, number h) -> nil` - draws a rectangle

`gpu.copy(number x, number y, number w, number h, number xo, number yo) -> nil` - copies a piece of the screen and shifts it by the specified (ox, oy) number of pixels

`gpu.update() -> nil` - update screen

`gpu.getresolution() -> number, number` - returns the screen resolution

---

`computer` - an interface for working with the virtual machine (computer)

`computer.gettotal() -> number` - returns the total amount of RAM

`computer.getused() -> number` - returns the amount of used RAM

`computer.pushevent(...) -> nil` - accepts any numbers and strings, creates an event and adds to the queue (max nums of args is 8)

`computer.pullevent(number timeout) -> ...` - retrieves information about an event by taking it from the event queue (returns all values as a string!) (timeout in ms)

- `keydown -> "keydown", scancode` - returns the event name and the scancode. Triggered when a key is pressed on the keyboard

- `keyup -> "keyup", scancode` - returns the event name and the scancode. Triggered when a key is released on the keyboard 

- `mousemotion -> "mousemotion", x, y` - returns the name of the event and the x and y position of the mouse. Triggered when the mouse is moves

- `mousedown -> "mousedown", button` - returns the event name and the button that pressed (1,2,3 for LMB, MMB, RMB). Triggered when the mouse is pressed

- `keyup -> "keyup", scancode` - returns the event name and the button that released (1,2,3 for LMB, MMB, RMB). Triggered when the mouse is released

---

`filesystem` - an interface for working with filesystem of VM

`filesystem.exists(string path) -> bool` - checks if a file exists

`filesystem.isdir(string path) -> bool` - checks if a folder exists

`filesystem.mkdir(string path) -> bool` - creates a folder, if successful - returns true

`filesystem.size(string path) -> bool, number` - returns true if successful and the size of the specified file

`filesystem.rmfile(string path) -> bool` - deletes the file (or folder) if it is empty, returns true if successful

`filesystem.rmdir(string path) -> bool` - deletes the folder if it is empty, returns true if successful

`filesystem.listdir(string path) -> bool, table` - reads a folder and returns true if successful and an array with the names of files and folders

`filesystem.rename(string oldname, string newname) -> bool` - renames a file, returns true if successful

`filesystem.open(string path, string mode) -> bool, number` - opens a file, returns true if successful and returns a file descriptor (max nums of open files is 10)

`filesystem.close(number fd) -> bool` - closes a file, returns true if successful

`filesystem.write(number fd, string data) -> bool` - writes data to file, returns true if successful

`filesystem.read(number fd, number bytes) -> bool, string` - reads a certain number of bytes, returns true and the read bytes if successful

`filesystem.seek(number fd, number type, number offset) -> bool` - shifts the cursor by a certain number of bytes, the type indicates what to shift relative to

- `1` - set (beginning of file)

- `2` - cursor

- `3` - end (end of file)

`filesystem.getpos(number fd) -> bool, number` - returns true if successful and the position of the cursor in a file

---

**Notes:** 

- if you try `while true do end` - the VM will be killed after 10 seconds (can be configured in `vm.cfg`)
