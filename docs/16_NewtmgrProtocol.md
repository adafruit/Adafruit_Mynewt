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
- **`Len`**:  TBD (presumably the payload len when `Data` is present)
- **`Group`**: TBD
- **`Seq`**: TBD
- **`Id`**: The command ID to send
- **`Data`**: The payload associated with the command `Id` above

### Example Packets

The following example corresponds to the `taskstats` command in newtmgr, and
can be seen by running `newtmgr -l DEBUG -c serial taskstats`:

```
Op:    0  # NMGR_OP_READ
Flags: 0
Len:   0  # No payload present
Group: 0
Seq:   0
Id:    2  # Command ID = 0x02 ()
Data:  [] # No payload
```

When serialized this will be sent as `0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x02`.
