/*
    @Author/Owner: Alessandro Lotto
    @Last update: 25/02/2023
    @Note: This code has been build completely from scratch

    @Description:
        This file defines the primitives for implementing the crypto operations for the simulation.
        Only AES-128 is implemented so far.
        The implementation is not optimized.
*/

#ifndef CRYPTO_H
#define CRYPTO_H

#include <stdio.h>
#include <stdlib.h>

using namespace std;

class crypto{
    public:
        unsigned char* AES128_encryption(char* message, int msg_length, char* key); //implementation of AES-128 encryption
        unsigned char* AES128_decryption(char* cipher, char* key);    //implementation of AES-128 decryption
    
    private:
        
        //AES-128 UTILITY FUNCTIONS
        unsigned char** expansion(unsigned char* starting_key);

        void enc_substitution(unsigned char matrix[][4]);
        void enc_shiftRows(unsigned char matrix[][4]);
        void enc_mixColumns(unsigned char matrix[][4]);
        void pi_s(unsigned char* a);

        void dec_substitution(unsigned char matrix[][4]);
        void dec_shiftRows(unsigned char matrix[][4]);
        void dec_mixColumns(unsigned char matrix[][4]);
        void pi_s_inv(unsigned char* a);
        unsigned char find_address(unsigned char a);

        void matrix_transform(unsigned char* message, unsigned char matrix[][4]);   //transform a linear number into a matricial representation according to the AES format
        void linear_transform(unsigned char matrix[][4], unsigned char* message);    //transform a matrix represented state into its linear representation        
        
        unsigned char* AES128_padding(unsigned char* message, int message_length);
};

//-------------------------------------- AES128 --------------------------------------------------------

/**
 * @brief Add the front 0-padding to the input message so to obtain a final message of 128 bits
 * 
 * @param message: pointer to the input message
 * @param msg_length: number of bytes of @param message
 * 
 * @return char*: pointer to the padded message 
 */

unsigned char* AES128_padding(unsigned char* message, int msg_length){

    unsigned char* padded_msg= (unsigned char*) calloc(16, 1);
    if(padded_msg == NULL)  exit(1);

    for(int i=0; i<msg_length; ++i) padded_msg[15-i]= message[msg_length-1-i];

    return padded_msg;
}

/**
 * @brief Return a 4x4 matrix populated from the input value as the status matrix of the AES algorithm
 * 
 * @param message: message to parse into a 4x4 matrix
 * 
 * @return unsigned char[][]: 4x4 matrix object 
 */

void matrix_transform(unsigned char* message, unsigned char matrix[][4]){

    for(int i=0; i<4; ++i){ //scan columns
        for(int j=0; j<4; ++j){ //scan rows
            matrix[j][i]= message[4*i+j];
        }
    }
}

/**
 * @brief Return a myInt object that is represented by the matrix state given in input
 * 
 * @param matrix: 4x4 matrix state to linearize
 * @param message: pointer to the object that will be populated with @param matrix
 */

void linear_transform(unsigned char matrix[][4], unsigned char* message){

    for(int i=0; i<4; ++i){ //scan the columns
        for(int j=0; j<4; ++j){ //scan the rows

            message[4*i+j]= matrix[j][i];
        }
    }
}

/**
 * @brief Implements the pi_s transformation for the substitution function
 * 
 * @param a: pointer to the row of the status matrix
 */

