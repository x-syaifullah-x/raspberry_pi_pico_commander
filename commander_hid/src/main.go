package main

import (
	"flag"
	"log"
	"net/http"
	"os"
	"os/signal"
	"syscall"

	commander "commander/src"
	"commander/src/packet"
	"commander/src/ws"

	"github.com/sstallion/go-hid"
)

const (
	Port = ":8081"
)

func main() {
	sig := make(chan os.Signal, 1)
	signal.Notify(sig, syscall.SIGTERM, syscall.SIGINT)

	go func() {
		<-sig
		os.Exit(0)
	}()

	if err := hid.Init(); err != nil {
		log.Fatal(err)
	}
	defer hid.Exit()

	vid := flag.Uint("vid", 0, "USB vendor id")
	pid := flag.Uint("pid", 0, "USB product id")
	serial := flag.String("serial", "", "device serial")

	flag.Parse()

	device, err := hid.Open(uint16(*vid), uint16(*pid), *serial)
	if err != nil {
		log.Fatalf("Invalid open device: %v", err)
	}
	defer device.Close()

	buf := make([]byte, 64)
	desc, err := device.GetReportDescriptor(buf)
	if err != nil {
		log.Fatalf("Invalid read descriptor: %v", err)
	}
	packet.MaxPacketSize = GetReportCount(ParseDescriptor(buf[:desc]))

	commands := [][]byte{
		{0xFF, packet.CodeDefaultLED, 0xFF}, // DEFAULT_LED_STATUS
		{0xFF, packet.CodeADCRead, 0x04},    // ADC_ONBOARD_STATUS
		{0xFF, packet.CodeTB6612FNG, 0xFF},  // METHODE_TB_6612_FNG_STATUS
	}

	buf = make([]byte, packet.MaxPacketSize)
	for _, command := range commands {
		if _, err := device.Write(command); err != nil {
			log.Fatalf("failed to write command: %v", err)
		}

		if _, err := device.Read(buf); err != nil {
			log.Fatalf("failed to read response: %v", err)
		}
		// ws.PendingRequests.Store(command[0], command[1:])

		// packet.Parse(buf, &ws.PendingRequests)
		packet.Parse(buf)
	}

	hub := ws.NewHub()
	go hub.Run()
	go commander.Listen(*hub, device)

	http.HandleFunc("/raspberry_pi_pico", func(w http.ResponseWriter, r *http.Request) {
		commander.ServeWS(hub, w, r, device, &packet.XState)
	})

	log.Printf("Server running on %s", Port)
	if err = http.ListenAndServe(Port, nil); err != nil {
		log.Fatal(err)
	}
}
