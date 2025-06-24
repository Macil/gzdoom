import { ChatClient } from "@twurple/chat";
import * as osc from "osc-min";

if (Deno.args.length === 0) {
  console.error("Usage: deno run -NE twitch.ts CHANNEL [CHANNEL ...]");
  Deno.exit(1);
}
const channels = Deno.args;

const listener = Deno.listenDatagram({
  transport: "udp",
  hostname: "127.0.0.1",
  // We're only sending, so we don't need to listen on port 9001. Leave that
  // port open for other processes and use an automatic port instead.
  port: 0,
});

const chatClient = new ChatClient({ channels });

const maxLines = 5;
const lines: string[] = [];

chatClient.onConnect(() => {
  console.log("Connected to Twitch chat channels:", channels);
});

chatClient.onMessage(
  async (_channel: string, user: string, text: string, _msg) => {
    const line = `${user}: ${text}`;
    console.log(line);
    lines.push(line);
    while (lines.length > maxLines) {
      lines.shift();
    }

    const dv = osc.toBuffer({
      address: "/gzdoom/networkCommand",
      args: [
        "external/in/screenMessage",
        new TextEncoder().encode(lines.join("\n") + "\0"),
      ],
    });
    await listener.send(
      new Uint8Array(dv.buffer, dv.byteOffset, dv.byteLength),
      {
        transport: "udp",
        hostname: "127.0.0.1",
        port: 9000,
      },
    );
  },
);

chatClient.connect();
