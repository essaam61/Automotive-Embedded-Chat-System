Designed and implemented a chat system that consists of Arm Cortex-M4 TivaC microcontroller, a PC, and some communication protocols; UART & CAN. The chat system is a communication system between 2 Tiva Cs defined in a form of states; Idle, Data Collection, Transmission, Reception & Presenting. Each state deternines what the microcontroller does and messages go back and forth between the two.

An Idle Tiva (not sending nor receiving) will have the blue LED on. Upon pressing button 1 on Tiva (A) the tiva will start accepting data (0-200 bytes) from the PC using UART. This is called the data collection state. Tiva (A) sends the data over CAN bus as message if we reached maximum limit (200 bytes) or if we pressed on button2. While sending the green LED will be on, sending will stop when the data ends. This is called the transmission state. In the reception state, Tiva (B) will start receiving once data is detected. Tiva (B) collects the data then prints them on PC using UART as well. This is called the presenting state (The data processing was expected to keep the same order). Recent Tiva B states will have the red LED on during both.

<p align="center">
   <img width="556" height="834" src="https://user-images.githubusercontent.com/109050863/197556011-d2d85e97-2db7-4c11-9de5-fe3f731dde37.PNG"> 
</p>