void pi_s(unsigned char* a){

    //substitution matrix definition
    unsigned char sub_matrix[16][16]={
        {0x63, 0x7C, 0x77, 0x7B, 0xF2, 0x6B, 0x6F, 0xC5, 0x30, 0x01, 0x67, 0x2B, 0xFE, 0xD7, 0xAB, 0x76},
        {0xCA, 0x82, 0xC9, 0x7D, 0xFA, 0x59, 0x47, 0xF0, 0xAD, 0xD4, 0xA2, 0xAF, 0x9C, 0xA4, 0x72, 0xC0},
        {0xB7, 0xFD, 0x93, 0x26, 0x36, 0x3F, 0xF7, 0xCC, 0x34, 0xA5, 0xE5, 0xF1, 0x71, 0xD8, 0x31, 0x15},
        {0x04, 0xC7, 0x23, 0xC3, 0x18, 0x96, 0x05, 0x9A, 0x07, 0x12, 0x80, 0xE2, 0xEB, 0x27, 0xB2, 0x75},
        {0x09, 0x83, 0x2C, 0x1A, 0x1B, 0x6E, 0x5A, 0xA0, 0x52, 0x3B, 0xD6, 0xB3, 0x29, 0xE3, 0x2F, 0x84},
        {0x53, 0xD1, 0x00, 0xED, 0x20, 0xFC, 0xB1, 0x5B, 0x6A, 0xCB, 0xBE, 0x39, 0x4A, 0x4C, 0x58, 0xCF},
        {0xD0, 0xEF, 0xAA, 0xFB, 0x43, 0x4D, 0x33, 0x85, 0x45, 0xF9, 0x02, 0x7F, 0x50, 0x3C, 0x9F, 0xA8},
        {0x51, 0xA3, 0x40, 0x8F, 0x92, 0x9D, 0x38, 0xF5, 0xBC, 0xB6, 0xDA, 0x21, 0x10, 0xFF, 0xF3, 0xD2},
        {0xCD, 0x0C, 0x13, 0xEC, 0x5F, 0x97, 0x44, 0x17, 0xC4, 0xA7, 0x7E, 0x3D, 0x64, 0x5D, 0x19, 0x73},
        {0x60, 0x81, 0x4F, 0xDC, 0x22, 0x2A, 0x90, 0x88, 0x46, 0xEE, 0xB8, 0x14, 0xDE, 0x5E, 0x0B, 0xDB},
        {0xE0, 0x32, 0x3A, 0x0A, 0x49, 0x06, 0x24, 0x5C, 0xC2, 0xD3, 0xAC, 0x62, 0x91, 0x95, 0xE4, 0x79},
        {0xE7, 0xC8, 0x37, 0x6D, 0x8D, 0xD5, 0x4E, 0xA9, 0x6C, 0x56, 0xF4, 0xEA, 0x65, 0x7A, 0xAE, 0x08},
        {0xBA, 0x78, 0x25, 0x2E, 0x1C, 0xA6, 0xB4, 0xC6, 0xE8, 0xDD, 0x74, 0x1F, 0x4B, 0xBD, 0x8B, 0x8A},
        {0x70, 0x3E, 0xB5, 0x66, 0x48, 0x03, 0xF6, 0x0E, 0x61, 0x35, 0x57, 0xB9, 0x86, 0xC1, 0x1D, 0x9E},
        {0xE1, 0xF8, 0x98, 0x11, 0x69, 0xD9, 0x8E, 0x94, 0x9B, 0x1E, 0x87, 0xE9, 0xCE, 0x55, 0x28, 0xDF},
        {0x8C, 0xA1, 0x89, 0x0D, 0xBF, 0xE6, 0x42, 0x68, 0x41, 0x99, 0x2D, 0x0F, 0xB0, 0x54, 0xBB, 0x16}
    };

    for(int j=0; j<4; ++j){

        unsigned char row_index= a[j]>>4;   //extract the first four bits indicating the rows

        unsigned char temp= (a[j]<<4);  //exctract the last four bits indicating the columns
        unsigned char column_index=  temp>>4;   //need to move back to correct the position

        a[j]= sub_matrix[row_index][column_index];  //changing value according to the substitution rule
    }
}

/**
 * @brief Implements the Substitution function of the AES algorithm: each element of the matrix represent an element of the substitution matrix. Then substitute with that element 
 * 
 * @param matrix: state matrix of AES
 */

void enc_substitution(unsigned char matrix[][4]){

    for(int i=0; i<4; ++i) //scan the rows
        pi_s(matrix[i]);    //apply the substitution transformation to the vector (row)
}

/**
 * @brief Implements the Shift Row function of the AES algorithm: rows are shifted of 1,2,3 position starting from the second row. The first row is not touched.
 * 
 * @param matrix: state matrix to which apply the function
 * 
 */

void enc_shiftRows(unsigned char matrix[][4]){

    for(int i=1; i<4; ++i){ //scan the rows

        unsigned char copy[4]= {matrix[i][0], matrix[i][1], matrix[i][2], matrix[i][3]}; //need to copy the row to perform the shift
        for(int j=0; j<4; ++j)  matrix[i][j]= copy[(j+i)%4];
    }
}

