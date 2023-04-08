/*
    @Author/Owner: Alessandro Lotto
    @Last update: 25/02/2023
    @Note: This code has been build completely from scratch

    @Description:
        This file defines the AMF entity for the simulation and implement methods for its actions.
*/

#ifndef AMF_H
#define AMF_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "./message.cpp"
#include "./crypto.cpp"

const char HO_RQED[]= "Handover Required";
const char HO_ACK[]= "Handover ACK";
const char HO_REST[]= "Handover Request";
const char REC[]= "Reconnection Recovery";
const char RECON_OK[]= "Reconnection Recovery OK";
const char RECON_REJ[]= "Reconnection Recovery Rejected";

unsigned char UE_key[]= "abcdefghilmnopqr";

using namespace std;

class AMF{
    private:
        int AMF_id;
        int x_pos;  //x-axis coordinate position
        int y_pos;  //y-axis coordinate position

        int n_bs;
        int* BS_ids;    //pointer to an array of integer containing the IDs of the BSs under its control        
        
        //-> No use of the BARON authentication token (AT) because initial access is not yet implemented in the simulation
        int is_patched; //descriminates whether to apply the patched version
        int rec_token;  //used only in the patched version -> here I take it as int because the possible future comparison
                        //-> avoid future computation to transofrm it into a string

        int find_Tindex(int tID);   //find the index for the msg_channel of tID
        int pending_request= 0;    //store the ID of the BS asking for handover. If -1 then means come from the other AMF

        int other_AMF;  //index to transmit to the other AMF
        int myIndex;    //used to clean up the message channel 

    public:
        AMF(int id, int x, int y, int num_bs, int patched); //constructor

        int get_posX(); //return the value of x_pos
        int get_posY(); //return the value of y_pos

        void handle_message(message* msg_channel[], message* msg, int* type_transmission);  //handle the message and transmit the corresponding response message
};

AMF::AMF(int id, int x, int y, int num_bs, int patched){

    AMF_id= id;
    x_pos= x;
    y_pos= y;
    n_bs= (num_bs % 2 == 0)?    num_bs/2 : (num_bs-1)/2;
    myIndex= num_bs+AMF_id;
    is_patched= patched;

    BS_ids= (int*) calloc(n_bs, sizeof(int));
    for(int i=0; i<n_bs; ++i){
        if(AMF_id==1)   BS_ids[i]= i+1;
        else    BS_ids[i]= n_bs+i+1;
    }

    other_AMF= num_bs+(3-AMF_id);
}

int AMF::get_posX()    {return x_pos;}
int AMF::get_posY()    {return y_pos;}

