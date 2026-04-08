package commander

import (
	"io"
	"net"
	"net/http"
	"time"

	"commander/src/packet"
	"commander/src/ws"

	"github.com/gorilla/websocket"
)

var (
	dummyDataId = map[string]uint8{"TOKEN_A": 1, "TOKEN_B": 2}
	upgrader    = websocket.Upgrader{CheckOrigin: func(r *http.Request) bool { return true }}
)

func ServeWS(hub *ws.Hub, w http.ResponseWriter, r *http.Request, writer io.Writer, s *packet.State) {
	conn, _ := upgrader.Upgrade(w, r, nil)
	conn.SetReadDeadline(time.Now().Add(5 * time.Second))
	var auth ws.Auth
	err := conn.ReadJSON(&auth)
	if err != nil {
		if ne, ok := err.(net.Error); ok && ne.Timeout() {
			conn.WriteMessage(websocket.CloseMessage, websocket.FormatCloseMessage(websocket.ClosePolicyViolation, "Authentication timeout (no response within 5 seconds)"))
		} else {
			conn.WriteMessage(websocket.CloseMessage, websocket.FormatCloseMessage(websocket.CloseUnsupportedData, "Invalid JSON payload for authentication"))
		}
		return
	}
	conn.SetReadDeadline(time.Time{})

	id := dummyDataId[auth.Token]
	if id == 0 {
		conn.WriteMessage(websocket.CloseMessage, websocket.FormatCloseMessage(websocket.ClosePolicyViolation, "Missing token"))
		return
	}

	conn.WriteJSON(ws.Response{Event: -1 /* EventInitialize */, Data: s})

	client := &ws.Client{
		ID:   id,
		Conn: conn,
		Send: make(chan []byte, 256),
		Hub:  hub,
	}

	hub.Register <- client

	go client.WritePump()
	go client.ReadPump(writer)
}