/**
 * @brief Implements the Mix Columns function of the AES algorithm.
 * 
 * @param matrix: state to which apply the transformation
 */

void enc_mixColumns(unsigned char matrix[][4]){

    unsigned char pi_matrix[4][4]= {
        {0x02, 0x03, 0x01, 0x01},
        {0x01, 0x02, 0x03, 0x01},
        {0x01, 0x01, 0x02, 0x03},
        {0x03, 0x01, 0x01, 0x02}
    };

    for(int i=0; i<4; ++i){ //scan the columns of the state matrix
        
        unsigned char copy[4]= {matrix[0][i], matrix[1][i], matrix[2][i], matrix[3][i]};    //need to copy the columns to perform the multiplication with the matrix

        //multiplication with the transformation matrix
        for(int j=0; j<4; ++j){ //scan the rows of pi_matrix
            unsigned char sum= 0x0;

            for(int k=0; k<4; ++k){ //scan the columns of pi_matrix
                bool need_XOR= (copy[k] >= 128)?    1 : 0;  //verify the is the most significant bit is 1, so we will need to perform the XOR operation

                if(pi_matrix[j][k]== 0x01)  sum^= copy[k];
                else if(pi_matrix[j][k]== 0x02){
                    unsigned char temp= copy[k]<<1;
                    temp^= 0x1b * need_XOR;
                    sum^= temp;
                }else{
                    unsigned char temp= copy[k]<<1;
                    temp^= 0x1b * need_XOR;
                    temp^= copy[k];
                    sum^= temp;
                }
            }

            matrix[j][i]= sum;  //store the value in the state matrix
        }        
    }
}

/**
 * @brief Implements the round keys generation function of the AES algorithm
 * 
 * @param starting_key: pointer to the starting key from which computing the round keys
 * 
 * @return vector <char*>: set of 10 round keys
 */

unsigned char** expansion(unsigned char* starting_key){

    unsigned char C[10][4]= {
        {0x01, 0x00, 0x00, 0x00},
        {0x02, 0x00, 0x00, 0x00},
        {0x04, 0x00, 0x00, 0x00},
        {0x08, 0x00, 0x00, 0x00},
        {0x10, 0x00, 0x00, 0x00},
        {0x20, 0x00, 0x00, 0x00},
        {0x40, 0x00, 0x00, 0x00},
        {0x80, 0x00, 0x00, 0x00},
        {0x1b, 0x00, 0x00, 0x00},
        {0x36, 0x00, 0x00, 0x00}
    };

    unsigned char** roundKeys;     //vector containing the round keys
    roundKeys=(unsigned char **)malloc(11*16);

    for(int i=0; i<16; i++) roundKeys[0][i]= starting_key[i];   //insert the starting key
    for(int i=1; i<11; ++i){    
        for(int j=0; j<16; j++) roundKeys[i][j]= 0x00;  //initialization to 0 for all the others
    }    
    
    for(int j=1; j<=10; ++j){   //execution rounds

        //splitting the previous key so to apply the transformation to each block easily
        unsigned char block_previous[4][4];
        for(int i=0; i<4; ++i){
            for(int k=0; k<4; ++k){
                block_previous[i][k]= roundKeys[j-1][(4*i)+k];
            }
        }
        
        //BLOCK 0

        // 1) circular 8-bit shift
        unsigned char circ_shifted_block3[4];
        for(int i=0; i<3; ++i)  circ_shifted_block3[i]= block_previous[3][i+1];
        circ_shifted_block3[3]= block_previous[3][0];

        // 2) apply the pi_s transformation
        pi_s(circ_shifted_block3);

        // 3) XOR operations
        for(int i=0; i<4; ++i)  roundKeys[j][i]= block_previous[0][i] ^ circ_shifted_block3[i] ^ C[j][i];

        
        //BLOCK 1
        for(int i=0; i<4; ++i)  roundKeys[j][4+i]= block_previous[1][i] ^ roundKeys[j][i];

        
        //BLOCK 2
        for(int i=0; i<4; ++i)  roundKeys[j][8+i]= block_previous[2][i] ^ roundKeys[j][4+i];


        //BLOCK 3
        for(int i=0; i<4; ++i)  roundKeys[j][12+i]= block_previous[3][i] ^ roundKeys[j][8+i];

    }

    return (unsigned char**) roundKeys;
}

