# Raspberry Pi Pico

Firmware for Raspberry Pi Pico used by RaspberryPiPicoCommander.

---

# Hardware Wiring

## 🔌 TB6612FNG Motor Driver Wiring

```text
Pi Pico               TB6612FNG
------------          -----------------
PIN_04 (GP2)  <---->  STBY
PIN_05 (GP3)  <---->  PWMA
PIN_06 (GP4)  <---->  AIN2  ----->  AO2  <---->  MOTOR A (-)
PIN_07 (GP5)  <---->  AIN1  ----->  AO1  <---->  MOTOR A (+)
PIN_08 (GND)  <---->  GND
PIN_09 (GP6)  <---->  BIN1  ----->  BO1  <---->  MOTOR B (+)
PIN_10 (GP7)  <---->  BIN2  ----->  BO2  <---->  MOTOR B (-)
PIN_11 (GP8)  <---->  PWMB
PIN_36 (3V3)  <---->  VCC
                      VM    <----------------->  POWER SUPPLY (+)
                      GND   <----------------->  POWER SUPPLY (-)
```
---

# Packet Layout

## Request Packet Format

```sh
printf '\x[ID]\x[CMD]\x[CMD_ARG_X]' > /dev/hidrawN
```

## Packet RX (Request)

| Byte     | Field       | Description        |
|----------|-------------|--------------------|
| `0`      | `ID`        | Packet ID          |
| `1`      | `CMD`       | Command            |
| `2..64`  | `CMD_ARG_X` | Command Argument X |

## Packet TX (Response)

| Byte   | Field               | Description            |
|--------|---------------------|------------------------|
| `0`    | `TYPE`              | Type Paylad            |
| `2`    | `ID`                | Packet ID              |
| `3+`   | `PAYLOAD`           | Command result payload |

---

# 🔄 System Control

## Reboot System

### Packet RX (Request)

| Byte   | Field       | Value                  | Description       |
|--------|-------------|:----------------------:|-------------------|
| `0`    | `ID`        | `<0x01..0xFF>`         | Packet ID         |
| `1`    | `CMD`       | `0x01`                 | System command    |
| `2`    | `CMD_ARG_0` | `0x01`                 | Normal reboot     |
|        |             | `0x02`                 | Reboot to BOOTSEL |

### Packet TX (Response)

| TYPE         | ID        | PAYLOAD     |
|:------------:|:---------:|:-----------:|
| `byte[0..1]` | `byte[2]` | `byte[3+]`  |
| `<0xFFFF>`   | `<0xFF>`  | `<0xFFFFF>` |

---

# 💡 Default LED Control

## Set LED State

### Packet RX (Request)

| Byte   | Field       | Value                  | Description    |
|--------|-------------|:----------------------:|----------------|
| `0`    | `ID`        | `<0x01..0xFF>`         | Packet ID      |
| `1`    | `CMD`       | `0x02`                 | LED command    |
| `2`    | `CMD_ARG_0` | `0x00`                 | Mode OFF       |
|        |             | `0x01`                 | Mode ON        |
|        |             | `0x02`                 | Mode Blink     |
|        |             | `0xFF`                 | Get LED status |
| `3`    | `CMD_ARG_1` | `<0x00..0xFF>`         | Blink delay    |

## Get LED State

### Packet RX (Request)

| Byte   | Field       | Value                  | Description |
|--------|-------------|:----------------------:|-------------|
| `0`    | `ID`        | `<0x01..0xFF>`         | Packet ID   |
| `1`    | `CMD`       | `0x02`                 | LED command |
| `2`    | `CMD_ARG_0` | `0xFF`                 | Status      |

### Packet TX (Response)

| TYPE         | ID        | MODE                 | DELAY_MS       |
|:------------:|:---------:|:--------------------:|:--------------:|
| `byte[0..1]` | `byte[2]` | `byte[3]`            | `byte[4..5]`   |
| `<0xFFFF>`   | `<0xFF>`  | `<0x00\|0x01\|0x02>` | `<0xFFFF>`       |

| MODE   | DESCRIPTION |
|--------|-------------|
| `0x00` | LED OFF     |
| `0x01` | LED ON      |
| `0x02` | LED Blink   |

---

# 📊 Read ADC

## Read ADC Onboard

### Packet RX (Request)

| Byte   | Field       | Value                  | Description      |
|--------|-------------|:----------------------:|------------------|
| `0`    | `ID`        | `<0x01..0xFF>`         | Packet ID        |
| `1`    | `CMD`       | `0x03`                 | Read ADC command |
| `2`    | `CMD_ARG_0` | `0x04`                 | ADC Onboard      |

