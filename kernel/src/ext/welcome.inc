ext_welcome:
 	uefi_call_wrapper ConOut, OutputString, ConOut, .msg.1
 	uefi_call_wrapper ConOut, SetAttribute, ConOut, 0xE
 	uefi_call_wrapper ConOut, OutputString, ConOut, .msg.2
 	uefi_call_wrapper ConOut, SetAttribute, ConOut, 7
 	uefi_call_wrapper ConOut, OutputString, ConOut, .msg.3
 	uefi_call_wrapper ConOut, OutputString, ConOut, .msg.4
	ret
.msg.1 du 'Welcome to ', 0
.msg.2 du 'InetOS', 0
.msg.3 du '! Enjoy yourself. ', 0xD, 0xA, 0
.msg.4 du 'Press keys to continue reducing the net.', 0xD, 0xA, 0