/**
 * @brief Build up the corresponding element containing the indices of the given element in the substitution matrix
 * 
 * @param a: element to which loof for the indeces
 * 
 * @return unsigned char: corresponding element
 */

unsigned char find_address(unsigned char a){

    unsigned char sub_matrix[16][16]={
        {0x63, 0x7C, 0x77, 0x7B, 0xF2, 0x6B, 0x6F, 0xC5, 0x30, 0x01, 0x67, 0x2B, 0xFE, 0xD7, 0xAB, 0x76},
        {0xCA, 0x82, 0xC9, 0x7D, 0xFA, 0x59, 0x47, 0xF0, 0xAD, 0xD4, 0xA2, 0xAF, 0x9C, 0xA4, 0x72, 0xC0},
        {0xB7, 0xFD, 0x93, 0x26, 0x36, 0x3F, 0xF7, 0xCC, 0x34, 0xA5, 0xE5, 0xF1, 0x71, 0xD8, 0x31, 0x15},
        {0x04, 0xC7, 0x23, 0xC3, 0x18, 0x96, 0x05, 0x9A, 0x07, 0x12, 0x80, 0xE2, 0xEB, 0x27, 0xB2, 0x75},
        {0x09, 0x83, 0x2C, 0x1A, 0x1B, 0x6E, 0x5A, 0xA0, 0x52, 0x3B, 0xD6, 0xB3, 0x29, 0xE3, 0x2F, 0x84},
        {0x53, 0xD1, 0x00, 0xED, 0x20, 0xFC, 0xB1, 0x5B, 0x6A, 0xCB, 0xBE, 0x39, 0x4A, 0x4C, 0x58, 0xCF},
        {0xD0, 0xEF, 0xAA, 0xFB, 0x43, 0x4D, 0x33, 0x85, 0x45, 0xF9, 0x02, 0x7F, 0x50, 0x3C, 0x9F, 0xA8},
        {0x51, 0xA3, 0x40, 0x8F, 0x92, 0x9D, 0x38, 0xF5, 0xBC, 0xB6, 0xDA, 0x21, 0x10, 0xFF, 0xF3, 0xD2},
        {0xCD, 0x0C, 0x13, 0xEC, 0x5F, 0x97, 0x44, 0x17, 0xC4, 0xA7, 0x7E, 0x3D, 0x64, 0x5D, 0x19, 0x73},
        {0x60, 0x81, 0x4F, 0xDC, 0x22, 0x2A, 0x90, 0x88, 0x46, 0xEE, 0xB8, 0x14, 0xDE, 0x5E, 0x0B, 0xDB},
        {0xE0, 0x32, 0x3A, 0x0A, 0x49, 0x06, 0x24, 0x5C, 0xC2, 0xD3, 0xAC, 0x62, 0x91, 0x95, 0xE4, 0x79},
        {0xE7, 0xC8, 0x37, 0x6D, 0x8D, 0xD5, 0x4E, 0xA9, 0x6C, 0x56, 0xF4, 0xEA, 0x65, 0x7A, 0xAE, 0x08},
        {0xBA, 0x78, 0x25, 0x2E, 0x1C, 0xA6, 0xB4, 0xC6, 0xE8, 0xDD, 0x74, 0x1F, 0x4B, 0xBD, 0x8B, 0x8A},
        {0x70, 0x3E, 0xB5, 0x66, 0x48, 0x03, 0xF6, 0x0E, 0x61, 0x35, 0x57, 0xB9, 0x86, 0xC1, 0x1D, 0x9E},
        {0xE1, 0xF8, 0x98, 0x11, 0x69, 0xD9, 0x8E, 0x94, 0x9B, 0x1E, 0x87, 0xE9, 0xCE, 0x55, 0x28, 0xDF},
        {0x8C, 0xA1, 0x89, 0x0D, 0xBF, 0xE6, 0x42, 0x68, 0x41, 0x99, 0x2D, 0x0F, 0xB0, 0x54, 0xBB, 0x16}
    };

    for(int i=0; i<16; ++i){    //scan the rows
        for(int j=0; j<16; ++j){    //scan the columns
            
            if(a==sub_matrix[i][j]){    //if element found, then need to reconstruct the new element starting from the indeces of its position

                unsigned char new_elem= 0;

                unsigned char* p= (unsigned char*) &i;  
                new_elem+= (*p)<<4; //store the row

                p= (unsigned char*) &j;
                new_elem+= *p;

                return new_elem;
            }
        }
    }
}

