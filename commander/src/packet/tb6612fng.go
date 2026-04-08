package packet

type TB_6612_FNG_Motor struct {
	Direction uint8 `json:"direction"`
	Speed     uint8 `json:"speed"`
}
type TB_6612_FNG struct {
	Power bool                        `json:"power"`
	Motor map[uint8]TB_6612_FNG_Motor `json:"motor"`
}
