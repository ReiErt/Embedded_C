TASK 1: 
When a character is received on the UART RX signal line, an ISR interrupt is triggered. The received character is saved in a 32-byte ring buffer. 
Data is read from the 32 byte ring buffer.

TASK 2:
Implement SW flow control using XOn/XOff in both the send function and the receive Interrupt Service Routine. 
If the ring buffer is full, we block incoming data to read and free space on the buffer.
Build a hysteresis when sending XOff and XOn so that the frequency of XOff and XOn remains as low as possible.