/**
 * @brief Inverse function of the pi_s tranformation
 * 
 * @param a: rows to which apply the transformation 
 */

void pi_s_inv(unsigned char* a){    

    for(int i=0; i<4; ++i){ //for each element
        char new_elem= find_address(a[i]);
        a[i]= new_elem;
    }
}

/**
 * @brief Inverse function of the substitution transformation
 * 
 * @param matrix matrix state to which apply the function
 *  
 */

void dec_substitution(unsigned char matrix[][4]){

    for(int i=0; i<4; ++i) //scan the rows
        pi_s_inv(matrix[i]);    //apply the substitution transformation to the vector (row)
}

/**
 * @brief Implements the inverse of the shift rows transformation
 * 
 * @param matrix: matrix state to which apply the function
 */

void dec_shiftRows(unsigned char matrix[][4]){

    for(int i=1; i<4; ++i){ //scan the rows

        unsigned char copy[4]= {matrix[i][0], matrix[i][1], matrix[i][2], matrix[i][3]}; //need to copy the row to perform the shift
        for(int j=0; j<4; ++j)  matrix[i][j]= copy[(j+4-i)%4];
    }
}

/**
 * @brief Implements the inverse of the mix columns transformation
 * 
 * @param matrix: matrix state to which apply the function
 */

void dec_mixColumns(unsigned char matrix[][4]){

    unsigned char pi_matrix[4][4]= {
        {0x0e, 0x0b, 0x0d, 0x09},
        {0x09, 0x0e, 0x0b, 0x0d},
        {0x0d, 0x09, 0x0e, 0x0b},
        {0x0b, 0x0d, 0x09, 0x0E}
    };

    for(int i=0; i<4; ++i){ //scan the columns of the state matrix
        
        unsigned char copy[4]= {matrix[0][i], matrix[1][i], matrix[2][i], matrix[3][i]};    //need to copy the columns to perform the multiplication with the matrix

        //multiplication with the transformation matrix
        for(int j=0; j<4; ++j){ //scan the rows of pi_matrix
            unsigned char sum= 0x0;

            for(int k=0; k<4; ++k){ //scan the columns of pi_matrix
                bool need_XOR= (copy[k] >= 128)?    1 : 0;  //verify the is the most significant bit is 1, so we will need to perform the XOR operation

                if(pi_matrix[j][k]== 0x09){
                    //x2
                    unsigned char temp= copy[k]<<1;
                    temp^= 0x1b * need_XOR;

                    //x2
                    need_XOR= (temp >= 128)?    1 : 0;
                    temp<<= 1;
                    temp^= 0x1b * need_XOR;

                    //x2
                    need_XOR= (temp >= 128)?    1 : 0;
                    temp<<= 1;
                    temp^= 0x1b * need_XOR;

                    //+X
                    temp^= copy[k];

                    sum^= temp; //accumulation

                }else if(pi_matrix[j][k]== 0x0b){
                    //x2
                    unsigned char temp= copy[k]<<1;
                    temp^= 0x1b * need_XOR;

                    //x2
                    need_XOR= (temp >= 128)?    1 : 0;
                    temp<<= 1;
                    temp^= 0x1b * need_XOR;

                    //+X
                    temp^= copy[k];

                    //x2
                    need_XOR= (temp >= 128)?    1 : 0;
                    temp<<= 1;
                    temp^= 0x1b * need_XOR;

                    //+X
                    temp^= copy[k];

                    sum^= temp; //accumulation

                }else if(pi_matrix[j][k]== 0x0d){
                    //x2
                    unsigned char temp= copy[k]<<1;
                    temp^= 0x1b * need_XOR;

                    //+X
                    temp^= copy[k];

                    //x2
                    need_XOR= (temp >= 128)?    1 : 0;
                    temp<<= 1;
                    temp^= 0x1b * need_XOR;

                    //x2
                    need_XOR= (temp >= 128)?    1 : 0;
                    temp<<= 1;
                    temp^= 0x1b * need_XOR;

                    //+X
                    temp^= copy[k];

                    sum^= temp; //accumullation

                }else{
                    //x2
                    unsigned char temp= copy[k]<<1;
                    temp^= 0x1b * need_XOR;

                    //+X
                    temp^= copy[k];

                    //x2
                    need_XOR= (temp >= 128)?    1 : 0;
                    temp<<= 1;
                    temp^= 0x1b * need_XOR;

                    //+X
                    temp^= copy[k];

                    //x2
                    need_XOR= (temp >= 128)?    1 : 0;
                    temp<<= 1;
                    temp^= 0x1b * need_XOR;

                    sum^= temp; //accumullation
                }
                
            }

            matrix[j][i]= sum;  //store the value in the state matrix            
        }        
    }
}

