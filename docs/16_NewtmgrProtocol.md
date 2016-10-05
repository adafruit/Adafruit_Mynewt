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
