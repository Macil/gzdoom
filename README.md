# gzdoom-ec

This is an experimental fork of GZDoom that allows external programs to communicate with game mods. This is accomplished by making GZDoom into an OSC (Open Sound Control) server and client.

Builds are not provided currently, but you can build it yourself by following the build instructions further below.

Future versions of this fork will probably switch away from OSC over UDP to a better protocol, but for now, OSC is used because it is easy to implement. (The current setup with OSC over UDP doesn't allow multiple clients to connect and listen to events, doesn't allow clients to even tell if they're connected, doesn't have guaranteed or in-order delivery of events, etc. A connection-oriented protocol would be much better. Also, instead of listening to a local port by default, which isn't a secure default because it could be accessed by other unprivileged users on the same system, it would be best to have GZDoom listen on a Unix domain socket or named pipe.)

## Example Usage

1. Install [Deno](https://deno.com/) to run the example scripts. (If you want to edit the scripts, you can use [Visual Studio Code](https://code.visualstudio.com/) with the [Deno extension](https://marketplace.visualstudio.com/items?itemName=denoland.vscode-deno) or [something else](https://docs.deno.com/runtime/getting_started/setup_your_environment/).)
2. Run GZDoom using Doom 2 (or [freedoom2.wad](https://freedoom.github.io/download.html)) with the `ec/examples/pk3` directory as a parameter, and then start a new game. You should find yourself in the example map in a room with a screen that says "Hello World".
3. While the game is running, run any of the scripts in the `ec/examples/deno` directory using `deno run -A`.

### Example Scripts

- **[listen.ts](./ec/examples/deno/listen.ts)**: Listens for events from GZDoom and prints them to the console. Player chat messages will show up. The example map's [ZScript code](./ec/examples/pk3/zscript/ec_examples.zs) will cause a message to show up whenever a monster dies and whenever the light switch is toggled.
- **[send.ts](./ec/examples/deno/send.ts)**: Sends text to render on-screen in the example map.
- **[twitch.ts](./ec/examples/deno/twitch.ts)**: Connects to a Twitch chat channel and relays chat messages to render on-screen in the example map. You must pass a Twitch channel name as an argument to this script: `deno run -A twitch.ts CHANNEL`.

## Supported Messages

GZDoom will listen on 127.0.0.1:9000/udp for incoming OSC messages. Supported addresses:
- `/gzdoom/networkCommand`: Sends a [NetworkCommand event](https://zdoom.org/wiki/Events_and_handlers#Network_Commands_and_Buffers) to ZScript. The OSC message must have a string argument representing the command name (which must begin with "external/in/") and then a blob argument containing the command data.

GZDoom will send OSC messages to 127.0.0.1:9001/udp for outgoing events. Supported addresses:
- `/gzdoom/networkCommand`: Sent when ZScript calls `EventHandler.SendNetworkCommand()` with a command that begins with "external/out/". The first argument will be the command name as a string, and the second argument will be the command data as a blob.
- `/gzdoom/chat`: Sent when a chat message is sent in the game. The first argument will be the player name as a string, and the second argument will be the message text as a string.

**Original GZDoom README follows:**

---

# Welcome to GZDoom!

[![Continuous Integration](https://github.com/ZDoom/gzdoom/actions/workflows/continuous_integration.yml/badge.svg)](https://github.com/ZDoom/gzdoom/actions/workflows/continuous_integration.yml)

## GZDoom is a modder-friendly OpenGL and Vulkan source port based on the DOOM engine

Copyright (c) 1998-2023 ZDoom + GZDoom teams, and contributors

Doom Source (c) 1997 id Software, Raven Software, and contributors

Please see license files for individual contributor licenses

Special thanks to Coraline of the EDGE team for allowing us to use her [README.md](https://github.com/3dfxdev/EDGE/blob/master/README.md) as a template for this one.

### Licensed under the GPL v3
##### https://www.gnu.org/licenses/quick-guide-gplv3.en.html
---

## How to build GZDoom

To build GZDoom, please see the [wiki](https://zdoom.org/wiki/) and see the "Programmer's Corner" on the bottom-right corner of the page to build for your platform.

# Resources
- https://zdoom.org/ - Home Page
- https://forum.zdoom.org/ - Forum
- https://zdoom.org/wiki/ - Wiki
- https://dsc.gg/zdoom - Discord Server
- https://docs.google.com/spreadsheets/d/1pvwXEgytkor9SClCiDn4j5AH7FedyXS-ocCbsuQIXDU/edit?usp=sharing - Translation sheet (Google Docs)
