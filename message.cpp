/*
    @Author/Owner: Alessandro Lotto
    @Last update: 25/02/2023
    @Note: This code has been build completely from scratch

    @Description:
        This file defines the Message entity for the simulation.
        It containes the info and functions for building messages that are exchanged between parties during the simulation. 
*/

#ifndef MESSAGE_H
#define MESSAGE_H

#include <string.h>
//#include <vector>

class message{

    private:
        char type[50];
        int content[10];
        unsigned char token[16];
        int is_token;   //define whether the message has the token field populated
        int n_content= 0;

    public:
        message(const char* msg_type, int msg_content[], int n_content);
        message(const char* msg_type, int msg_content[], int n_content, unsigned char* tk);

        char* get_type();   //return a pointer to the type field
        int* get_content(); //return a pointer to the content field
        int get_content(int index); //return the value of content in position index
        int get_n_content();    //return the number of entries of the content field
        unsigned char* get_token(); //return a pointer to an allocated copy of the token value

        void print_type();  //print type
        void print_content();   //print content
};

message::message(const char* msg_type, int msg_content[], int n_content){

    strcpy(type, msg_type);
    for(int i=0; i<n_content; ++i)  content[i]= msg_content[i];
    this->n_content= n_content;

    is_token= 0;
}

message::message(const char* msg_type, int msg_content[], int n_content, unsigned char* tk){

    strcpy(type, msg_type);
    for(int i=0; i<n_content; ++i)  content[i]= msg_content[i];
    this->n_content= n_content;

    for(int i=0; i<16; ++i) token[i]= tk[i];

    is_token= 1;
}

char* message::get_type()   {return type;}
int* message::get_content()  {return content;}
int message::get_content(int index) {return content[index];}
int message::get_n_content()    {return n_content;}

unsigned char* message::get_token(){
    if(!is_token)   return NULL;    //if there is no token then return NULL

    unsigned char* copy_token= (unsigned char*) calloc(16, 1);  //allocate the memory space
    for(int i=0; i<16; ++i) copy_token[i]= token[i];

    return copy_token;
}

void message::print_type()  {printf("Message type: %s\n", type);}
void message::print_content()   {for(int i=0; i<n_content; i++) printf("Content %d: %d\n", i, content[i]);}

#endif  /*MESSAGE_H*/