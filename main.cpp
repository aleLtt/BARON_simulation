/*
    @Author/Owner: Alessandro Lotto
    @Last update: 25/02/2023
    @Note: This code has been build completely from scratch

    @Description:
        This file implements a simplified software simulation for 5G handover to evaluate the performance of BARON.
        Three possible scenarios are implemented:
            1- Standard handover with no BARON in place
            2- Handover using BARON & no FBS attack in place
            3- Handover using BARON & FBS attack runnin -> in case of attack success => BARON connection recovery mechanism is activated
        The simulation does not implement all the specifications of 3GPP standard handover, but only those that are 
        enough and necessary for BARON overhead performance evaluation.
*/

#include <stdio.h>
#include <vector>
#include <string>
#include <chrono>

#include "user.cpp"
#include "base_station.cpp"
#include "AMF.cpp"
#include "Utility.cpp"
#include "message.cpp"

using namespace std;
using namespace chrono;

int main(){

    srand(10);  //for replicability of results

    // DEFINITIONS FOR WRITING IN FILES THE RESULTS
/*  
    // Used when simulation of standard handover

    char nameFile1_std[]= "results1_std.xls";   //name of file for printing results
    FILE* write1= fopen(nameFile1_std, "w");    //open the file in writing mode

    char nameFile2_std[]= "results2_std.xls";   //name of file for printing results
    FILE* write2= fopen(nameFile2_std, "w");    //open the file in writing mode
*/
/*
    // Used when simulation of BARON but without considering the FBS attack scenario -> BARON hoverhead

    char nameFile1_patch[]= "results1_patch.xls";   //name of file for printing results
    FILE* write1= fopen(nameFile1_patch, "w");    //open the file in writing mode

    char nameFile2_patch[]= "results2_patch.xls";   //name of file for printing results
    FILE* write2= fopen(nameFile2_patch, "w");    //open the file in writing mode
*/
///*

    // Used when simulation of BARON with considering the FBS attack scenario -> BARON handover + connection recvery

    char nameFile1_patch_att[]= "results1_patch_att.xls";   //name of file for printing results
    FILE* write1= fopen(nameFile1_patch_att, "w");    //open the file in writing mode

    char nameFile2_patch_att[]= "results2_patch_att.xls";   //name of file for printing results
    FILE* write2= fopen(nameFile2_patch_att, "w");    //open the file in writing mode
///*
    char nameFile3_patch_att[]= "results3_patch_att.xls";   //name of file for printing results
    FILE* write3= fopen(nameFile3_patch_att, "w");    //open the file in writing mode


    // SCENARIO DEFINITION

    vector <float> overall_time1;   //keeps the total execution time for handover when tBS is in sAMF  (in case of attack, tBS is for the reconnection)
    vector <float> overall_time2;   //keeps the total execution time for handover when tBS is NOT in sAMF   (in case of attack, tBS is for the reconnection)
    vector <float> overall_time3;   //keeps the total execution time for handover case under attack and reconnection with sBS 

    int handover_version= 1;    //defines the handover scenario: 0= standard; 1= patched
    int is_attacker= 1; //defines if there is the attacker: 0 = No ; 1 = Yes
    int n_rounds= 1001; //number of simulation runs we want for each case scenario
    int n_bs= 12 + is_attacker*1;    //number of base stations (BS) -> this comprises also the attacker

    if(handover_version)    printf("PATCHED HANDOVER\n");
    else    printf("STANDARD HANDOVER\n");
    if(is_attacker) printf("WITH ATTACKER\n");
    else printf("NO ATTACKER\n");
    printf("\n");

    int j=0;
    while((overall_time1.size() < n_rounds || overall_time2.size() < n_rounds) && ((((int) overall_time3.size()) - 1*(1-is_attacker)) < n_rounds*is_attacker)){
        //The condition for stopping the simulation looks at whether we have reached a certain number of simulations for each of the different scenarios define by the "overall_timeX" variables
        
        //printf("------------------- ROUND %d ----------------------\n", j);
        j++;

        //---------------------------------------------- INITIALIZATION -----------------------------//

        vector <float> time; //maintains the round time simulation

        /*
            Used for simulation stop condition. Its value define the result of the handover
            - 1= handover successful
            - 2= handover failed + reconnection recovery successful
            - -1= handover failed + reconnection recovery rejected
            - -2= handover failed + reconnection recovery aborted (BS failed in authentication)
        */
        int handover_completed= 0;

        //Creation of AMFs
        vector <AMF*> amf(2);
        amf[0]= new AMF(1, 0, 0, n_bs, handover_version); //create the AMF-1 and the corresponding BSs are assigned under its control
        amf[1]= new AMF(2, 2200, 1100, n_bs, handover_version); //create the AMF-2 and the corresponding BSs are assigned under its control
        //puts("-- AMF creation OK");

        //Creation of the BSs
        vector <base_station*> bs(n_bs); //collection of base stations
        bs[0]= new base_station(1, 200, 1000, handover_version, 0, 1, n_bs + 1);
        bs[1]= new base_station(2, 450, 800, handover_version, 0, 1, n_bs + 1);
        bs[2]= new base_station(3, 800, 400, handover_version, 0, 1, n_bs + 1);
        bs[3]= new base_station(4, 1000, 50, handover_version, 0, 1, n_bs + 1);
        bs[4]= new base_station(5, 100, 650, handover_version, 0, 1, n_bs + 1);
        bs[5]= new base_station(6, 300, 200, handover_version, 0, 1, n_bs + 1);

        bs[6]= new base_station(7, 950, 900, handover_version, 0, 2, n_bs + 2);
        bs[7]= new base_station(8, 1250, 500, handover_version, 0, 2, n_bs + 2);
        bs[8]= new base_station(9, 1400, 250, handover_version, 0, 2, n_bs + 2);
        bs[9]= new base_station(10, 1200, 1100, handover_version, 0, 2, n_bs + 2);
        bs[10]= new base_station(11, 1850, 1000, handover_version, 0, 2, n_bs + 2);
        bs[11]= new base_station(12, 2000, 750, handover_version, 0, 2, n_bs + 2);
        //printf("-- BSs creation OK\n");

        //Creation of the UE
        if(!handover_version && j!=1)   {random_selection(0, 2200);    random_selection(0, 1300);}  //this is to make the simulations with and without BARON to return the same user positions -> empitically discovered this relationship

        user* ue= new user(12, random_selection(0, 2200), random_selection(0, 1300), handover_version, is_attacker);  //located at random position
        
        ue_set_connected(ue, bs, (is_attacker)? n_bs-1 : n_bs); //create the connection of the UE with sBS -> sBS is the 2nd closest BS, so to always be in case of handover needed.
        
        //Print some info of the UE:
        //puts("-- UE creation OK");
        //printf("UE - location: (%d, %d)\n", ue->get_posX(), ue->get_posY());
        //printf("UE - connected: %d\n", ue->get_connected());
        //printf("UE - connected AMF: %d\n\n", ue->get_AMF());

        //Creation of the ATTACKER -> within ray of 150m w.r.t. UE location
        if(is_attacker){
            //Random selection of which legitimate BS attacker emulates -> random selection of BS_id value
            //This fake BS_id must not be the same as the one the UE is connected to for simulation
            int fake_id;
            for(fake_id; (fake_id=random_selection(1,13)) == ue->get_connected(); ) {}            
            
            bs[12]= new base_station(fake_id, random_selection(ue->get_posX()-150, ue->get_posX()+150), random_selection(ue->get_posY()-150, ue->get_posY()+150), handover_version, 1, 0, -1);
            
            //Print some info of the Attacker:
            //printf("-- ATTACKER creation OK\n");
            //printf("ATTACKER - location: (%d, %d)\n", bs[12]->get_posX(), bs[12]->get_posY());
            //printf("ATACKER - fake ID: %d\n\n", fake_id);
        }

        /*
            This represents the channel sensed by the user when it has to measure beacons from other BS.
            We avoid implementing the full and real beam sweeping and decoding process, thus we implement a high level concept of it.
            Column 0-> signal power received
            Column 1-> ID of corresponding BS
        */
        double channel[n_bs][2];

        /*
            This is the channel for simulating the transmission of messages.
            The data structure contain pointers to the exchanged message. Only one message can be in place at a time.
            Element msg[i] != 0 -> entity [i] has received a message.
            The correspondence is as follows:
                - position 0 -> ue
                - position 1 -> bs[0]
                - position 2 -> bs[1]
                - ....
                - position X -> ATTACKER
                - last position -1 -> AMF-1
                - last postion -> AMF-2
        */
        message* msg[n_bs+3];        
        for(int i=0; i<n_bs+3; ++i) msg[i]= NULL;   //declaration to NULL for protection of garbage values        
        
        
        //--------------------------------------- MEASUREMENT REPORT TRANSMISSION --------------------------------//

        transmit_beacons(channel, n_bs, ue, bs);    //populate @channel with the received power signals and corresponding BS-IDs
        int best_bs= ue->select_best_bs(channel, n_bs); //select the best BS and target the index
        
        // Print the target BS
        //printf("UE - target BS: %d\n", best_bs);
        
        if(ue->get_connected() != best_bs){
            //if best BS different from the currently connected, then need handover -> by the way of how simulation implmented, this is always true

            ue->set_target(best_bs);    //set the ID of the target BS
            ue->transmit_message(msg, "Measurement Report");    //transmit the measurement report message

            time.push_back(compute_delay(ue, bs[ue->get_connected()-1], 0));   //compute the transmission delay and save it. -1 because of the disallignement between BS_id and position in bs        


            //------------------------------------ HANDOVER PROCEDURE ---------------------------------------------//
            
            /*
                Defines the type of transmission = source - receiver. Used to compute the transmission delay:
                --------> NEED TO MODIFY AND GENERALIZE
                - 1 = ue -> BS
                - 2 = BS -> AMF
                - 3 = AMF <-> AMF
                - 4 = BS <-> BS
                - 5 = AMF -> BS
                - 6 = BS -> UE
            */            
            int type_transmission= 0;   //it is set within the message handling function
            
            int from= 0;    //contains the index in the msg data stucture of who is the transmitter of the transmitted message
            int to= 0;  //contains the index in the msg data stucture of who is the receiver of the transmitted message

            while(handover_completed == 0){ //loop until the handover is completed

                auto start= steady_clock::now();    //start the timer for comoputing the time for message handling

                //handle_message -> defines how the entity should handle the arrived message, transmitting the corresponding response.
                //the message is transmitted inside the function                                
                if(msg[0] != NULL){
                    ue->handle_message(msg, msg[0], channel, &handover_completed, &type_transmission);  //UE
                    from= 0;    //UE has received the message, so it will be handle and transmit it to someone -> save as transmitter

                }else if(msg[n_bs+1] != NULL){  //AMF-1
                    amf[0]->handle_message(msg, msg[n_bs+1], &type_transmission); 
                    from= n_bs+1;

                }else if(msg[n_bs+2] != NULL){  //AMF-2
                    amf[1]->handle_message(msg, msg[n_bs+2], &type_transmission);
                    from= n_bs+2;

                }else{   //BSs
                    for(int i=1; i<n_bs+1; ++i){    
                        if(msg[i] != NULL){                    
                            bs[i-1]->handle_message(msg, msg[i], &type_transmission);    //i-1 because of the different position in the two arrays
                            from= i;
                        }
                    }
                }                

                auto stop= steady_clock::now(); //stop the timer                
                auto handling_time = std::chrono::duration_cast<std::chrono::nanoseconds>(stop-start);  //time evaluation for message handling
                time.push_back(handling_time.count()*1e-9); //store the handling time

                // Print which entity has received the message -> channel[i] != NULL
                //for(int i=0; i<n_bs+3; ++i) printf("Channel[%d]: %p\n\n", i, msg[i]);

                for(int i=0; i<n_bs+3; ++i){
                    if(msg[i] != NULL)  to= i;  //save the reeiver
                }

                // Print some info about the transmition that happened
                //printf("Type transmission: %d\n", type_transmission);
                //printf("From: %d\nTo: %d\n", from, to);

                //Compute the transmission time
                switch(type_transmission){
                    case 1: //UE -> BS
                        time.push_back(compute_delay(ue, bs[to-1], 0));                        
                    break;

                    case 2: //BS -> AMF
                        time.push_back(compute_delay(bs[from-1], amf[to-n_bs-1], 1));                        
                    break;

                    case 3: //AMF <-> AMF
                        time.push_back(compute_delay(amf[0], amf[1], 1));                        
                    break;

                    case 4: //BS <-> BS
                        time.push_back(compute_delay(bs[from-1], bs[to-1], 1));                        
                    break;

                    case 5: //AMF -> BS
                        time.push_back(compute_delay(bs[to-1], bs[from-n_bs-1], 1));                        
                    break;

                    case 6: //BS -> ue
                        time.push_back(compute_delay(ue, bs[from-1], 0));                        
                    break;

                    default: ;
                    break;
                }

            } //#while(!handover_completed)

          
            //Handover simulation completed. Need to sum each saved time so to define the overall handover execution time
            /*
            // Print which of the possible scenario happened
            if(handover_completed==1)   printf("HANDOVER SUCCESSFUL\n\n");
            if(handover_completed==2)   printf("HANDOVER FAILED - RECONNECTION RECOVERY SUCCESSFUL\n\n");
            if(handover_completed==-1)  printf("HANDOVER FAILED - RECONNECTION RECOVERY REJECTED\n\n");
            if(handover_completed==-2)  printf("HANDOVER FAILED - RECONNECTION RECOVERY ABORTED\n\n");
            */

            //compute the overall run-time execution
            double sum= 0.0;
            for(int i=0; i<time.size(); ++i)    sum+= time[i];
            
            int sBS= ue->get_connected();   //ID of the BS to which UE was connected before handover 
            int tBS= ue->get_target();  //Id of the target BS -> in case of attack, this is the BS for reconnection

            if(is_attacker){   //if attack scenario
                if(overall_time3.size() < n_rounds && sBS == tBS)   overall_time3.push_back(sum);   //reconnection with sBS
                else if(overall_time2.size() < n_rounds && bs[sBS-1]->get_AMF() != bs[tBS-1]->get_AMF()) overall_time2.push_back(sum);   //reconnection in tBS !in sAMF
                else if(overall_time1.size() < n_rounds)    overall_time1.push_back(sum);   //reconnection in tBS in sAMF

            }else{  //if no attack scenario
                if( overall_time2.size() < n_rounds && bs[sBS-1]->get_AMF() != bs[tBS-1]->get_AMF() )   overall_time2.push_back(sum);   //tBS !in sAMF
                else if(overall_time1.size() < n_rounds)    {
                    overall_time1.push_back(sum);   //tBS in sAMF
                }               
            }

        } //#if(measurement report)


        //delete the entities for memory saving        
        delete ue;        
        for(int i=0; i< n_bs+3; ++i)  delete msg[i];
        for(int i=0; i<n_bs; ++i)   delete bs[i];
        for(int i=0; i<amf.size(); ++i) delete amf[i];        

    }   //#for(int j=0; j<n_rounds; ++j)


    //-------------------------------------TIME EXECUTION ANALYSIS ----------------------------------------//
/*
    // ARITMETIC AVERAGE
    double sum1= 0.0;
    for(int i=0; i<overall_time1.size(); ++i){
        sum1+= overall_time1[i];
        //fprintf(write1, "%.9f\n", overall_time1[i]);
    }
    printf("Scenario 1 - Aritmetic average execution time: %.9f\n", sum1/overall_time1.size());

    double sum2= 0.0;
    for(int i=0; i<overall_time2.size(); ++i){
        sum2+= overall_time2[i];
        //fprintf(write2, "%.9f\n", overall_time2[i]);
    }
    printf("Scenario 2 - Aritmetic average execution time: %.9f\n", sum2/overall_time2.size());
    
    if(is_attacker){
        double sum3= 0.0;
        for(int i=0; i<overall_time3.size(); ++i){
            sum3+= overall_time3[i];
            //fprintf(write3, "%.9f\n", overall_time3[i]);
        }
        printf("Scenario 3 - Aritmetic average execution time: %.9f\n", sum3/overall_time3.size());
    }
*/

    //Group the results according to the execution time and its occurrencies. Then order the times to compute the median

    //scenario 1
    vector <stat_t> statistic1;
    group(overall_time1, &statistic1);    //group
    sort(&statistic1);   //sort the values
    for(int i=0; i<statistic1.size(); ++i)  fprintf(write1, "%.9f;%d\n", statistic1[i].value, statistic1[i].occurences);

    //scenario 2
    vector <stat_t> statistic2;
    group(overall_time2, &statistic2);    //group
    sort(&statistic2);   //sort the values
    for(int i=0; i<statistic2.size(); ++i)  fprintf(write2, "%.9f;%d\n", statistic2[i].value, statistic2[i].occurences);
    
    //scenario 3
    vector <stat_t> statistic3;
    if(is_attacker){        
        group(overall_time3, &statistic3);    //group
        sort(&statistic3);   //sort the values
        for(int i=0; i<statistic3.size(); ++i)  fprintf(write3, "%.9f;%d\n", statistic3[i].value, statistic3[i].occurences);
    }
   
    printf("\n");
  
    // MEDIAN VALUE
    int mediumVal= (n_rounds-1)/2;
    
    //scenario 1
    double median1= 0.0;
    int count1= 0;
    for(int i=0; i<statistic1.size(); ++i){
        if(count1 + statistic1[i].occurences >= mediumVal){
            median1= statistic1[i].value;
            break;
        }
        count1+= statistic1[i].occurences;
    }
    printf("SCENARIO 1 - MEDIAN: %.9f\n", median1);

    //scenario 2
    double median2= 0.0;
    int count2= 0;
    for(int i=0; i<statistic2.size(); ++i){
        if(count2 + statistic2[i].occurences >= mediumVal){
            median2= statistic2[i].value;
            break;
        }
        count2+= statistic2[i].occurences;
    }
    printf("SCENARIO 2 - MEDIAN: %.9f\n", median2);

    if(is_attacker){
        //scenario 3
        double median3= 0.0;
        int count3= 0;
        for(int i=0; i<statistic3.size(); ++i){
            if(count3 + statistic3[i].occurences >= mediumVal){
                median3= statistic3[i].value;
                break;
            }
            count3+= statistic3[i].occurences;
        }
        printf("SCENARIO 3 - MEDIAN: %.9f\n", median3);
    }

    puts("CORRECTLY TERMINATED");
    return 0;

} //#main()