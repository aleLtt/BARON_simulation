/*
    @Author/Owner: Alessandro Lotto
    @Last update: 25/02/2023
    @Note: This code has been build completely from scratch

    @Description:
        This file defines the UE entity for the simulation and implement methods for its actions.
*/

#ifndef USER_H
#define USER_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "message.cpp"
#include "./crypto.cpp"

using namespace std;

const char MR[]= "Measurement Report";
const char HC[]= "Handover Command";
const char RO[]= "RACH OK";
const char REC_OK[]= "Reconnection Recovery OK";
const char REC_REJ[]= "Reconnection Recovery Rejected";

unsigned char AMF_key[]= "abcdefghilmnopqr";
unsigned char sBS_key[]= "djv0ncjodnon0nnn";

class user{
    private:
        int ue_id;  //user identifier
        int x_pos;  //x-axis coordinate position
        int y_pos;  //y-axis coordinate position

        int connected;   //containes the ID of the connected base station -> for the simulation we assume it is already connected to with a BS
        int s_index;  //is the index used to transmit with sBS
        int target= 0;  //contains the ID of the traget cell
        int t_index= 0;    //used for transmittig messages - simulates the antenna steering
        int sAMF;   //contains the Id of the current serving AMF
        int n_bs;   //number of surrounding base stations 

        int is_patched; //descriminates whether to apply the patched version
        int if_attacker;    //used to determine if there is the attacker. It is used only for the transmission type variable so to correctly compute the trransmission delay
        
        unsigned int auth_token;  //authentication token. Used only in the patched version
        unsigned int rec_token;  //reconnection token. Used only in the patched version
        unsigned char rec_token_enc[4];   //used in case of reconnection with the sBS -> use the encrypted value

        int find_Tindex(double channel[][2], int tID);   //find the index for the msg_channel of tID

    public:
        user(int id, int x, int y, int patched, int attacker); //constructor

        int get_id();   //return the ID of the UE
        int get_posX(); //return the value of x_pos
        int get_posY(); //return the value of y_pos
        int get_connected();    //return the value of connected
        int get_target();   //return the value of target
        int get_AMF();  //return the value of sAMF

        void set_connected(int sBS);
        void set_sIndex(int index);
        void set_target(int tBS);   //set the ID of the target BS for handover
        void set_AMF(int amf);

        int select_best_bs(double channel[][2], int n_bs);    //returns the ID of the base station for which receive the best signal
        int select_best_bs(double channel[][2], int n_bs, int exclude);    //returns the ID of the base station for which receive the best signal excluding the given ID
        int transmit_message(message* msg_channel[], const char* message_type); //transmit the message
        void handle_message(message* msg_channel[], message* msg, double channel[][2], int* handover_completed, int* type_transmission);

};

user::user(int id, int x, int y, int patched, int attacker){
    ue_id= id; 
    x_pos= x;   
    y_pos= y; 
    is_patched= patched;
    if_attacker= attacker;
}

int user::get_id()      {return ue_id;}
int user::get_posX()    {return x_pos;}
int user::get_posY()    {return y_pos;}
int user::get_connected()   {return connected;}
int user::get_target()  {return target;}
int user::get_AMF()     {return sAMF;}

void user::set_connected(int sBS)   {connected= sBS;}
void user::set_sIndex(int index)    {s_index= index;}
void user::set_target(int tBS)  {target= tBS;}
void user::set_AMF(int amf)     {sAMF= amf;}

int user::select_best_bs(double channel[][2], int n_bs){

    this->n_bs= n_bs;

    int index= 0;
    for(int i=1; i<n_bs; ++i){
        if(channel[i][0] > channel[index][0])   index= i;
    }

    t_index= index+1;  //+1 because in the data structure for messages, the BSs are shifted of one position with respect to the data structure for beacons

    return (int)channel[index][1];
}


