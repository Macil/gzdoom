import * as osc from "osc-min";

const listener = Deno.listenDatagram({
  transport: "udp",
  hostname: "127.0.0.1",
  // We're only sending, so we don't need to listen on port 9001. Leave that
  // port open for other processes and use an automatic port instead.
  port: 0,
});

const msg = `Some text here\n${new Date()}\n${Math.random()}`;
const dv = osc.toBuffer({
  address: "/gzdoom/networkCommand",
  args: [
    "external/in/screenMessage",
    new TextEncoder().encode(msg + "\0"),
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
console.log("Sent:", JSON.stringify(msg));
