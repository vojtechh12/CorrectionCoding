# Implementation of Correction Coding
- This project implements correction coding using Hamming code.
- 3 main components: transmitter (TX), receiver (RX), and Software Defined Channel (SDCH)
  - TX represented by PC is encoding user input message
  - SDCH, part of TX code, corrupts the data
  - RX reads the data and corrects it
  
---

<img src="./images/CorrCod.png" alt="" border=3 width=700>
</img>

Project overview diagram

---

<img src="./images/flow-diagram.png" alt="flow diagram" border=3 width=300>
</img>

Project flowchart

---

## 1. Theoretical background
- hamming space block code overview
- matrices specification
- encode and decode equations
- examine error correction using syndrome (simple proof)
## 2. Physical layer implementation
- tx and rx connected via UART
- UART sends 8 bits of data
- segmentation and concatenation
- implementation of matrix multiplication over F body

## 3. Future work
- use of additional bit for e.g. CRC in the future