//returns the ID of the base station for which receive the best signal, excluding the @exclude ID value
int user::select_best_bs(double channel[][2], int n_bs, int exclude){

    int index= (channel[0][1] != exclude)?  0 : 1;

    for(int i=1+index; i<n_bs; ++i){
        if( (channel[i][0] > channel[index][0]) && (channel[i][1] != exclude))   index= i;
    }

    t_index= index+1;  //+1 because in the data structure for messages, the BSs are shifted of one position with respect to the data structure for beacons

    return (int)channel[index][1];

}


int user::transmit_message(message* msg_channel[], const char* message_type){

    //printf("UE - Measurement Report Transmission\n");

    if(!strcmp(message_type, MR)){  //Measurement Report case

        int content[2];
        content[0]= ue_id;
        content[1]= target; //ID of tBS

        message* msg;

        if(is_patched){ //if patched version
            auth_token= rand(); //generate the random value for the authentication token

            //conversion into a string
            unsigned char temp[4];
            unsigned char* temp1= (unsigned char*) &auth_token;
            for(int i=0; i<4; ++i)  temp[i]= *(temp1 + i);

            unsigned char* temp2= AES128_encryption(temp, 4, AMF_key); //encryption
            msg= new message(message_type, content, 2, temp2);

            free(temp2);    //free the allocated space

        }else   msg= new message(message_type, content, 2+1*is_patched);

        msg_channel[s_index]= msg;
        return 1;
    }    
    return 0;   //in case the message has not correctly sent
}


