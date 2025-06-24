import * as osc from "osc-min";

function formatOscArg(arg: osc.OscArgOutputOrArray): unknown {
  switch (arg.type) {
    case "array":
      return arg.value.map(formatOscArg);
    case "blob":
      return {
        type: "blob",
        data: new Uint8Array(
          arg.value.buffer,
          arg.value.byteOffset,
          arg.value.byteLength
        ),
        asString: new TextDecoder().decode(arg.value),
      };
    default:
      return arg;
  }
}

function formatOscMessage(message: osc.OscPacketOutput): unknown {
  switch (message.oscType) {
    case "bundle":
      return {
        oscType: "bundle",
        timetag: message.timetag,
        elements: message.elements.map(formatOscMessage),
      };
    case "message":
      return {
        address: message.address,
        args: message.args.map(formatOscArg),
      };
    default:
      return message;
  }
}

const listener = Deno.listenDatagram({
  transport: "udp",
  hostname: "127.0.0.1",
  // When receiving, we need to listen on the port that gzdoom is sending to.
  port: 9001,
});

console.log("Listening...");
for await (const [data, _address] of listener) {
  const oscMessage = osc.fromBuffer(data);
  console.log(formatOscMessage(oscMessage));
}
