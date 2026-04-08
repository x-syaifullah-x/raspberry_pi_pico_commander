package packet

type LED struct {
	Satus   uint8  `json:"status"`
	DelayMS uint16 `json:"delay_ms,omitempty"`
}