void user::handle_message(message* msg_channel[], message* msg, double channel[][2], int* handover_completed, int* type_transmission){
    
    char* msg_type= msg->get_type();
    //printf("UE - Message received: %s\n", msg_type);

    if(!strcmp(msg_type, HC)){   //Handover command

        if(is_patched){ //if patched versione then need to save the reconnection token

            //RECONNECTION TOKEN EXTRACTION
            unsigned char* enc_token= msg->get_token();    //extract the encrypted authentication token
            for(int i=0; i<4; ++i)  rec_token_enc[i]= enc_token[12+i];  //save the 4 least-significant bytes of the encrypted value in case for reconnection with sBS
            unsigned char* dec_token= AES128_decryption(enc_token, AMF_key); //decryption            

            rec_token= 0;
            unsigned char* temp= (unsigned char*) &rec_token;
            for(int i=0; i<4; ++i)  *(temp + i)= dec_token[12+i];  //move the decrypted value into the int container

            //printf("UE - reconnection token extracted: %d\n", rec_token);

            free(enc_token);    //free the allocated space
            free(dec_token);    //free the allocated space
        }

        int content[]= {};   //empty content
        message* new_msg= new message("RACH procedure", content, 0); //build up the corresponding message
        msg_channel[t_index]= new_msg;    //transmit the message

        delete msg; //destroy the received message
        msg_channel[0]= NULL;   //index 0 because is the UE

        *type_transmission= 1;

        return;
    }

    if(!strcmp(msg_type, RO)){   //RACH OK
        if(is_patched){

            unsigned char* enc_token= msg->get_token(); //extract the encrypted token
            unsigned char* dec_token= AES128_decryption(enc_token, AMF_key); //decryption

            unsigned int temp= 0;
            unsigned char* temp1= (unsigned char*) &temp;
            for(int i=0; i<4; ++i)  *(temp1 + i)= dec_token[12+i];  //move the decrypted value into an int container

            if(auth_token+1 == temp){   //if authentication token is correct
                *type_transmission= 0;
                *handover_completed= 1;
                //printf("UE - Authentication token correct\n");
            }else{
                //authentication token is not correct, thus consider it as an attack
                // -> need to recover a legitimate connection
                
                //printf("UE - Authentication token NOT correct -> proceed for reconnection\n");
                
                int new_target= select_best_bs(channel, n_bs, target);  //look for the best BS but excluding the previously selected tBS
                //printf("UE - new target for reconnection: %d\n", new_target);

                target= new_target; //set the new target
                t_index= target;
                
                if(new_target != connected){    //if new target BS different from sBS

                    //printf("UE - Reconnection with different BS\n");              

                    rec_token++;    //update

                    int content[]= {ue_id, sAMF};
                    
                    unsigned char temp2[4];
                    unsigned char* temp3= (unsigned char*) &rec_token;
                    for(int i=0; i<4; ++i)  temp2[i]= *(temp3 + i);

                    unsigned char* temp4= AES128_encryption(temp2, 4, AMF_key); //encryption
                    message* new_msg= new message("Reconnection Recovery", content, 2, temp4);

                    msg_channel[t_index]= new_msg;    //transmit the message
                    delete msg; //destroy the received message
                    msg_channel[0]= NULL;   //index 0 because is the UE
                    *type_transmission= 1;

                    free(temp4);    //free the allocated space

                }else{
                    //if previous sBS, then can use directly it as CTE
                
                    for(int i=0; i<4; ++i)  rec_token_enc[i]^= sBS_key[i]; //need to transform the encrypted reconnection token -> we implement a simple XOR with the key of sBS

                    int content[]= {ue_id};  //empty content
                    message* new_msg= new message("Reconnection Recovery", content, 1, rec_token_enc);  //generate the new message

                    msg_channel[s_index]= new_msg;    //transmit the message
                    delete msg; //destroy the received message
                    msg_channel[0]= NULL;   //index 0 because is the UE
                    *type_transmission= 1;
                }               
            }
            
            free(enc_token);
            free(dec_token);

            return;

        } //#if(is_patched)

        *type_transmission= 0;  //not doing anything -> no message transmitted so no time to compute needed
        *handover_completed= 1;

        return;

    } //#if(RACH OK)

    if(!strcmp(msg_type, REC_OK)){  //Reconnection Recovery OK

        if(target == connected){    //if reconnection with sBS
            unsigned char* temp= msg->get_token();  //extraction of the received token      

            int temp1= 0;
            unsigned char* temp2= (unsigned char*) &temp1;
            for(int i=0; i<4; ++i)  *(temp2+i)= temp[i];   //move the received reconnection token into an int container

            //printf("UE - Reconnection token extracted: %u\n", temp1);

            free(temp); //free the alloated space

            //now need to transform the encrypted reconnection token into an int value
            rec_token= 0;      
            temp2= (unsigned char*) &rec_token;
            for(int i=0; i<4; ++i)  *(temp2+i)= rec_token_enc[i];

            if(rec_token+1 == temp1)    *handover_completed= 2;    //verification of the token
            else    *handover_completed= -2;

            *type_transmission= 0;  //not doing anything -> no message transmitted so no time to compute needed

            return;
        
        }else{  //if reconnection with other BS
            unsigned char* enc_token= msg->get_token(); //extract the encrypted token
            unsigned char* dec_token= AES128_decryption(enc_token, AMF_key); //decryption

            unsigned int temp= 0;
            unsigned char* temp1= (unsigned char*) &temp;
            for(int i=0; i<4; ++i)  *(temp1 + i)= dec_token[12+i];  //move the decrypted value into an int container

            //printf("UE - token obtained: %u, token expected: %u\n", temp, rec_token+1);

            if(rec_token+1 == temp) *handover_completed= 2;
            else    *handover_completed= -2;

            *type_transmission= 0;
            return;
        }
    }

    if(!strcmp(msg_type, REC_REJ)){  //Reconnection Recovery Rejected

        *type_transmission= 0;  //not doing anything -> no message transmitted so no time to compute needed
        *handover_completed= -1;
        return;

    }
}

//--------------------------------

int user::find_Tindex(double channel[][2], int tID){

    for(int i=0; i<n_bs; ++i)   if(channel[i][1]==tID)  return i+1;
}

#endif  /*USER_H*/