/**
 * @brief Implements the AES encryption algorithm with 128-bit key
 * 
 * @param message: pointer to the message to encrypt
 * @param msg_length: number of bytes of the message
 * @param key: key used for encryption
 * 
 *  @return char*: pointer to the encrypted message object
 */

unsigned char* AES128_encryption(unsigned char* message, int msg_length, unsigned char* key){

    unsigned char* msg= AES128_padding(message, msg_length); //uses calloc()

    //----------- ROUND KEYS GENERATION ----------------
    unsigned char C[11][4]= {
        {0x00, 0x00, 0x00, 0x00},
        {0x01, 0x00, 0x00, 0x00},
        {0x02, 0x00, 0x00, 0x00},
        {0x04, 0x00, 0x00, 0x00},
        {0x08, 0x00, 0x00, 0x00},
        {0x10, 0x00, 0x00, 0x00},
        {0x20, 0x00, 0x00, 0x00},
        {0x40, 0x00, 0x00, 0x00},
        {0x80, 0x00, 0x00, 0x00},
        {0x1b, 0x00, 0x00, 0x00},
        {0x36, 0x00, 0x00, 0x00}
    };

    unsigned char roundKeys[11][16];     //vector containing the round keys

    for(int i=0; i<16; i++) roundKeys[0][i]= key[i];   //insert the starting key
    for(int i=1; i<11; ++i){    
        for(int j=0; j<16; j++) roundKeys[i][j]= 0x00;  //initialization to 0 for all the others
    }


    for(int j=1; j<=10; ++j){   //execution rounds

        //splitting the previous key so to apply the transformation to each block easily
        unsigned char block_previous[4][4];
        for(int i=0; i<4; ++i){
            for(int k=0; k<4; ++k){
                block_previous[i][k]= roundKeys[j-1][(4*i)+k];
            }
        }
        
        //BLOCK 0

        // 1) circular 8-bit shift
        unsigned char circ_shifted_block3[4];
        for(int i=0; i<3; ++i)  circ_shifted_block3[i]= block_previous[3][i+1];
        circ_shifted_block3[3]= block_previous[3][0];

        // 2) apply the pi_s transformation
        pi_s(circ_shifted_block3);

        // 3) XOR operations
        for(int i=0; i<4; ++i)  roundKeys[j][i]= block_previous[0][i] ^ circ_shifted_block3[i] ^ C[j][i];

        for(int i=0; i<4; ++i)  roundKeys[j][4+i]= block_previous[1][i] ^ roundKeys[j][i];  //BLOCK 1
        for(int i=0; i<4; ++i)  roundKeys[j][8+i]= block_previous[2][i] ^ roundKeys[j][4+i];    //BLOCK 2        
        for(int i=0; i<4; ++i)  roundKeys[j][12+i]= block_previous[3][i] ^ roundKeys[j][8+i];   //BLOCK 3
    }
    //------------------------------------------------------

    int n_round= 10;
    unsigned char matrix[4][4]; //creation of the matrix state

    for(int i=0; i<16; i++) msg[i] = msg[i] ^ roundKeys[0][i];    //XOR with the round Key_0
    
    matrix_transform(msg, matrix); //transformation in matrix state    

    for(int i=1; i<=n_round; ++i){

        enc_substitution(matrix);
        enc_shiftRows(matrix);
        if(i<=9)    enc_mixColumns(matrix);

        linear_transform(matrix, msg); //linearize the container in order to perform the modulo sum with the round key
        for(int j=0; j<16; j++) msg[j] = msg[j] ^ roundKeys[i][j];    //XOR with the round Key_i
        matrix_transform(msg, matrix);
    }

    linear_transform(matrix, msg);  //linearize the encrypted message

    return msg;
}

