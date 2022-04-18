type TZV_CAN.hex | findstr /i /v ":00000001FF" > temp.hex

del TZV_CANcl.hex
(
type temp.hex
type canloader.hex
)>TZV_CANcl.hex

del temp.hex

type TZV_CAN.eep | findstr /i /v ":00000001FF" > temp.eep

del TZV_CANcl.eep
(
type temp.eep
type canloader.eep
)>TZV_CANcl.eep

del temp.eep

PAUSE

exit