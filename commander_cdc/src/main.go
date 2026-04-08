package main

import (
	"errors"
	"flag"
	"log"
	"net/http"
	"os"
	"os/signal"
	"syscall"

	commander "commander/src"
	"commander/src/packet"
	"commander/src/ws"

	"go.bug.st/serial"
	"go.bug.st/serial/enumerator"
)

const (
	Port = ":8080"
)

func main() {
	sig := make(chan os.Signal, 1)
	signal.Notify(sig, syscall.SIGTERM, syscall.SIGINT)

	go func() {
		<-sig
		os.Exit(0)
	}()

	vid := flag.String("vid", "", "USB vendor id")
	pid := flag.String("pid", "", "USB product id")
	serialNumber := flag.String("serial", "", "device serial")
	flag.Parse()

	var serialPort serial.Port
	defer func() {
		serialPort.Close()
	}()
	ports, err := enumerator.GetDetailedPortsList()
	if err != nil {
		log.Fatal(err)
	}
	if len(ports) == 0 {
		log.Fatal("No serial ports found!")
	}

	for _, port := range ports {
		if port.VID == *vid && port.PID == *pid && port.SerialNumber == *serialNumber {
			mode := &serial.Mode{
				BaudRate: 115200,
			}
			serialPort, err = serial.Open(port.Name, mode)
			if err != nil {
				log.Fatal(err)
			}
		}
	}

	commands := [][]byte{
		{0xFF, packet.CodeDefaultLED, 0xFF}, // DEFAULT_LED_STATUS
		{0xFF, packet.CodeADCRead, 0x04},    // ADC_ONBOARD_STATUS
		{0xFF, packet.CodeTB6612FNG, 0xFF},  // METHODE_TB_6612_FNG_STATUS
	}

	buf := make([]byte, packet.MaxPacketSize)
	for _, command := range commands {
		if _, err := serialPort.Write(command); err != nil {
			if portErr, ok := errors.AsType[*serial.PortError](err); ok {
				switch portErr.Code() {
				case serial.PortClosed:
					os.Exit(0)
				}
			} else {
				log.Fatalf("failed to write command: %v", err)
			}
		}

		if _, err := serialPort.Read(buf); err != nil {
			if portErr, ok := errors.AsType[*serial.PortError](err); ok {
				switch portErr.Code() {
				case serial.PortClosed:
					os.Exit(0)
				}
			} else {
				log.Fatalf("failed to read response: %v", err)
			}
		}

		// ws.PendingRequests.Store(command[0], command[1:])
		// packet.Parse(buf, &ws.PendingRequests)
		packet.Parse(buf)
	}

	hub := ws.NewHub()
	go hub.Run()

	go commander.Listen(*hub, serialPort)

	http.HandleFunc("/raspberry_pi_pico", func(w http.ResponseWriter, r *http.Request) {
		commander.ServeWS(hub, w, r, serialPort, &packet.XState)
	})

	log.Printf("Server running on %s", Port)
	if err = http.ListenAndServe(Port, nil); err != nil {
		log.Fatal(err)
	}
}