### Packet TX (Response)

| ID        | STATUS    | VOLTAGE        | TEMPERATURE    |
|:---------:|:---------:|:--------------:|:--------------:|
| `byte[0]` | `byte[1]` | `byte[2..3]`   | `byte[4..5]`   |
| `<0xFF>`  | `0x00`    | `0xFFFF`       | `0xFFFF`       |

---

# ⚙️ Motor Control (TB6612FNG)

## Driver Power Control

### Packet RX (Request)

| Byte   | Field       | Value                  | Description          |
|--------|-------------|:----------------------:|----------------------|
| `0`    | `ID`        | `<0x01..0xFF>`         | Packet ID            |
| `1`    | `CMD`       | `0x04`                 | Motor command        |
| `2`    | `CMD_ARG_0` | `0x01`                 | Driver power control |
| `3`    | `CMD_ARG_1` | `0x00`                 | Driver power OFF     |
|        |             | `0x01`                 | Driver power ON      |

### Packet TX (Response)

| ID        | STATUS    | POWER          |
|:---------:|:---------:|:--------------:|
| `byte[0]` | `byte[1]` | `byte[2]`      |
| `<0xFF>`  | `0x00`    | `<0x00\|0x01>` |

| POWER  | DESCRIPTION |
|:------:|-------------|
| `0x00` | OFF         |
| `0x01` | ON          |

## Motor Control

### Packet RX (Request)

| Byte   | Field       | Value                  | Description       |
|--------|-------------|:----------------------:|-------------------|
| `0`    | `ID`        | `<0x01..0xFF>`         | Packet ID         |
| `1`    | `CMD`       | `0x04`                 | Motor command     |
| `2`    | `CMD_ARG_0` | `0x02`                 | Set motor state   |
| `3`    | `CMD_ARG_1` | `0x01`                 | MOTOR_1           |
|        |             | `0x02`                 | MOTOR_2           |
| `4`    | `CMD_ARG_2` | `0x00`                 | Direction Free    |
|        |             | `0x01`                 | Direction Forward |
|        |             | `0x02`                 | Direction Reverse |
|        |             | `0x03`                 | Direction Brake   |
| `5`    | `CMD_ARG_3` | `<0..100>`             | Speed             |

### Packet TX (Response)

| ID        | STATUS    | MOTOR          | DIRECTION                  | SPEED      |
|:---------:|:---------:|:--------------:|:--------------------------:|:----------:|
| `byte[0]` | `byte[1]` | `byte[2]`      | `byte[3]`                  | `byte[4]`  |
| `<0xFF>`  | `0x00`    | `<0x00\|0x01>` | `<0x00\|0x01\|0x02\|0x03>` | `<0..100>` |

| DIRECTION | DESCRIPTION |
|:---------:|:------------|
| `0x00`    | Free        |
| `0x01`    | Forward     |
| `0x02`    | Reverse     |
| `0x03`    | Brake       |

## Get Motor Driver Status

### Packet RX (Request)

| Byte   | Field       | Value                  | Description      |
|--------|-------------|:----------------------:|------------------|
| `0`    | `ID`        | `<0x01..0xFF>`         | Packet ID        |
| `1`    | `CMD`       | `0x04`                 | Motor command    |
| `2`    | `CMD_ARG_0` | `0xFF`                 | Get motor status |

### Packet TX (Response)

| ID        | STATUS    | DRIVER_STATE   | MOTOR_1_DIRECTION          | MOTOR_1_SPEED | MOTOR_2_DIRECTION          | MOTOR_2_SPEED |
|:---------:|:---------:|:--------------:|:--------------------------:|:-------------:|:--------------------------:|:-------------:|
| `byte[0]` | `byte[1]` | `byte[2]`      | `byte[3]`                  | `byte[4]`     | `byte[5]`                  | `byte[6]`     |
| `<0xFF>`  | `0x00`    | `<0x00\|0x01>` | `<0x00\|0x01\|0x02\|0x03>` | `<0..100>`    | `<0x00\|0x01\|0x02\|0x03>` | `<0..100>`    |

| DRIVER_STATE  | DESCRIPTION |
|--------|-------------|
| `0x00` | Driver OFF  |
| `0x01` | Driver ON   |

| MOTOR_N_DIRECTION | DESCRIPTION |
|-------------------|-------------|
| `0x00`            | Free        |
| `0x01`            | Forward     |
| `0x02`            | Reverse     |
| `0x03`            | Brake       |

---