> Angel (awm) is a dynamic tiling window manager for X11 written in C using the Xlib api.

https://github.com/user-attachments/assets/ef1698c3-68d0-44c8-86a4-163e684648d7

<img width="1913" height="1077" alt="picture" src="https://github.com/user-attachments/assets/88179fe5-17a2-4577-bdeb-658039117cdc" />

#### Core features

* Nine Tiling layouts 

    1. Simple vertical
    2. Simple Horizontal
    3. Master left
    4. Master right
    5. Monocle
    6. Master master left (a master region who's stack area is itself a master layout)
    7. Master master right
    8. Master left monocle (a master region who's stack area is a monocle layout) 
    9. Master right monocle

* Config file parsing for unique configuration language
* Hot reloading configuration file
* Keyboard | mouse driven actions
* Ten workspaces
* Multi-monitor support
* Dock / bar support
* Minimized window actions
* Subset of ICCCM/EWMH (In progress, the important ones are implemented)
* Floating windows
* Fullscreen windows (per-monitor)
* Window resizing
* Mapping / unmapping instead of minimizing (a mapped window is one visible on the screen. An unmapped window is a window that exists but is not viewable.)

### Dependencies

Angel's build  has three dependencies: `x11`, `xrandr`, and `xwallpaper`.

### Building

To build and install angel, run

```bash
make
sudo make install
```

By default Angel is installed to /usr/local/bin, adjust this behavior by setting `PREFIX`.
```bash
# Install to /usr/bin
sudo make install PREFIX=/usr 
```

Then, choose Angel through your session manager or use `.xinitrc` and `startx`.

```
# ~/.xinitrc
exec angel
```

### A quick note about multi-monitor and focus

Focus mode is one of `mouse`, `pointer`. If the focus mode is `mouse`, new windows will appear on the monitor that holds the mouse. If the focus mode is `pointer`, new windows will appear on the monitor that holds focus. If there is no focus, new windows will appear on the primary monitor, or the monitor that holds the mouse if no primary monitor exists. On startup, focus mode is `pointer`, but windows will appear on the primary monitor until the mouse has moved at least once.

Move between modes by setting binds that target these actions

```
bind $mod-shift-f focus-mode-focus
bind $mod-shift-p focus-mode-pointer
```

See details for more binds below.

Once inside resize mode, focus changes from focus-follows-mouse to click-to-focus.

### Configuration

Angel is configured by placing a `angel.conf` inside `$HOME/.config/angel`. An example configuration file is found in `angel/configs`

You should set the terminal you want to be opened with the open-term bind (and know the bind) before you start your system with Angel managing your windows.

Statements in the configuration file are separated by either a newline or a semicolon. Angel defined identifiers such as names of layouts must not be a string. Color definitions and the path for `set-background` must be a string. The terminal name used in ```define terminal``` is not strictly required to be a string, but must be a string if the name of the terminal exceeds one word. The same goes for names used in `declare` and commands given to `bind-command`. If an environment variable is used, string form is necessary.

#### Comments 

Comments are created in the configuration file with use of #.

```
# Comment
```

#### Health-check

Check the health of your configuration by running

```
angel -c | --check-config-health
```

Note that running Angel with this flag does not cause it to attempt to manage any windows, it merely checks the health of `$HOME/.config/angel/angel.conf`.

#### Refreshing

There exists a bind which reloads Angel. Note that it defaults to `super + shift + r` if it is not set in the config. If you change the configuration while running, you can use this bind to update Angel's knowledge of your configuration. Workspace / monitor state will be preserved on reload.

#### Variables

Variables in the configuration file are defined with `let`. For example,

``` 
let mod = super
let term = "kitty"
```

Variables are then referenced by prefixing the name with a dollar sign.

```
define terminal = $term
```

#### Mod

Set a preferred modifier key by defining a variable (conventionally named mod) with one of: `super`, `ctrl`, `alt`, `shift`. This variable can then be used during bind sets.

#### Exec

Angel can execute commands on startup with the `exec` statement

```
exec "echo 'Angel WM (AWM)'"
```

#### Set-background

Set a background with `set-backgound`

```
set-background <path>
```

#### Setting workspace layouts

Workspace layouts are set in the following way

```
layouts {
    ...
    i -> layout
    ...
}
```

with

$$
i \in {1,2,3,...,10}
,$$

and layout one of :

* angel_master_left
* angel_master_right
* angel_simple_vertical
* angel_simple_horizontal
* angel_monocle
* angel_master_master_left
* angel_master_master_right
* angel_master_left_monocle
* angel_master_right_monocle

For example,

```
layouts {
    1 -> angel_master_left
    2 -> angel_master_right
    3 -> angel_monocle
}
```

If a workspace is omitted, it will revert to what is set in 

```
set default-workspace-layout = <layout>
```

If `set-default-workspace-layout` is omitted, `angel_master_left` is used.

A layout for a workspace can be changed on-the-fly by setting appropriate binds.

```
bind $mod-alt-1 tile_master_left
bind $mod-alt-2 tile_master_right
bind $mod-alt-3 tile_simple_vertical
bind $mod-alt-4 tile_simple_horizontal
bind $mod-alt-5 tile_monocle
bind $mod-alt-6 tile_master_left_monocle
bind $mod-alt-7 tile_master_right_monocle
bind $mod-alt-8 tile_master_master_left
bind $mod-alt-9 tile_master_master_right
```

#### Defines

There are five possible definitions in the Angel configuration that are set by the define keyword.
```
define terminal = ...
define root_cursor = ...
define minimized_height = ...
define new-focus-start = ...
define next-focus-on-close = ...
```

```terminal``` is the name of the application that gets opened by the action `open-term`. `new-focus-start` can be either `adjacent` or `end`. If `new-focus-start` is set to `adjacent`, new windows will appear adjacent to the current focus. If `new-focus-start` is set to `end`, new windows will appear at the end of the layout.

`new-focus-on-close` can be set to either `use-stack` or `next`. If `new-focus-on-close` is set to `use-stack`, the focus stack is used to retrieve the next focus on window close. If `new-focus-on-close` is set to `next`, the next focus is the next logical client in the layout ordering.

Focus changes are added to the internal focus stack. If a focus path is from windows `A` -> `B` -> `C` -> `A` -> `B` and window `B` is closed, focus to `A` is restored. Similarly, if `A` is closed, focus to `C` is restored.

The root-cursor can be one of

    standard_pointer 
    text_insertion 
    busy 
    hand_shape  
    crosshair 
    four_df_resize 
    horizontal_resize 
    vertical_resize 
    top_resize 
    bottom_resize 
    left_resize 
    right_resize 
    top_left_resize 
    top_right_resize 
    bottom_left_resize 
    bottom_right_resize 

Numeric quantities can have suffix `%h` or `%w` to be converted to a percentage of the smallest monitor width or height. For example,

```
define minimized_height = 1%h
```

Uses 1% the height of the monitor with the smallest height.

#### Sets

The possible sets are 

```
set gap = ...
set focused-border-width = ...
set unfocused-border-width = ...
set resize-border-width = ...
set float-border-width = ...
set minimized-border-width = ...
set monocle-border-width = ...
set fullscreen-border-width = ...

set float-move-step = ...
set space-between-monocle = ...
set minimized-height-inc = ...
set gap-inc-size = ...

set minimized-position = ...
set default-workspace-layout = ...
```

Non-numeric quantities are given to `minimized-position` and `default-workspace-layout`. `minimized-position` can be one of `top`, `bottom`, `left`, `right`. The behavior of default-workspace-layout is described above.

#### Colors

The possible color settings are

```
color resize-border = ...
color focused-border = ...
color unfocused-border = ...
color float-border = ...
color fullscreen-border = ...
```

The given color spec must be a string, and can be a general name such as `green`, `blue`, `pink`, a hex color code, or an rgb specification.

```
# where <red>, <green>, and <blue> are each between 1 and 4 hexadecimal digits.
"RGB:<red>/<green>/<blue>"

# where <red>, <green>, and <blue> are floating-point numbers between 0.0 and 1.0, inclusive.
"RGBi:<red>/<green>/<blue>"
```

#### Declares

Declare applications to start either tiled (default) or float with ```declare```.

```
declare <name> float
declare "<name>" float

declare <name> tiled
declare "<name>" tiled
```

#### Bind-command

Bind commands to keys with ```bind-command```.

```
bind-command <key-combo> <command>
```

For example,

```
bind-command $mod-shift-d "$HOME/.config/rofi/launchers/type-2/launcher.sh"
```

#### Binds

Binds are specified via

```
bind <key-combo> <action-identifier>
```

The action-identifiers are 

Action                      | Description                                          |
--------------------------- | ---------------------------------------------------- |
`close-win`                 | Close the focused window                             |
`open-term`                 | Open the terminal set by `define terminal`           |
`win-down`                  | Focus the window below                               |
`win-up`                    | Focus the window above                               |
`win-left`                  | Focus the window to the left                         |
`win-right`                 | Focus the window to the right                        |
`move-win-down`             | Move the focused window down                         |
`move-win-up`               | Move the focused window up                           |
`move-win-left`             | Move the focused window left                         |
`move-win-right`            | Move the focused window right                        |
`move-win-monitor-down`     | Move the focused window to the monitor below         |
`move-win-monitor-up`       | Move the focused window to the monitor above         |
`move-win-monitor-left`     | Move the focused window to the monitor on the left   |
`move-win-monitor-right`    | Move the focused window to the monitor on the right  |
`enter-resize`              | Enter resize mode                                    |
`exit-resize`               | Exit resize mode                                     |
`resize-down`               | Resize the focused window downward                   |
`resize-up`                 | Resize the focused window upward                     |
`resize-left`               | Resize the focused window to the left                |
`resize-right`              | Resize the focused window to the right               |
`toggle-fullscreen`         | Toggle fullscreen for the focused window             |
`toggle-float`              | Toggle the focused window between tiled and floating |
`toggle-float-focus`        | Toggle focus between tiled and floating windows      |
`switch-float-focus`        | Switch focus within the floating window layer        |
`minimize-win`              | Minimize the focused window                          |
`toggle-minimize-focus`     | Toggle focus between normal and minimized windows    |
`tile_master_left`          | Use the master-left tiling layout                    |
`tile_master_right`         | Use the master-right tiling layout                   |
`tile_simple_vertical`      | Use a simple vertical tiling layout                  |
`tile_simple_horizontal`    | Use a simple horizontal tiling layout                |
`tile_monocle`              | Use the monocle layout                               |
`tile_master_left_monocle`  | Use master-left with monocle behavior                |
`tile_master_right_monocle` | Use master-right with monocle behavior               |
`tile_master_master_left`   | Use the master-master-left layout                    |
`tile_master_master_right`  | Use the master-master-right layout                   |
`minimize-left`             | Position the minimized window area on the left       |
`minimize-right`            | Position the minimized window area on the right      |
`minimize-top`              | Position the minimized window area at the top        |
`minimize-bottom`           | Position the minimized window area at the bottom     |
`switch-to-w1`              | Switch to workspace 1                                |
`switch-to-w2`              | Switch to workspace 2                                |
`switch-to-w3`              | Switch to workspace 3                                |
`switch-to-w4`              | Switch to workspace 4                                |
`switch-to-w5`              | Switch to workspace 5                                |
`switch-to-w6`              | Switch to workspace 6                                |
`switch-to-w7`              | Switch to workspace 7                                |
`switch-to-w8`              | Switch to workspace 8                                |
`switch-to-w9`              | Switch to workspace 9                                |
`switch-to-w10`             | Switch to workspace 10                               |
`move-to-w1`                | Move the focused window to workspace 1               |
`move-to-w2`                | Move the focused window to workspace 2               |
`move-to-w3`                | Move the focused window to workspace 3               |
`move-to-w4`                | Move the focused window to workspace 4               |
`move-to-w5`                | Move the focused window to workspace 5               |
`move-to-w6`                | Move the focused window to workspace 6               |
`move-to-w7`                | Move the focused window to workspace 7               |
`move-to-w8`                | Move the focused window to workspace 8               |
`move-to-w9`                | Move the focused window to workspace 9               |
`move-to-w10`               | Move the focused window to workspace 10              |
`restart-manager`           | Restart the window manager                           |
`quit-manager`              | Quit the window manager                              |
`unmap-workspace`           | Unmap the current workspace                          |
`map-workspace`             | Map a previously unmapped workspace                  |
`unmap-window`              | Unmap the focused window                             |
`map-latest-unmap`          | Remap the most recently unmapped window              |
`gap-dec`                   | Decrease window gaps                                 |
`gap-inc`                   | Increase window gaps                                 |
`minimize-inc`              | Increase the minimized-window area                   |
`minimize-dec`              | Decrease the minimized-window area                   |
`float-all-tiled`           | Convert all tiled windows to floating                |
`tile-all-float`            | Convert all floating windows to tiled                |
`minimize-all-windows`      | Minimize all windows                                 |
`unminimize-all-windows`    | Restore all minimized windows                        |
`focus-mode-focus`          | Use focus-based focus mode                           |
`focus-mode-pointer`        | Use pointer-based focus mode                         |
`cycle-tiled-forward`       | Move focus focus in stacking order                   |
`cycle-tiled-backward`      | Move focus backward in stacking order                |
`new-focus-start-adjacent`  | Hot change `new-focus-start` define                  |
`new-focus-start-end`       | Hot change `new-focus-start` define                  |
`next-focus-on-close-focus-stack` | Hot change `next-focus-on-close` define        |
`next-focus-on-close-next`  | Hot change `next-focus-on-close` define              |
