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
The main principle of coding is to gain the ability to detect and, under some conditions, even correct errors. This is achieved through redundancy. There are two ways to ensure redundancy. Alphabet expansion ($M_c$) and dimension expansion ($N_c$).
To achieve redundancy, the following must hold:

$M_c^{N_c L} > M_b^{N_b L}$

That is, the set of all possible sequences in code embedding space must be larger than the number of all possible messages (datawords).

### 1.1 Linear (Systematic) Block Codes
For linear codes, the input-output relation between dataword ($\mathbf{d}$) and codeword ($\mathbf{c}$) is linear one-to-one mapping.

Block codes segment the input data into fixed-length block and then encodes these blocks independently, i.e. without memory.

Systematic block codes are block codes, where the dataword is a part of the codeword. The remaining part of codeword is called parity. Parity ensures redundancy and is exploited to detect and correct errors.

A generator matrix for systematic block code:

$$ \mathbf{G} = \begin{bmatrix}
\mathbf{I_{N_b}} \\
\mathbf{P}
\end{bmatrix}$$

The codeword is then obtained like this:
$$\mathbf{c} = \mathbf{G}\mathbf{b}$$
Note that the first block of generator matrix is an identity matrix. The first $N_b$ positions of a codeword is the coded dataword.

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