/**
 * @brief 
 * 
 * @param cipher: pointer to the ciphertext to decode
 * @param key: decryption key
 * 
 * @return char*: pointer to the computed plaintext  
 */

unsigned char* AES128_decryption(unsigned char* cipher, unsigned char* key){

    //----------- ROUND KEYS GENERATION ----------------
    unsigned char C[11][4]= {
        {0x00, 0x00, 0x00, 0x00},
        {0x01, 0x00, 0x00, 0x00},
        {0x02, 0x00, 0x00, 0x00},
        {0x04, 0x00, 0x00, 0x00},
        {0x08, 0x00, 0x00, 0x00},
        {0x10, 0x00, 0x00, 0x00},
        {0x20, 0x00, 0x00, 0x00},
        {0x40, 0x00, 0x00, 0x00},
        {0x80, 0x00, 0x00, 0x00},
        {0x1b, 0x00, 0x00, 0x00},
        {0x36, 0x00, 0x00, 0x00}
    };

    unsigned char roundKeys[11][16];     //vector containing the round keys

    for(int i=0; i<16; i++) roundKeys[0][i]= key[i];   //insert the starting key
    for(int i=1; i<11; ++i){    
        for(int j=0; j<16; j++) roundKeys[i][j]= 0x00;  //initialization to 0 for all the others
    }
    
    for(int j=1; j<=10; ++j){   //execution rounds

        //splitting the previous key so to apply the transformation to each block easily
        unsigned char block_previous[4][4];
        for(int i=0; i<4; ++i){
            for(int k=0; k<4; ++k){
                block_previous[i][k]= roundKeys[j-1][(4*i)+k];
            }
        }
        
        //BLOCK 0

        // 1) circular 8-bit shift
        unsigned char circ_shifted_block3[4];
        for(int i=0; i<3; ++i)  circ_shifted_block3[i]= block_previous[3][i+1];
        circ_shifted_block3[3]= block_previous[3][0];

        // 2) apply the pi_s transformation
        pi_s(circ_shifted_block3);

        // 3) XOR operations
        for(int i=0; i<4; ++i)  roundKeys[j][i]= block_previous[0][i] ^ circ_shifted_block3[i] ^ C[j][i];

        for(int i=0; i<4; ++i)  roundKeys[j][4+i]= block_previous[1][i] ^ roundKeys[j][i];  //BLOCK 1
        for(int i=0; i<4; ++i)  roundKeys[j][8+i]= block_previous[2][i] ^ roundKeys[j][4+i];    //BLOCK 2        
        for(int i=0; i<4; ++i)  roundKeys[j][12+i]= block_previous[3][i] ^ roundKeys[j][8+i];   //BLOCK 3
    }
    //------------------------------------------------------

    int n_round= 10;

    unsigned char matrix[4][4]; //creation of the matrix state
    matrix_transform(cipher, matrix); //transformation of the linear cipher into the matrix state
    
    unsigned char* msg= (unsigned char*) calloc(128, 1);

    for(int i=1; i<=n_round; ++i){

        linear_transform(matrix, msg); //linearize the state matrix in order to perform the modulo sum with the round key
        for(int j=0; j<16; j++) msg[j] = msg[j] ^ roundKeys[n_round+1-i][j];    //XOR with the round Key_i
        matrix_transform(msg, matrix);

        if(i>1) dec_mixColumns(matrix);
        dec_shiftRows(matrix);
        dec_substitution(matrix);     
    }
        
    linear_transform(matrix, msg); //linearize the container in order to perform the modulo sum with the round key
    for(int j=0; j<16; j++) msg[j] = msg[j] ^ roundKeys[0][j];    //XOR with the round Key_i

    return msg;
}


#endif  /*CRYPTO_H*/