# BARON_simulation

This directory containes the files for a simplified software simulation to evaluate the performance of the BARON defence methodology in the 5G Networks framework to defend against Fake Base Station (FBS) attacks.
The simulation if fully written in C++ language but requires also C standard library.

The performance evaluation is considered in the handover scenario.
Three possible cases are implemented:
  1- Standard handover with no BARON in place
  2- Handover using BARON & no FBS attack in place
  3- Handover using BARON & FBS attack runnin -> in case of attack success => BARON connection recovery mechanism is activated
The simulation does not implement all the specifications of the 3GPP standard handover, but only those that are enough and necessary for BARON overhead performance evaluation.

SIMULATION SCENARIO:\\
In the simulation we consider a 2-dimensional plane with coordinates (𝑥, 𝑦) and located two AMFs, with each controlling 6 BSs.\\
The User (UE) is randomly placed in the 2-dimensional plane at the start of each simulation, and we assume that it has a connection to a legitimate Base-Station (BS)  at the start of the run.\\
We choose the serving BS (sBS) to be the second-nearest BS to the UE. As a result, the nearest BS will be selected as the target BS (tBS) for handover.\\
The received signal power (𝑃𝑅) from base station 𝑖 ($BS_𝑖$) is modelled according to standard signal power propagation:

$$ 𝑃𝑅_𝑖 = {PT_i \over d\left(UE, BS_i\right)^2}$$

where 𝑑(𝑥, 𝑦) is the distance between 𝑥 and 𝑦, and 𝑃𝑅𝑖 is the transmission power of $BS_𝑖$.\\
In the presence of an attacker carrying out a FBS attack, we place the rogue BS (rBS) within a range of 150𝑚 from the UE’s position.\\
The rBS uses a BS identifier assigned at random, but different from that of the sBS.\\
We additionally ensure that the rBS has a higher transmission power in order to maximize the probability of coming under an FBS attack scenario.\\
