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
0x01 (`NMGR_GROUP_ID_IMAGE`), and was generated with `$ newtmgr -l DEBUG -c serial image list`:

> See [imagelist.go](https://github.com/apache/incubator-mynewt-newt/blob/master/newtmgr/protocol/imagelist.go)
for a full list of commands in the IMAGE `Group`.

```
$ newtmgr -l DEBUG -c serial image list
2016/11/11 12:25:51 [DEBUG] Writing newtmgr request &{Op:0 Flags:0 Len:0 Group:1 Seq:0 Id:0 Data:[]}
2016/11/11 12:25:51 [DEBUG] Serializing request &{Op:0 Flags:0 Len:0 Group:1 Seq:0 Id:0 Data:[]} into buffer [0 0 0 0 0 1 0 0]
2016/11/11 12:25:51 [DEBUG] Tx packet dump:
00000000  00 00 00 00 00 01 00 00                           |........|

2016/11/11 12:25:51 [DEBUG] Writing [6 9] to data channel
2016/11/11 12:25:51 [DEBUG] Writing [65 65 111 65 65 65 65 65 65 65 69 65 65 68 99 119] to data channel
2016/11/11 12:25:51 [DEBUG] Writing [10] to data channel
2016/11/11 12:25:51 [DEBUG] Reading [6 9 65 65 111 65 65 65 65 65 65 65 69 65 65 68 99 119] from data channel
2016/11/11 12:25:51 [DEBUG] Rx packet dump:
00000000  00 00 00 00 00 01 00 00                           |........|

2016/11/11 12:25:51 [DEBUG] Deserialized response &{Op:0 Flags:0 Len:0 Group:1 Seq:0 Id:0 Data:[]}
2016/11/11 12:25:51 [DEBUG] Reading [13] from data channel
2016/11/11 12:25:51 [DEBUG] Reading [6 9 65 73 85 66 65 81 66 55 65 65 69 65 65 76 57 109 97 87 49 104 90 50 86 122 110 55 57 107 99 50 120 118 100 65 66 110 100 109 86 121 99 50 108 118 98 109 85 119 76 106 77 117 77 71 82 111 89 88 78 111 87 67 68 83 84 76 77 70 69 49 81 88 75 55 85 81 110 53 121 48 114 110 104 104 50 87 49 113 47 102 120 71 50 48 103 115 54 121 48 48 113 75 101 79 48 71 104 105 98 50 57 48 89 87 74 115 90 102 86 110 99 71 86 117 90 71 108 117] from data channel
2016/11/11 12:25:51 [DEBUG] Reading [4 20 90 47 82 112 89 50 57 117 90 109 108 121 98 87 86 107 57 87 90 104 89 51 82 112 100 109 88 49 47 47 57 114 99 51 66 115 97 88 82 84 100 71 70 48 100 88 77 65 47 49 78 116] from data channel
2016/11/11 12:25:51 [DEBUG] Rx packet dump:
00000000  01 01 00 7b 00 01 00 00  bf 66 69 6d 61 67 65 73  |...{.....fimages|
00000010  9f bf 64 73 6c 6f 74 00  67 76 65 72 73 69 6f 6e  |..dslot.gversion|
00000020  65 30 2e 33 2e 30 64 68  61 73 68 58 20 d2 4c b3  |e0.3.0dhashX .L.|
00000030  05 13 54 17 2b b5 10 9f  9c b4 ae 78 61 d9 6d 6a  |..T.+......xa.mj|
00000040  fd fc 46 db 48 2c eb 2d  34 a8 a7 8e d0 68 62 6f  |..F.H,.-4....hbo|
00000050  6f 74 61 62 6c 65 f5 67  70 65 6e 64 69 6e 67 f4  |otable.gpending.|
00000060  69 63 6f 6e 66 69 72 6d  65 64 f5 66 61 63 74 69  |iconfirmed.facti|
00000070  76 65 f5 ff ff 6b 73 70  6c 69 74 53 74 61 74 75  |ve...ksplitStatu|
00000080  73 00 ff                                          |s..|

2016/11/11 12:25:51 [DEBUG] Deserialized response &{Op:1 Flags:1 Len:123 Group:1 Seq:0 Id:0 Data:[191 102 105 109 97 103 101 115 159 191 100 115 108 111 116 0 103 118 101 114 115 105 111 110 101 48 46 51 46 48 100 104 97 115 104 88 32 210 76 179 5 19 84 23 43 181 16 159 156 180 174 120 97 217 109 106 253 252 70 219 72 44 235 45 52 168 167 142 208 104 98 111 111 116 97 98 108 101 245 103 112 101 110 100 105 110 103 244 105 99 111 110 102 105 114 109 101 100 245 102 97 99 116 105 118 101 245 255 255 107 115 112 108 105 116 83 116 97 116 117 115 0 255]}
Images:
 slot=0
    version: 0.3.0
    bootable: true
    flags: active confirmed
    hash: d24cb3051354172bb5109f9cb4ae7861d96d6afdfc46db482ceb2d34a8a78ed0
Split status: N/A
```

When serialised this will be sent as `0x00 0x00 0x00 0x00 0x00 0x01 0x00 0x00`.

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
