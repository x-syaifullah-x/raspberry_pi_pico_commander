package packet

type ADC struct {
	Input   uint8   `json:"input"`
	Voltage float64 `json:"voltage"`
	Temp    float64 `json:"temp"`
}
