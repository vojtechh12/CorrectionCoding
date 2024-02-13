#include <stdio.h>

// generator matrix G
char mxG[4] = {0x45, 0x27, 0x16, 0x0B};

// transposed parity check matrix H^T
char mxHt[7] = {0x05, 0x07, 0x06, 0x03, 0x04, 0x02, 0x01};

char syndrome(char x) {
    // hamming code syndrome calculation
	// TODO check with printf on PC
    char tmp;
    char state = 0x00;
    for (int i=0; i<7; i++) {
        if (((x>>(6-i)) & 0x01) == 0) {         // masking columns of parity check matrix
            continue;
        }
        tmp = mxHt[i];
        if (state != 0x00){
            tmp ^= state;
        }
        state = tmp;
    }

    return tmp;
}

int main() {



    char b = 0x0A;      // dataword (I am only interested in lower nibble, i.e. 1010)

    // lets try in cycle for one whole dataword b
    char tmp;
    char state = 0x00;
    for (int i=0; i<4; i++) {
        if (((b>>(3-i)) & 0x01) == 0) {
            printf("FLAG raised for i=%d\n",i);
            continue;

        }

        tmp = mxG[i];
        if (state != 0x00){
            tmp ^= state;
        }
        state = tmp;
        // printf("i=%d, tmp=%x, state=%x\n", i,tmp,state);
    }
    // add some logic for null b word
    printf("The resulting codeword: %x\n", tmp);

    // ------------------------------------------
    // decoding, syndrome calculation, error correction

    //testing distinct syndrome values for bit flips on each position, assume sent data 0x53
    char x = 0x13; // error on bit n 6 ||| (0 .. 6)
    printf("data b4 error correction: %x\n", x);

    char s = 0x00;
    s = syndrome(x);

    printf("Syndrome for changed bit nX: %x\n", s);

    // error correction
    switch (s)
    {
    case 0x00:
        // errorless transmission
        break;
    case 0x01:
        x ^= 0x01;
        break;
    case 0x02:
        x ^= 0x02;
        break;
    case 0x04:
        x ^= 0x04;
        break;
    case 0x03:  // error on bit n3
        x ^= 0x08;
        break;
    case 0x06:  // error on bit n4
        x ^= 0x10;
        break;
    case 0x07:  // error on bit n5
        x ^= 0x20;
        break;
    case 0x05:  // error on bit n6
        x ^= 0x40;
        break;  
    default:
        // throw flag for retransmission (send NACK)
        break;
    }

    printf("data after error correction: %x\n", x);





    return 0;
}