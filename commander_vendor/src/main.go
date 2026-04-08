package main

import (
	commander "commander/src"
	"commander/src/packet"
	"commander/src/ws"
	"flag"
	"fmt"
	"log"
	"net/http"
	"os"
	"os/signal"
	"strconv"
	"syscall"

	"github.com/google/gousb"
)

type endpoints struct {
	iface int
	in    int
	out   int
}

const (
	Port = ":8083"
)

func findBulkEndpoints(cfg *gousb.Config) (endpoints, error) {
	var ep endpoints
	var found bool
	for _, iface := range cfg.Desc.Interfaces {
		for _, alt := range iface.AltSettings {
			var inEp, outEp int
			for _, ep := range alt.Endpoints {
				if ep.TransferType != gousb.TransferTypeBulk {
					continue
				}
				switch ep.Direction {
				case gousb.EndpointDirectionIn:
					if ep.Address == 0x84 {
						inEp = ep.Number
					}
				case gousb.EndpointDirectionOut:
					if ep.Address == 0x04 {
						outEp = ep.Number
					}
				}
			}
			if inEp != 0 && outEp != 0 {
				ep = endpoints{iface: alt.Number, in: inEp, out: outEp}
				found = true
				break
			}
		}
		if found {
			break
		}
	}

	if !found {
		return ep, fmt.Errorf("no interface with bulk IN and OUT endpoints found")
	}
	return ep, nil
}

func main() {
	sig := make(chan os.Signal, 1)
	signal.Notify(sig, syscall.SIGTERM, syscall.SIGINT)

	go func() {
		<-sig
		os.Exit(0)
	}()

	ctx := gousb.NewContext()
	defer ctx.Close()

	vidString := flag.String("vid", "0", "USB vendor id")
	pidString := flag.String("pid", "0", "USB product id")
	flag.Parse()

	vid, err := strconv.ParseUint(*vidString, 0, 16)
	if err != nil {
		log.Fatalf("invalid vid: %v", err)
	}
	pid, err := strconv.ParseUint(*pidString, 0, 16)
	if err != nil {
		log.Fatalf("invalid pid: %v", err)
	}

	dev, err := ctx.OpenDeviceWithVIDPID(gousb.ID(vid), gousb.ID(pid))
	if err != nil {
		log.Fatalf("Could not open a device: %v", err)
	}
	defer dev.Close()

	cfg, err := dev.Config(1)
	if err != nil {
		log.Fatalf("config: %v", err)
	}
	defer cfg.Close()

	dev.SetAutoDetach(true)

	ep, err := findBulkEndpoints(cfg)
	if err != nil {
		log.Fatal(err)
	}

	iface, err := cfg.Interface(ep.iface, 0)
	if err != nil {
		log.Fatalf("interface: %v", err)
	}
	defer iface.Close()

	epOut, err := iface.OutEndpoint(ep.out)
	if err != nil {
		log.Fatalf("%s.OutEndpoint(7): %v", iface, err)
	}

	epIn, err := iface.InEndpoint(ep.in)
	if err != nil {
		log.Fatalf("%s.InEndpoint(7): %v", iface, err)
	}

	commands := [][]byte{
		{0xFF, 0x02, 0xFF}, // DEFAULT_LED_STATUS
		{0xFF, 0x03, 0x04}, // ADC_ONBOARD_STATUS
		{0xFF, 0x04, 0xFF}, // METHODE_TB_6612_FNG_STATUS
	}

	buf := make([]byte, packet.MaxPacketSize)
	for _, command := range commands {
		if _, err := epOut.Write(command); err != nil {
			log.Fatalf("failed to write command: %v", err)
		}

		n, err := epIn.Read(buf)
		if err != nil {
			log.Fatalf("failed to read response: %v", err)
		}

		packet.Parse(buf[:n])
	}

	hub := ws.NewHub()
	go hub.Run()
	go commander.Listen(*hub, epIn)

	http.HandleFunc("/raspberry_pi_pico", func(w http.ResponseWriter, r *http.Request) {
		commander.ServeWS(hub, w, r, epOut, &packet.XState)
	})

	log.Printf("Server running on %s", Port)
	if err = http.ListenAndServe(Port, nil); err != nil {
		log.Fatal(err)
	}
}
