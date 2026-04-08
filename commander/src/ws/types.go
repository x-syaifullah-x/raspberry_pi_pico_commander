package ws

type Auth struct {
	Token string `json:"token"`
}

type ErrResponse struct {
	Code    uint8  `json:"code"`
	Message string `json:"message,omitempty"`
}

func (e ErrResponse) Error() string {
	return e.Message
}

type Response struct {
	Event int32 `json:"event"`
	Data  any   `json:"data,omitempty"`
	Error error `json:"error,omitempty"`
}
