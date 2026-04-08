package ws

type Hub struct {
	Clients    map[uint8]*Client
	Broadcast  chan []byte
	Register   chan *Client
	Unregister chan *Client
}

func NewHub() *Hub {
	return &Hub{
		Clients:    make(map[uint8]*Client),
		Broadcast:  make(chan []byte),
		Register:   make(chan *Client),
		Unregister: make(chan *Client),
	}
}

func (h *Hub) Run() {
	for {
		select {
		case c := <-h.Register:
			h.Clients[c.ID] = c

		case c := <-h.Unregister:
			if _, ok := h.Clients[c.ID]; ok {
				delete(h.Clients, c.ID)
				close(c.Send)
			}

		case msg := <-h.Broadcast:
			for _, value := range h.Clients {
				select {
				case value.Send <- msg:
				default:
					close(value.Send)
					delete(h.Clients, value.ID)
				}
			}
		}
	}
}
