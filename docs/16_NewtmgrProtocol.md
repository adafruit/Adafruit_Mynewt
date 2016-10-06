# The Newtmgr protocol

The `newtmgr` tool uses a custom protocol to send commands and responses
between the manager and the end node using a variety of transports (currently
TTY serial or BLE).

The protocol isn't documented but the following information has been inferred
from the source code available on Github and using the `-l DEBUG` flag When
executing commands.

## Newtmgr Frame Format

ToDo

### Endianness

Frames are serialized as **Big Endian** when dealing with values > 8 bits.

### Frame format

Frames in newtmgr have the following format:

```
type NmgrReq struct {
	Op    uint8
	Flags uint8
	Len   uint16
	Group uint16
	Seq   uint8
	Id    uint8
	Data  []byte
}
```

`Op` can be one of the following values:

```
const (
	NMGR_OP_READ      = 0
	NMGR_OP_READ_RSP  = 1
	NMGR_OP_WRITE     = 2
	NMGR_OP_WRITE_RSP = 3
)
```

- **`op`**: The operation code
- **`Flags`**: TBD
- **`Len`**:  The payload len when `Data` is present
- **`Group`**: Commands are organized into groups. Groups are defined
  [here](https://github.com/apache/incubator-mynewt-newt/blob/master/newtmgr/protocol/defs.go).
- **`Seq`**: TBD
- **`Id`**: The command ID to send. Commands in the default `Group` are defined
  [here](https://github.com/apache/incubator-mynewt-newt/blob/master/newtmgr/protocol/defs.go).
- **`Data`**: The payload associated with the command `Id` above

### Example Packets

The following example commands show how the different fields work:

#### Simple Read Request: `taskstats`

The following example corresponds to the `taskstats` command in newtmgr, and
can be seen by running `newtmgr -l DEBUG -c serial taskstats`:

```
Op:    0  # NMGR_OP_READ
Flags: 0
Len:   0  # No payload present
Group: 0  # 0x00 = NMGR_GROUP_ID_DEFAULT
Seq:   0
Id:    2  # 0x02 in group 0x00 = NMGR_ID_TASKSTATS
Data:  [] # No payload (len = 0 above)
```

When serialized this will be sent as `0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x02`.

If this was sent using the serial port, you would get the following request and response:

> Debug output lines 2..5 can be ignored ... these are an artifact of the serial connection where the values need to be encoded to a limited hex range so that they values can be properly displayed in a terminal emulator, etc. No such restrictions exist when working with BLE, whether the full 8-bit range is valid and available.

```
$ newtmgr -l DEBUG -c serial taskstats
2016/10/05 14:30:43 [DEBUG] Writing netmgr request &{Op:0 Flags:0 Len:0 Group:0 Seq:0 Id:2 Data:[]}
2016/10/05 14:30:43 [DEBUG] Serializing request &{Op:0 Flags:0 Len:0 Group:0 Seq:0 Id:2 Data:[]} into buffer [0 0 0 0 0 0 0 2]
2016/10/05 14:30:43 [DEBUG] Writing [6 9] to data channel
2016/10/05 14:30:43 [DEBUG] Writing [65 65 111 65 65 65 65 65 65 65 65 65 65 105 66 67] to data channel
2016/10/05 14:30:43 [DEBUG] Writing [10] to data channel
2016/10/05 14:30:43 [DEBUG] before deserializing:
2016/10/05 14:30:43 [DEBUG] Deserialized response &{Op:0 Flags:0 Len:0 Group:0 Seq:0 Id:2 Data:[]}
2016/10/05 14:30:43 [DEBUG] before deserializing:{"rc": 0,"tasks": {"idle": {"prio": 255,"tid": 0,"state": 1,"stkuse": 26,"stksiz": 64,"cswcnt": 3255553,"runtime": 3118922,"last_checkin": 0,"next_checkin": 0},"shell": {"prio": 3,"tid": 1,"state": 2,"stkuse": 64,"stksiz": 384,"cswcnt": 367,"runtime": 2,"last_checkin": 0,"next_checkin": 0},"newtmgr": {"prio": 4,"tid": 2,"state": 1,"stkuse": 122,"stksiz": 512,"cswcnt": 9,"runtime": 10,"last_checkin": 0,"next_checkin": 0},"blinky": {"prio": 10,"tid": 3,"state": 2,"stkuse": 26,"stksiz": 128,"cswcnt": 3125,"runtime": 0,"last_checkin": 0,"next_checkin": 0},"bleuart_bridge": {"prio": 5,"tid": 4,"state": 1,"stkuse": 28,"stksiz": 256,"cswcnt": 3130609,"runtime": 0,"last_checkin": 0,"next_checkin": 0},"bleprph": {"prio": 1,"tid": 5,"state": 2,"stkuse": 226,"stksiz": 336,"cswcnt": 6283,"runtime": 0,"last_checkin": 0,"next_checkin": 0},"ble_ll": {"prio": 0,"tid": 6,"state": 2,"stkuse": 64,"stksiz": 80,"cswcnt": 145919,"runtime": 5868,"last_checkin": 0,"next_checkin": 0}}}
```

#### Group Read Request: `image list`

The following command lists images on the device and uses commands from `Group`
0x01 (`NMGR_GROUP_ID_IMAGE`), and was generated with `newtmgr -l DEBUG -c serial image list`:

> See [imagelist.go](https://github.com/apache/incubator-mynewt-newt/blob/master/newtmgr/protocol/imagelist.go)
for a full list of commands in the IMAGE `Group`.

```
Op:    0  # NMGR_OP_READ
Flags: 0
Len:   0  # No payload present
Group: 1  # 0x01 = NMGR_GROUP_ID_IMAGE
Seq:   0
Id:    0  # 0x00 in group 0x01 = IMGMGR_NMGR_OP_LIST
Data:  [] # No payload (len = 0 above)
```

## Transports

### Newtmgr Over Serial

`newtmgr` can be used over TTY serial with the following parameters:

- Baud Rate: 115200
- HW Flow Control: None

### Newtmgr Over BLE

`newtmgr` can be used over BLE with the following GATT service and
characteristic UUIDs.

- **Service UUID**: `8D53DC1D-1DB7-4CD3-868B-8A527460AA84`
- **Characteristic UUID**: `DA2E7828-FBCE-4E01-AE9E-261174997C48`

The  "newtmgr" service consists of one write no-rsp characteristic
for newtmgr requests: a single-byte characteristic that can only
accepts write-without-response commands.  The contents of each write
command contains an NMP request.  NMP responses are sent back in the
form of unsolicited notifications from the same characteristic.
