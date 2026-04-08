const ws = new WebSocket("ws://localhost:8083/raspberry_pi_pico")

ws.onclose = (e) => {
  console.log("Server closed:", e);
}

ws.onopen = (x) => {
  ws.send(JSON.stringify({
    token: "TOKEN_A",
  }));
};

ws.onmessage = (event) => {
  console.log("Message from server:", event.data);
}

const sleep = (ms) => new Promise(resolve => setTimeout(resolve, ms));
for (let index = 0; index < 100; index++) {
  await sleep(1);
  ws.send(new Uint8Array([0x02, 0xFF]))
}

// ws.send(new Uint8Array([value]))