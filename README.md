# DanilusVM

A small virtual machine written in C

### About

Toy virtual machine, written in C, uses Lua 5.2 and SDL (in the future there will be a programming language)

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
# apt install make libsdl2-2.0-0 libsdl2-dev liblua5.2-0 liblua5.2-dev clang
```

If you are using a Arch family operating system, you can use this command:

```shell
# pacman -S make sdl2 lua52 clang
```

### Build 'n' Run

To compile, use the following command:

```shell
$ make
```

The compiled program will be in `./bin/main`

The files of your operating system must be on the path ./FS/* (relative to the program file). The entry point is the `init.lua` file (you can change file system path in `vm.cfg`)

To run the VM, enter the program directory (`./bin` by default) and run `./main`, or use:

```shell
$ make run
```

### Config file

The VM configuration can be done in the `vm.cfg` file.

At the moment you can configure the processor speed(`cpu_clock`), the amount of RAM(`ram_size`), the time after which the system will be killed in case of a dead loop, as well as the path to the ile system of VM.

### API

`gpu` - library for drawing graphics on the screen.

*(at the moment it is not possible to get the screen resolution from Lua, it is 800x600)*

`gpu.setColor(number color) -> nil` - sets the color to draw

`gpu.getColor() -> number` - returns the current color

`gpu.getPixel(number x, number y) -> number`- returns the color of the specified pixel

`gpu.clear() -> nil` - clears the screen by filling it with the currently set color
`gpu.fill(number x, number y, number w, number h) -> nil` - draws a rectangle

`gpu.copy(number x, number y, number w, number h, number xo, number yo) -> nil` - copies a piece of the screen and shifts it by the specified (ox, oy) number of pixels

`gpu.update() -> nil` - update screen

---

`computer` - library for working with a virtual machine (computer)

`computer.getTotal() -> number` - returns the total amount of RAM

`computer.getUsed() -> number` - returns the amount of used RAM

`computer.pushEvent(...) -> nil` - accepts any numbers and strings, creates an event and adds to the queue

`computer.pullEvent(number timeout) -> ...` - retrieves information about an event by taking it from the event queue (returns all values ​​as a string!) (timeout in ms)

- `keydown -> "keydown", string` - returns the event name and scancode. Triggered when a key is pressed on the keyboard

- `keyup -> "keyup", string` - returns the event name and scancode. Triggered when a key is released on the keyboard 

- (Yes, that's all for now :)

---

**Notes:** 

- if you try `while true do end` - the VM will be killed after 10 seconds (can be configured in `vm.cfg`)