void AMF::handle_message(message* msg_channel[], message* msg, int* type_transmission){

    char* msg_type= msg->get_type();
    
    //printf("AMF %d - Message received: %s\n", AMF_id, msg_type);

    if(!strcmp(msg_type, HO_RQED)){   //Handover Required

        pending_request= msg->get_content(0);   //extracting the transmitter for future retransmission.

        int t_index= find_Tindex(msg->get_content(2));    //get the index of tBS for transmitting message to it

        int n_content= 1+ ((t_index==-1)? 1:0);
        int content[n_content];
        message* new_msg;   //define the message object

        if(t_index == -1){  //if tBS does not belong to this AMF, then need to contact the other AMF
            content[0]= msg->get_content(1);    //extract the UE ID and insert it the content of the message
            content[1]= msg->get_content(2);   //extract the tBS ID and insert it the content of the message            

            t_index= other_AMF; //set the target as the other AMF
            *type_transmission= 3;

        }else{
            content[0]= {msg->get_content(1)};    //extract the UE ID and insert in the content of the message
            *type_transmission= 5;
        }

        if(is_patched){ //if patched version then need to compute the authentication token
            
            //AUTHENTICATION TOKEN EXTRACTION
            unsigned char* enc_token= msg->get_token();    //extract the encrypted authentication token
            unsigned char* dec_token= AES128_decryption(enc_token, UE_key); //decryption

            free(enc_token);    //free the allocated space

            unsigned int auth_token= 0;
            unsigned char* temp= (unsigned char*) &auth_token;
            for(int i=0; i<4; ++i)  *(temp + i)= dec_token[12+i];  //move the decrypted value into an int container

            auth_token+= 1;  //increase the value

            unsigned char temp1[4];
            unsigned char* temp2= (unsigned char*) &auth_token;
            for(int i=0; i<4; ++i)  temp1[i]= *(temp2 + i); //move the updated token into a string container

            unsigned char* temp3= AES128_encryption(temp1, 4, AMF_key); //encryption
            new_msg= new message("Handover Request", content, n_content, temp3);    //building the message

            free(temp3);    //free the allocated space

        }else new_msg= new message("Handover Request", content, n_content); //build the message

        msg_channel[t_index]= new_msg;    //transmit the message

        delete msg; //destroy the received message
        msg_channel[myIndex]= NULL; //clear the channel from the received message        
     
        return;
    }

    
    if(!strcmp(msg_type, HO_ACK)){   //Handover ack

        int t_index;    //index to which transmit
        message* new_msg;

        if(pending_request == -1){  //if pending request was from the other AMF, forward the message to it
            *type_transmission= 3;
            msg_channel[other_AMF]= msg;    //simple forwarding of the message received

        }else{  //if the pending request was from BS
            t_index= find_Tindex(pending_request);    //otherwise get the index of sBS for transmitting message to it
            *type_transmission= 5;

            int content[0]; //empty content            

            if(is_patched){ //if patched then need to compute the reconnection token

                rec_token= rand(); //generate the random value for the reconnection token

                //printf("AMF - reconnection token generated: %d\n", rec_token);

                //conversion into a string
                unsigned char temp[4];
                unsigned char* temp1= (unsigned char*) &rec_token;
                for(int i=0; i<4; ++i)  temp[i]= *(temp1 + i);

                unsigned char* temp2= AES128_encryption(temp, 4, UE_key); //encryption -> call fucntion 'calloc' inside
                new_msg= new message("Handover Command", content, 0, temp2);

                free(temp2);    //free the allocated space

            }else   new_msg= new message("Handover Command", content, 0); //build up the corresponding message        
        
            msg_channel[t_index]= new_msg;    //transmit the message
            delete msg; //destroy the received message
        }

        msg_channel[myIndex]= NULL; //clear the channel from the received message
        return;
    }


    if(!strcmp(msg_type, HO_REST)){ //Handover Request

        pending_request= -1;    //set the pending request to recall it comes from the AMF
        *type_transmission= 5;

        int t_index= find_Tindex(msg->get_content(1));  //search the tBS

        int content[]= {msg->get_content(0)};    //extract the UE ID and insert in the content of the message
        message* new_msg;

        if(is_patched){ //if patched version then need to extract the authentication token to add to the new message
            
            //AUTHENTICATION TOKEN EXTRACTION
            unsigned char* token= msg->get_token();    //extract the encrypted authentication token
            
            new_msg= new message("Handover Request", content, 1, token);    //building the message
            free(token);    //free the allocated space

        }else new_msg= new message("Handover Request", content, 1); //build the message

        msg_channel[t_index]= new_msg;    //transmit the message

        delete msg; //destroy the received message
        msg_channel[myIndex]= NULL; //clear the channel from the received message        
     
        return;
    }

    if(!strcmp(msg_type, REC)){ //Reconnection Recovery
        
        /*
        printf("-- Source id: %d\n", msg->get_content(0));
        printf("-- is base station?: %d\n", msg->get_content(1));
        printf("-- UE id: %d\n", msg->get_content(2));
        printf("-- AMF id: %d\n", msg->get_content(3));
        */

        if(msg->get_content(1) == 0){    //if request coming from the another AMF
            //printf("AMF %d - Reconnection recovery from other AMF\n", AMF_id);

            unsigned char* enc_token= msg->get_token(); //extract the encrypted token
            unsigned char* dec_token= AES128_decryption(enc_token, UE_key); //decryption

            unsigned int temp= 0;
            unsigned char* temp1= (unsigned char*) &temp;
            for(int i=0; i<4; ++i)  *(temp1 + i)= dec_token[12+i];  //move the decrypted value into an int container

            int content[0];
            message* new_msg;

            if(temp == rec_token+1){    //if the reconnection token is correct -> need to answer back
                rec_token+= 2;  //update the token

                //printf("AMF %d - token new transmitted: %u\n", AMF_id, rec_token);

                unsigned char temp2[4];
                temp1= (unsigned char*) &rec_token;
                for(int i=0; i<4; ++i)  temp2[i]= *(temp1 + i); //move the updated token into a string container

                unsigned char* temp3= AES128_encryption(temp2, 4, UE_key);  //encrypt the reconnection token
                new_msg= new message("Reconnection Recovery OK", content, 0, temp3);    //building the message

                free(temp3);

            }else   new_msg= new message("Reconnection Recovery Rejected", content, 0);

            free(enc_token);
            free(dec_token);

            msg_channel[other_AMF]= new_msg;

            delete msg;
            msg_channel[myIndex]= NULL;

            *type_transmission= 3;
            return;

        }else{  //request coming from a BS
            if(msg->get_content(3) == AMF_id){   //if *this is the target AMF

                //printf("AMF %d - Reconnection recovery from BS\n", AMF_id);

                unsigned char* enc_token= msg->get_token(); //extract the encrypted token
                unsigned char* dec_token= AES128_decryption(enc_token, AMF_key); //decryption

                unsigned int temp= 0;
                unsigned char* temp1= (unsigned char*) &temp;
                for(int i=0; i<4; ++i)  *(temp1 + i)= dec_token[12+i];  //move the decrypted value into an int container

                int content[0];
                message* new_msg;

                if(temp == rec_token+1){    //if the reconnection token is correct -> need to answer back
                    rec_token+= 2;  //update the token

                    unsigned char temp2[4];
                    temp1= (unsigned char*) &rec_token;
                    for(int i=0; i<4; ++i)  temp2[i]= *(temp1 + i); //move the updated token into a string container

                    unsigned char* temp3= AES128_encryption(temp2, 4, UE_key);  //encrypt the reconnection token
                    new_msg= new message("Reconnection Recovery OK", content, 0, temp3);    //building the message

                    free(temp3);

                }else   new_msg= new message("Reconnection Recovery Rejected", content, 0);

                free(enc_token);
                free(dec_token);

                msg_channel[find_Tindex(msg->get_content(0))]= new_msg;

                delete msg;
                msg_channel[myIndex]= NULL;

                *type_transmission= 5;
                return;

            }else{  //if *this is NOT the target AMF, then need to contact the other
                
                //printf("AMF %d - Reconnection recovery for other AMF\n", AMF_id);

                pending_request= msg->get_content(0);   //save the BS the request is coming from for future answer transmisison

                int content[]= {AMF_id, 0, msg->get_content(2), msg->get_content(3)};   //AMF id, is_BS: 0= no (-> is AMF) 1= yes, UE id, AMF id
                unsigned char* temp= msg->get_token();

                message* new_msg= new message("Reconnection Recovery", content, 4, temp);

                msg_channel[other_AMF]= new_msg;

                delete msg;
                msg_channel[myIndex]= NULL;

                *type_transmission= 3;
                return;
            }
        }
    }

    if(!strcmp(msg_type, RECON_OK)){  //Reconnection Recovery OK
        //this type of message can arrive only from another AMF -> only need to forward the message

        msg_channel[pending_request]= msg;
        msg_channel[myIndex]= NULL;

        *type_transmission= 5;
        return;
    }

    if(!strcmp(msg_type, RECON_REJ)){   //Reconnection Recovery Rejected
        //this type of message can arrive only from another AMF -> only need to forward the message
        msg_channel[pending_request]= msg;
        msg_channel[myIndex]= NULL;

        *type_transmission= 5;
        return;
    }
}


//------------------------------------------------------------
int AMF::find_Tindex(int tID){

    for(int i=0; i<n_bs; ++i){
        if(BS_ids[i]==tID)  return tID;
    }

    return -1;
}
#endif  /*BASE_STATION_H*/