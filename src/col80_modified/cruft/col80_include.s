; ----------------------------------------------------------------------------
; Zero page labels (OS equates)

DOSINI          = $000C
ICAX1Z          = $002A
ICAX2Z          = $002B
TMPCHR          = $0050
LMARGN          = $0052
ROWCRS          = $0054
COLCRS          = $0055
DINDEX          = $0057
SAVMSC          = $0058
BUFCNT          = $006B

; ----------------------------------------------------------------------------
; Zero page labels (COL80 equates)

screen_ptr_lo   = $00CB
screen_ptr_hi   = $00CC
font_ptr_lo     = $00CD
font_ptr_hi     = $00CE

; ----------------------------------------------------------------------------
; Non-zeropage RAM labels (OS equates)

COLOR1          = $02C5
COLOR2          = $02C6
RUNAD           = $02E0
MEMTOP          = $02E5
SSFLAG          = $02FF
HATABS          = $031A
ICCOM           = $0342
ICBAL           = $0344
ICBAH           = $0345

; ----------------------------------------------------------------------------
; Hardware (memory-mapped I/O, OS equates)

CONSOL          = $D01F
AUDF1           = $D200
AUDC1           = $D201

; ----------------------------------------------------------------------------
; OS ROM labels

s_dev_open_lo   = $E410 ; (not named in OS sources)
s_dev_open_hi   = $E411 ; ""
k_dev_get_lo    = $E424 ; ""
k_dev_get_hi    = $E425 ; ""
CIOV            = $E456 ; Central Input/Output entry point
