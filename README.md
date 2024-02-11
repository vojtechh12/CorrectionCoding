# Overview
- High level flowchart (tx - sdch - rx)
- hamming space block code overview
- matrices specification
- example of error correction

<img src="./images/CorrCod.png" alt="" border=3 width=700>
</img>


# Physical layer implementation
- tx and rx connected via UART
- UART sends 8 bits of data
- segmentation and concatenation
- implementation of matrix multiplication

# Future work
- use of additional bit for e.g. CRC in the future
