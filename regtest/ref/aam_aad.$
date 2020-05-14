! Test AAM and AAD opcode.

	_EXIT	= 1
.SECT .TEXT
start:
	AAM
	AAD
	XOR     AX, AX
	PUSH    AX
	PUSH	_EXIT
	SYS
.SECT .BSS
