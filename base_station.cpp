/*
    @Author/Owner: Alessandro Lotto
    @Last update: 25/02/2023
    @Note: This code has been build completely from scratch

    @Description:
        This file defines the Base Station entity for the simulation and implement methods for its actions.
*/

#ifndef BASE_STATION_H
#define BASE_STATION_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "message.cpp"

using namespace std;

const char MS_REP[]= "Measurement Report";
const char HO_REQ[]= "Handover Request";
const char HO_COM[]= "Handover Command";
const char RACH[]= "RACH procedure";
const char REC_REC[]= "Reconnection Recovery";
const char REC_REC_OK[]= "Reconnection Recovery OK";
const char REC_REC_REJ[]= "Reconnection Recovery Rejected";

unsigned char ue_key[]= "djv0ncjodnon0nnn";


class base_station{
    private:
        int BS_id;  //BS identifier
        int x_pos;  //x-axis coordinate position
        int y_pos;  //y-axis coordinate position

        int active_context= -1; //contains the ID of the UE for which this BS has an active context. If -1 => no active context; -> POSSIBLE TO GENERALIZE IN CASE OF MULTI USER -> TRANSORM INTO A VECTOR

        int t_power= 100; //power transmission

        int under_AMF;  //contains the ID of the AMF which this BS is controlled by
        int amf_index;  //index for transmitting messages to the AMF

        unsigned char* auth_token;  //used only in the patched version - authenticates the BS
        unsigned char* rec_token;   //used only in the patched version - reconnection recovery in case of attack
        //-> here I take it as char* because transform in int only in case of reconnection -> if handover ok, then this would have been useless computation

        int is_patched; //descriminates whether to apply the patched version
        int is_attacker;    //defines if the BS is the attacker or not: 0= no; 1= yes
        

    public:
        base_station(int id, int x, int y, int patched, int is_attacker, int amf_id, int amf_index); //constructor

        int get_posX(); //return value of x_pos
        int get_posY(); //return value of y_pos
        int get_power();    //return value t_power
        int get_id();   //return value of BS_id
        int get_AMF();  //return the ID of the AMF under the control of which the BS is
        int get_active_context();   //return the value of active_context

        void activate_context(int UE_id); //set the UE_id as active context

        void handle_message(message* msg_channel[], message* msg, int* type_transmission);  //handle the incoming message and transmit the corresponding response
};

base_station::base_station(int id, int x, int y, int patched, int is_attacker, int amf_id, int amf_index){

    BS_id= id;
    x_pos= x;
    y_pos= y;
    is_patched= patched;
    this->is_attacker= is_attacker;

    under_AMF= amf_id;
    this->amf_index= amf_index;

}

int base_station::get_posX()    {return x_pos;}
int base_station::get_posY()    {return y_pos;}
int base_station::get_power()   {return t_power;}
int base_station::get_id()      {return BS_id;}
int base_station::get_AMF()     {return under_AMF;}
int base_station::get_active_context()  {return active_context;}

void base_station::activate_context(int UE_id)    {active_context= UE_id;}

