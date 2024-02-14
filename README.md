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

**Systematic block codes** are block codes, where the dataword is a part of the codeword. The remaining part of codeword is called parity. Parity ensures redundancy and is exploited to detect and correct errors.

A generator matrix for systematic block code:

$$ \mathbf{G} = \begin{bmatrix}
\mathbf{I_\mathrm{N_b}} \\
\mathbf{P}
\end{bmatrix}$$

The codeword is then obtained like this:
$$\mathbf{c} = \mathbf{G}\mathbf{b}$$
Note that the first block of generator matrix is an identity matrix. The first $N_b$ positions of a codeword is the coded dataword.

**Hamming codes** are a subset of systematic block codes with the ability to correct all weight one errors. This project exploits Hamming (7,4) code, which takes 4-bit input dataword and encodes it with additional 3-bit parity to form a 7-bit dataword. These 7 bits are sent via UART from TX to RX.

The generator matrix of used Hamming code(7,4) looks like this:

```math
\mathbf{G} = 
\begin{bmatrix}
1 & 0 & 0 & 0 \\ 0 & 1 & 0 & 0 \\ 0 & 0 & 1 & 0 \\ 0 & 0 & 0 & 1 \\ 1 & 1 & 1 & 0 \\ 0 & 1 & 1 & 1 \\ 1 & 1 & 0 & 1 \end{bmatrix}
```

### 1.2 Error Correction in Systematic Block Codes
Error correction exploits parity check matrix $\mathbf{H}$.

$$\mathbf{H} = \begin{bmatrix}
-\mathbf{P} \\
\mathbf{I_\mathrm{N_c - N_b}}
\end{bmatrix}$$

Parity check matrix columns form the basis of codeword null-space, orthogonal to codeword sub-space.

$$\mathbf{H^\mathrm{T}} \mathbf{G} = \mathbf{O}$$

Let's assume AWGN channel. The received codeword $\mathbf{x}$ is:
$$\mathbf{x} = \mathbf{c} + \mathbf{w}$$

To determine, if a codeword was received correctly, a **syndrome** is calculated. Syndrome is a projection of the received codeword to the sub-space orthogonal to codeword sub-space.

*Remember, that the parity check matrix describes sub-space orthogonal to codeword-subspace. Therefore, if a projection of a received code to this space is non-zero, we know that the data must have been corrupted*

$$\mathbf{s} = \mathbf{H^\mathrm{T}} \mathbf{x}$$

If the syndrome is zero, we know that a valid codeword was received. One characteristic sign of Hamming code is that it has distinct syndromes for all weight one errors, enabling correction of such errors.

The parity check matrix for selected Hamming code(7,4) looks like this:

```math
\mathbf{H^\mathrm{T}} = 
\begin{bmatrix}
1 & 1 & 1 & 0 & 1 & 0 & 0\\ 0 & 1 & 1 & 1 & 0 & 1 & 0 \\ 1 & 1 & 0 & 1 & 0 & 0 & 1  \end{bmatrix}
```

### 1.3 Example Error Correction
This section examines an example of what is implemented in the code. Input data (1 Byte) is segmented into two nibbles. Each one is encoded with Hamming(7,4) code.

Lets assume an input:
```C
char message = '7'; //i.e. 0b00110111 in ASCII
```

This is, as mentioned, segmented into lower nibble and higher nibble. In the following, only lower nibble (`0x7`) encoding and decoding es shown. The 4-bit lower nibble is multiplied with the generator matrix to yield the codeword.

```math
\begin{bmatrix}
1 & 0 & 0 & 0 \\ 0 & 1 & 0 & 0 \\ 0 & 0 & 1 & 0 \\ 0 & 0 & 0 & 1 \\ 1 & 1 & 1 & 0 \\ 0 & 1 & 1 & 1 \\ 1 & 1 & 0 & 1 \end{bmatrix} \begin{bmatrix} 0 \\ 1 \\ 1 \\ 1 \end{bmatrix} = \begin{bmatrix}
0 \\ 1 \\ 1 \\ 1 \\ 0 \\ 1 \\0
\end{bmatrix}
```
Now let's consider an AWGN channel that toggle bit n4 (indexing from LSB with zero). Then the received bit will be:

```math
\mathbf{x} = \begin{bmatrix}
0\\1\\0\\1\\0\\1\\0
\end{bmatrix}
```

This yields syndrome:

```math
\mathbf{s} = \begin{bmatrix}
1\\1\\0
\end{bmatrix}
```

Upon checking with the parity check matrix $\mathbf{H^\mathrm{T}}$, this syndrome correcponds to its column n4, indicating an error on bit n4. This allows for correction of such error. Limitation of this code are weight one errors. If more than one bit was corrupted, the calculated syndrome would not have the ability to distinguish various other error patterns.

## 2. Physical layer implementation
This section intends to highlight a few key techniques used to implement the aforementioned theory.

### 2.1 Vector-Matrix Multiplication over Binary Data
Vector-Matrix multiplication is basically masking columns of matrix based on vector values (1 or 0) and then summing those columns to create a resulting vector. Summing over binary data is modulo 2 operation and is effectively implemented by XOR.

### 2.2 Sending Zero Char
Lower nibble of zero is `0x0`. This cannot form a valid codeword. Therefore, all bytes received from user are incremented by 1. Zero is then sent as 1 and so on. On receiving end, upon effectively decoding and concatenating sent byte, it is decremented by 1. This allows to encode and send zero.

```C
data_byte = getchar();
        data_byte +=1; // enable tx of zero character
```

- tx and rx connected via UART
- UART sends 8 bits of data
- segmentation and concatenation
- implementation of matrix multiplication over F body

## 3. Future work
- implement ARQ mechanism
- study linear albebra terminology to better grasp documentation


## 4. Video Demo
[youtube demo](https://youtube.com/shorts/7p8gvKg8kL0?feature=share)
