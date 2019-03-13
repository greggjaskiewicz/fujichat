
 .ifdef FUJICHAT
START_ADDRESS = $9C01 ; subtract $0800 (2048) if using BASIC or other cart
;START_ADDRESS = $7A00
 .else
START_ADDRESS = $7A00
 .endif