void base_station::handle_message(message* msg_channel[], message* msg, int* type_transmission){

    char* msg_type= msg->get_type();
    
    //if(is_attacker) printf("BS %d (attacker) - Message received: %s\n", BS_id, msg_type);
    //else    printf("BS %d - Message received: %s\n", BS_id, msg_type);

    if(!strcmp(msg_type, MS_REP)){   //Measurement report

        message* new_msg;
        int content[]= {BS_id, msg->get_content(0), msg->get_content(1)};  //ID of base station, ID of user, ID of tBS

        if(is_patched){            
            unsigned char* temp= msg->get_token();  //extract the token -> dynamic memory stored
            new_msg= new message("Handover Required", content, 3, temp);    //build the message with the token

            free(temp); //free the allocated space
        }else   new_msg= new message("Handover Required", content, 3); //build up the corresponding message

        msg_channel[amf_index]= new_msg;    //transmit the message

        delete msg; //destroy the received message
        msg_channel[BS_id]= NULL;   //clear the channel from the message received

        *type_transmission= 2;
        return;
    }
    
    if(!strcmp(msg_type, HO_REQ)){   //Handover request

        if(is_patched)  auth_token= msg->get_token();   //if patched version then need to extract and store the authentication token

        int content[]= {};   //empty message
        message* new_msg= new message("Handover ACK", content, 0); //build up the corresponding message
        msg_channel[amf_index]= new_msg;    //transmit the message

        delete msg; //destroy the received message
        msg_channel[BS_id]= NULL;   //index 2 because the BS is the target -> NEED TO GENERALIZE

        *type_transmission= 2;

        return;
    }

    if(!strcmp(msg_type, HO_COM)){  //Handover command

        if(is_patched)  rec_token= msg->get_token();    //if patched version then need to store the encrypted reconnection token 

        msg_channel[0]= msg;    //simulation of forwarding the message to UE
        msg_channel[BS_id]= NULL;   //index 1 because the BS is the serving -> NEED TO GENERALIZE

        *type_transmission= 6;
       
        return;   
    }

    if(!strcmp(msg_type, RACH)){    //RACH procedure

        int content[0];
        message* new_msg;

        if(is_attacker){

            int token= rand();  //get a random number
            unsigned char temp[4];
            unsigned char* temp1= (unsigned char*) &token;
            for(int i=0; i<4; ++i)  temp[i]= *(temp1 + i);

            new_msg= new message("RACH OK", content, 0, temp1);  //generate the message
            msg_channel[0]= new_msg;    //transmit the message

            delete msg; //destroy the received message
            msg_channel[13]= NULL;   //index because the BS is the attacker --> TRY TO AUTOMATIZE IT

            *type_transmission= 6;
        }else{

            if(is_patched)  new_msg= new message("RACH OK", content, 0, auth_token);
            else new_msg= new message("RACH OK", content, 0); //build up the corresponding message

            msg_channel[0]= new_msg;    //transmit the message

            delete msg; //destroy the received message
            msg_channel[BS_id]= NULL;   //index 2 because the BS is the target

            *type_transmission= 6;
        }
            
        return;        
    }

    if(!strcmp(msg_type, REC_REC)){ //Reconnection Recovering
        
        if(active_context == msg->get_content(0)){
            //printf("BS %d - active context for UE: %d\n", BS_id, active_context);
            //if the context for the UE is active, then no need to pass through the AMF
        
            unsigned char temp[4];
            for(int i=0; i<4; ++i)  temp[i]= rec_token[12+i];   //extraction of the 4 least-significant bytes
            for(int i=0; i<4; ++i)  temp[i]^= ue_key[i];    //apply the XOR transformation

            unsigned char* temp1= msg->get_token(); //extraction of the received token
            int check= 1;   //used for signal reconnection token correctness

            //verification of the reconnection token
            for(int i=0; i<4; ++i){
                if(temp[i] != temp1[i]){    //if do not correspond => can not accept the reconnection
                    check= 0;
                    break;
                }
            }

            free(temp1);

            int content[0];
            message* new_msg;

            if(check){  //if authentication ok -> accept the reconnection
                
                int temp2= 0;
                temp1= (unsigned char*) &temp2;
                for(int i=0; i<4; ++i)  *(temp1 + i)= temp[i];  //move the reconnection token value into an int container

                temp2+= 1;
                //printf("BS %d - reconnection ok, new token: %u\n", BS_id, temp2);

                unsigned char temp3[4];
                temp1= (unsigned char*) &temp2;
                for(int i=0; i<4; ++i)  temp3[i]= *(temp1 + i); //move the updated token into a string container

                new_msg= new message("Reconnection Recovery OK", content, 0, temp3);    //building the message

            }else   new_msg= new message("Reconnection Recovery Rejected", content, 0);

            msg_channel[0]= new_msg;    //transmit the message to UE

            delete msg; //destroy the received message
            msg_channel[BS_id]= NULL;

            *type_transmission= 6;        
            return;

        }else{
            //printf("BS %d - NO active context for UE\n", BS_id);
            
            //if no active context for the UE, need to forward the message to the AMF
            int content[]= {BS_id, 1, msg->get_content(0), msg->get_content(1)};   //BS id, is_BS: 0= no (-> is AMF) 1= yes, UE id, AMF id
            unsigned char* temp= msg->get_token();

            message* new_msg= new message("Reconnection Recovery", content, 4, temp);
            
            msg_channel[amf_index]= new_msg;

            delete msg;
            msg_channel[BS_id]= NULL;

            *type_transmission= 2;
            return;
        }
    }

    if(!strcmp(msg_type, REC_REC_OK)){  //Reconnection Recovery OK
        //this type of message can arrive only from the AMF -> need just to forward the message

        msg_channel[0]= msg;
        msg_channel[BS_id]= NULL;

        *type_transmission= 6;
        return;
    }

    if(!strcmp(msg_type, REC_REC_REJ)){  //Reconnection Recovery OK
        //this type of message can arrive only from the AMF -> need just to forward the message

        msg_channel[0]= msg;
        msg_channel[BS_id]= NULL;

        *type_transmission= 6;
        return;
    }
}


#endif  /*BASE_STATION_H*/