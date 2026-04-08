package commander

import (
	"commander/src/packet"
	"commander/src/ws"
	"encoding/json"
	"io"
	"log"
	"os"

	"github.com/gorilla/websocket"
)

func Listen(hub ws.Hub, reader io.Reader) {
	buf := make([]byte, packet.MaxPacketSize)
	for {
		n, err := reader.Read(buf)
		if err != nil {
			for _, client := range hub.Clients {
				client.Conn.WriteMessage(websocket.CloseMessage, websocket.FormatCloseMessage(websocket.CloseGoingAway, "Device disconnected"))
			}
			log.Printf("reader.Read failed: %v", err)
			os.Exit(0)
		}

		packet_rx := packet.Parse(buf[:n])
		res := ws.Response{Event: int32(packet_rx.Code), Data: packet_rx.Payload}
		if packet_rx.Code == packet.CodeErr {
			client := hub.Clients[packet_rx.ID]
			if client != nil {
				payload, _ := json.Marshal(res)
				client.Send <- payload
			}
			continue
		}

		payload, _ := json.Marshal(res)
		hub.Broadcast <- payload
	}
}
