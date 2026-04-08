package packet

type State struct {
	LED         LED           `json:"led"`
	ADC         map[uint8]ADC `json:"adc,omitempty"`
	TB_6612_FNG TB_6612_FNG   `json:"tb_6612_fng"`
}

func NewState() *State {
	return &State{
		ADC: map[uint8]ADC{},
		TB_6612_FNG: TB_6612_FNG{Power: false,
			Motor: map[uint8]TB_6612_FNG_Motor{},
		}}
}
