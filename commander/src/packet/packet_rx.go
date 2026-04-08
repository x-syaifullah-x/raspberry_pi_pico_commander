package packet

import (
	"fmt"
	"log"
	"strings"
)

type Code uint16

const (
	// Little Endian
	CodeErr            Code = iota
	CodeSystem              = 0x0001
	CodeDefaultLED          = 0x0002
	CodeADCRead             = 0x0003
	CodeTB6612FNGPower      = 0x0104
	CodeTB6612FNGMotor      = 0x0204
	CodeTB6612FNGState      = 0xFF04
)

type PacketRX struct {
	Code    Code
	ID      uint8
	Payload any
}

type PacketRXErr struct {
	Code    uint8  `json:"code,omitempty"`
	Message string `json:"message,omitempty"`
}

func (e *PacketRXErr) Error() string {
	return e.Message
}

var (
	MaxPacketSize int   = 32
	XState        State = *NewState()
)

func formatSig(b []byte) string {
	names := []string{"ID", "METHODE", "PAYLOAD_0", "PAYLOAD_1", "PAYLOAD_2", "PAYLOAD_3", "PAYLOAD_4", "PAYLOAD_5"}
	parts := make([]string, 0, len(b))
	for i, v := range b {
		name := fmt.Sprintf("byte%d", i)
		if i < len(names) {
			name = names[i]
		}
		parts = append(parts,
			fmt.Sprintf("%s=0x%02X", name, v),
		)
	}
	return "[" + strings.Join(parts, ", ") + "]"
}

func formatRes(b []byte) string {
	names := []string{"STATUS" /* 0=OK | 1=ERR */, "VAL_0", "VAL_1", "VAL_2", "VAL_3", "VAL_4", "VAL_5", "VAL_6", "VAL_7"}
	parts := make([]string, 0, len(b))
	for i, v := range b {
		name := fmt.Sprintf("byte%d", i)
		if i < len(names) {
			name = names[i]
		}
		parts = append(parts,
			fmt.Sprintf("%s=0x%02X", name, v),
		)
	}
	return "[" + strings.Join(parts, ", ") + "]"
}

func Parse(buf []uint8) *PacketRX {
	// log.Printf("SIG: %s", formatSig(buf[0:8]))
	// log.Printf("RES: %s", formatRes(buf[8:]))

	code := Code(uint16(buf[0]) | uint16(buf[1])<<8) // Little Endian
	log.Println(code)
	id := buf[2]
	payload := buf[3:]
	switch code {
	case CodeErr:
		errMessage := "ERR_UNKNOWN"
		errCode := payload[0]
		switch errCode {
		case 0x01:
			errMessage = "ERR_CMD_UNKNOWN"
		case 0x02:
			errMessage = "ERR_CMD_ARG_INVALID"
		}
		return &PacketRX{Code: code, ID: id, Payload: PacketRXErr{Code: errCode, Message: errMessage}}
	case CodeDefaultLED:
		XState.LED = LED{
			Satus:   payload[0],
			DelayMS: uint16(payload[1]) | uint16(payload[2])<<8,
		}
		return &PacketRX{Code: code, ID: id, Payload: XState.LED}
	case CodeADCRead:
		input := payload[0]
		voltage := float64(uint16(payload[1])|uint16(payload[2])<<8) / 100.0
		temp := float64(uint16(payload[3])|uint16(payload[4])<<8) / 100.0
		XState.ADC[input] = ADC{Input: input, Voltage: voltage, Temp: temp}
		return &PacketRX{Code: code, ID: id, Payload: XState.ADC[input]}
	case CodeTB6612FNGPower:
		XState.TB_6612_FNG.Power = payload[0] > 0
		if !XState.TB_6612_FNG.Power {
			for key := range XState.TB_6612_FNG.Motor {
				XState.TB_6612_FNG.Motor[key] = TB_6612_FNG_Motor{Direction: 0, Speed: 0}
			}
		}
		return &PacketRX{Code: code, ID: id, Payload: XState.TB_6612_FNG}
	case CodeTB6612FNGMotor:
		XState.TB_6612_FNG.Motor[payload[0]] = TB_6612_FNG_Motor{
			Direction: payload[1],
			Speed:     payload[2],
		}
		return &PacketRX{Code: code, ID: id, Payload: XState.TB_6612_FNG}
	case CodeTB6612FNGState:
		XState.TB_6612_FNG.Power = payload[0] == 1
		XState.TB_6612_FNG.Motor[0] = TB_6612_FNG_Motor{
			Direction: payload[1],
			Speed:     payload[2],
		}
		XState.TB_6612_FNG.Motor[1] = TB_6612_FNG_Motor{
			Direction: payload[3],
			Speed:     payload[4],
		}
		return &PacketRX{Code: code, ID: id, Payload: XState.TB_6612_FNG}
	default:
		return &PacketRX{Code: code, ID: id, Payload: XState}
	}
}
