/*
    @Author/Owner: Alessandro Lotto
    @Last update: 25/02/2023
    @Note: This code has been build completely from scratch

    @Description:
        This file defines a set of utility functions.
*/

#ifndef UTILITY_H
#define UTILITY_H

#include <math.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

#include "user.cpp"
#include "base_station.cpp"
#include "message.cpp"

double LIGHT_SPEED_FREE = 3e8;
double LIGHT_SPEED_WIRE = 2e8;

using namespace std;

typedef struct{

    float value;
    int occurences; 

}stat_t;


class Utility{

    public:
        double compute_distance(auto* from, auto* to);  //compute the distance between from and to 
        void ue_set_connected(user* ue, vector <base_station*> bs, int n_bs);  //set to which BS the UE gets connected for simulation initialization
        void ue_set_AMF(user* ue, vector <base_station*> bs, int n_bs); //set the AMF for the UE 
        void transmit_beacons(double channel[][2], int n_bs, user* ue, vector <base_station*> bs);   //fulfill the channel with the trabnsmitted signals
        double compute_delay(user* ue, base_station* bs, int transmission_channel);   //compute the transmission delay between ue and bs
        int random_selection(int min, int max); //return an integer random number within min and max
        void group(vector <float> a, vector <stat_t>* b);
        void occurences_to_probability(vector <stat_t>* b);
        float expected_value(vector <stat_t>* b);
        void sort(vector <stat_t>* b);
};


/**
 * Compute the Euclidean distance between two entities
 */
double compute_distance(auto* from, auto* to){

    return sqrt(pow(from->get_posX() - to->get_posX(), 2) + pow(from->get_posY() - to->get_posY(), 2));
}

/**
 * Set to which BS the UE gets connected for simulation initialization -> the second closer
*/
void ue_set_connected(user* ue, vector <base_station*> bs, int n_bs){

    float closest= compute_distance(ue, bs[0]);
    float closest_2= compute_distance(ue, bs[1]);
    int index= 0;
    int index_2= 1;

    if(closest_2 < closest){
        float temp= closest;
        closest= closest_2;
        closest_2= temp;

        index= 1;
        index_2= 0;
    }

    for(int i=2; i<n_bs; ++i){
        float distance= compute_distance(ue, bs[i]);
        if(distance < closest){
            closest_2= closest;
            closest= distance;

            index_2= index;
            index= i;
        }else if(distance < closest_2){
            closest_2= distance;
            index_2= i;
        }
    }

    ue->set_connected(bs[index_2]->get_id());   //set the ID of the connected base station
    ue->set_sIndex(index_2+1);  //set the index of the channel for message transmission
    bs[index_2]->activate_context(ue->get_id());    //activate the UE-context for the target BS
    ue->set_AMF(bs[index_2]->get_AMF());

    //printf("to connection: %d\n", bs[index_2]->get_id());
}

/**
 * Set the AMF value for the UE. This is derived from the current serving BS
*/
void ue_set_AMF(user* ue, vector <base_station*> bs, int n_bs){

    int BS= ue->get_connected();
    ue->set_AMF(bs[BS-1]->get_AMF());
}

/**
 * Fulfill the channel with the power signal received by the user
 */
void transmit_beacons(double channel[][2], int n_bs, user* ue, vector <base_station*> bs){
    
    for(int i=0; i<n_bs; ++i){  
        channel[i][0]= bs[i]->get_power() / pow(compute_distance(ue, bs[i]), 2);
        channel[i][1]= bs[i]->get_id();
    }
}


double compute_delay(auto from, auto to, int transmission_channel){

    //  transmission_channel= 0 -> free space; 1 -> wired

    return (compute_distance(from, to) / ((transmission_channel==0)?   LIGHT_SPEED_FREE : LIGHT_SPEED_WIRE));
}

int random_selection(int min, int max){
    
    if(min<0)   min=0;
    return min + ( rand() % (max-min));
}

void group(vector <float> a, vector <stat_t>* b){
    
    for(int i=0; i< a.size(); ++i){ //scan all element of vector a

        int j=0;
        for(j; j<b->size(); ++j){  //scan all elements of vector b
            if(a[i]== ((*b)[j]).value){
                ((*b)[j]).occurences++;  //if already stored the value, then just need to increase the occurrence value
                break;
            }
        }
        
        if(j==b->size()){    //if scanned all the elements but not found, then need to add
            stat_t temp;
            temp.value= a[i];
            temp.occurences= 1;

            b->push_back(temp);
        }
    }
}

void occurences_to_probability(vector <stat_t>* b){

    int n=0;
    for(int i=0; i< b->size(); ++i) n+= ((*b)[i]).occurences;
    for(int i=0; i< b->size(); ++i) ((*b)[i]).occurences /= n;
}

float expected_value(vector <stat_t>* b){
    float expected= 0.0;

    for(int i=0; i < b->size(); ++i) expected+= ((*b)[i]).value * ((*b)[i]).occurences;   

    return expected;
}

void sort(vector <stat_t>* b){
    vector <stat_t> a;

    while(b->size() > 0){
        int min_pos= 0;
        float min_value= ((*b)[0]).value;

        for(int i=1; i< b->size(); ++i){
            if(min_value > ((*b)[i]).value){
                min_pos = i;
                min_value = ((*b)[i]).value;
            }
        }

        a.push_back((*b)[min_pos]);
        b->erase(b->begin()+min_pos);
    }

    for(int i=0; i< a.size(); ++i)  b->push_back(a[i]);
}



#endif  /*UTILITY_H*/