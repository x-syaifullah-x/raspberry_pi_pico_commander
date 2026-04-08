package ws

import (
	"io"

	"github.com/gorilla/websocket"
)

type Client struct {
	ID   uint8
	Conn *websocket.Conn
	Send chan []byte
	Hub  *Hub
	buf  [65]byte
}

func (c *Client) ReadPump(device io.Writer) {
	defer func() {
		c.Hub.Unregister <- c
		c.Conn.Close()
	}()

	for {
		_, msg, err := c.Conn.ReadMessage()
		if err != nil {
			break
		}

		c.buf[0] = c.ID
		// nano := time.Now().UnixNano()
		// req_id := uint64(nano) ^ rand.Uint64()
		x := 1
		copy(c.buf[x:], msg)
		newMsg := c.buf[:x+len(msg)]
		if _, err = device.Write(newMsg); err != nil {
			c.Conn.WriteJSON(Response{Error: ErrResponse{Code: 0xFF, Message: err.Error()}})
			continue
		}
	}
}

func (c *Client) WritePump() {
	defer c.Conn.Close()

	for msg := range c.Send {
		c.Conn.WriteMessage(websocket.TextMessage, msg)
	}
}
