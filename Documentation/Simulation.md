# HW Simulation

You can simulate HW with Mynewt by setting the BSP to `@apache-mynewt-core/hw/bsp/native`,
 building, then running the app as detailed below.

### Set the BSP to `native`

```
$ newt target set targetname bsp=@apache-mynewt-core/hw/bsp/native
```

### Rebuild the target as a local binary executable

```
$ newt build targetname
```

When the build process is complete you should see something like:

```
Linking bleuart.elf
App successfully built: /Users/ktown/Dropbox/microBuilder/code/nRF52/Mynewt/bletest/bin/bleuart/apps/bleuart/bleuart.elf
```

### Running the native binary

Depending on the output `.elf` file generated about, you can then run the simulation binary locally via:

```
$ /Users/ktown/Dropbox/microBuilder/code/nRF52/Mynewt/bletest/bin/bleuart/apps/bleuart/bleuart.elf
```

### Connect to UART via minicom

If you have Console or Shell supported included, you will see the TTY port to connect to, such as:

```
uart0 at /dev/ttys002
```

You can connect to this using minicom in a new terminal window via:

```
$ minicom -D /dev/ttys002
```

If you don't already have minicom installed, you can install it via `brew install minicom`.

Enter `?` for a list of commands available.

To exit minicom on OS X press `esc+x`. 
