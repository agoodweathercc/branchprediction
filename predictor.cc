#include "predictor.h"
#include <algorithm>    // std::max
#include <cmath>
#define PHT_CTR_MAX  3
//chuan: for tournament predictor
#define TOURNAMENT_CTR_MAX 3
#define PHT_CTR_INIT 2

#define HIST_LEN   6
#define TOUR_LEN   10
#define BHT_BIT_SIZE 11
#define BHT_HIST_LENGTH 12
#define PHT_LOCAL_CTR_INIT 2
#define PHT_LOCAL_CTR_MAX  3
#define UINT16   unsigned short int

#define PERCEPTRON_HIST_LEN 12
#define PERCEPTRON_BIT_SIZE 8
#define OUTPUT_THRESHOLD 3
// #define DIM1 pow(2, PERCEPTRON_HIST_LEN)
// #define DIM2 pow(2, PERCEPTRON_BIT_SIZE)
#define DIM1 1024
#define DIM2 256
#define DIM3 10
#define H 1289281

/////////////// STORAGE BUDGET JUSTIFICATION ////////////////
// Total storage budget: 52KB + 32 bits

// Total PHT size for global predictor = 2^16 * 2 bits/counter = 2^17 bits = 16KB
// GHR size for global predictor: 32 bits

// Total PHT counters for Global predictor: 2^16
// Total PHT counters for local predictor: 2^16
// Total PHT size for local predictor = 2^16 * 2 bits/counter = 2^17 bits = 16KB
// Total BHT size for local predictor = 2^11 * 16 bits/counter = 2^15 bits = 4KB
// Total Size for local predictor = 16KB + 4KB = 20KB

// Total Tournament counters is: 2^16
// Total Tournament counter's size = 2^16 * 2 bits/counter = 2^17 bits = 16KB
/////////////////////////////////////////////////////////////



/////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////

PREDICTOR::PREDICTOR(void){

  historyLength    = HIST_LEN;
  ghr              = 0;
  numPhtEntries    = (1<< HIST_LEN);
  pht = new UINT32[numPhtEntries];
  GA = new int[DIM3];

  for(UINT32 l=0; l< DIM3; l++){
    GA[l]=128;
  }

  for(UINT32 ii=0; ii< numPhtEntries; ii++){
    pht[ii]=PHT_CTR_INIT;
  }

  //when 00, 01, use global predictor; when 10, 11, use local predictor
  numTournamentCounter = (1<<TOUR_LEN);
  predictorChooseCounter = new UINT32[numTournamentCounter];
  for(UINT32 jj=0; jj< numTournamentCounter; jj++){
    predictorChooseCounter[jj] = 0;
  }

  //Initialization for local branch predictor
  bht_history_length = BHT_HIST_LENGTH;
  bht_bit_size = BHT_BIT_SIZE;
  numBhtEntries    = (1<< bht_bit_size);
  bht = new UINT16[numBhtEntries];
  for(UINT32 kk=0; kk< numBhtEntries; kk++){
    bht[kk]=0;
  }

  numPhtLocalEntries = (1<<bht_history_length);
  pht_local = new UINT32[numPhtLocalEntries];
  for(UINT32 ll=0; ll< numPhtLocalEntries; ll++){
    pht_local[ll]=PHT_LOCAL_CTR_INIT;
  }

  //int dim1 = numPhtEntries;
  dim1 = DIM1;
  dim2 = DIM2;
  dim3 = DIM3;

  // Initialization for percetron predictor
  w = new int **[DIM1];
  // std::cout << "first level w" << '\n';
  for (UINT32 i =0; i< DIM1; i++){
    w[i] = new int*[DIM2];
    // std::cout << "second level w" << '\n';
    for(UINT32 j=0; j<DIM2; j++){
      w[i][j] = new int[DIM3];
        }
   }

   for (UINT32 i =0; i<DIM1; i++){
     for(UINT32 j=0; j<DIM2; j++){
       for(UINT32 k=0; k<DIM3; k++){
         w[i][j][k]=0;
       }
     }
    }

    GHR = new int[DIM3];
    for (UINT32 i=0; i<DIM3; i++){
      GHR[i]=1;
    }

}

/////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////

bool PREDICTOR::GetPrediction(UINT32 PC){
  //Add for tournament predictor: when 00, 01, use global predictor; when 10, 11, use local predictor
  UINT32 pCC   = PC >> (32-TOUR_LEN);
  //cout << "PC Is " << PC << endl;
  if (predictorChooseCounter[pCC] < 2) {
        //use global predictor
      //  GetLocalPrediction(PC);
       GetPerceptronPrediction(PC);
       //std::cout << "use perceptron" << '\n';
  } else {
      //use local predictor
      // GetPerceptronPrediction(PC);
      //std::cout << "use local" << '\n';
      GetLocalPrediction(PC);
  }
//return NOT_TAKEN;

}


bool PREDICTOR::GetPerceptronPrediction(UINT32 PC){
  //int DIM1 = 2^PERCEPTRON_HIST_LEN;
  output = w[(PC & 0x3FF) ][0][0];
  // std::cout << "output is " << output << '\n';
  for (UINT32 i=0; i<DIM3; i++){
    // UINT32 bitStatus = (ghr >> ) & 1;
    UINT32 bitStatus = (GHR[i]==1);
    // cout << "bitStatus is " << bitStatus << endl;
    UINT32 address = 0xff & GA[i];
    //cout << "GA_I is " << GA[i] << endl;
    // cout << "address is " << address << endl;
    //UINT32 address = 3;
    if (bitStatus == 1){
      output = output + w[(PC & 0x3FF) ][address][i];
      // std::cout << "output is " << output << '\n';
    } else{
      output = output - w[(PC & 0x3FF) ][address][i];
      // std::cout << "output is " << output << '\n';
    }
  }
  // cout << "output is " << output << endl;
  if (output >= OUTPUT_THRESHOLD){
   return TAKEN;
   output = 1;
  } else {
    output = -1;
   return NOT_TAKEN;
  }
}

//for global predictor
//bool   PREDICTOR::GetGlobalPrediction(UINT32 PC){
//     UINT32 phtIndex   = (PC^ghr) % (numPhtEntries);
//     UINT32 phtCounter = pht[phtIndex];
//     //printf("%s\n", PC);
//     if(phtCounter > PHT_CTR_MAX/2){
//         return TAKEN;
//     }else{
//         return NOT_TAKEN;
//     }
// }

//for local predictor
bool   PREDICTOR::GetLocalPrediction(UINT32 PC){
    UINT32 bhtIndex   = (PC >> (32-bht_bit_size));
    UINT16 bht_result = bht[bhtIndex];
    //cout << "bhtIndex is " << bht_result << endl;
    UINT32 pht_local_index = (PC^(UINT32)(bht_result))% (numPhtLocalEntries);
    //std::cout << "pht local index is " << pht_local_index << '\n';

    if(pht_local[pht_local_index] > PHT_LOCAL_CTR_MAX/2){
        return TAKEN;
    }else{
        return NOT_TAKEN;
    }
}

/////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////
int PREDICTOR::sig(int val){

  return (val>0)-(val<0);
}
void  PREDICTOR::UpdatePredictor(UINT32 PC, bool resolveDir, bool predDir, UINT32 branchTarget){
  // std::cout << "sig(-2) is " << sig(-2) << '\n';


  // UINT32 phtIndex   = (PC^ghr) % (numPhtEntries);
  // UINT32 phtCounter = pht[phtIndex];

  // update the PHT for global predictor
  // if(resolveDir == TAKEN){
  //   pht[phtIndex] = SatIncrement(phtCounter, PHT_CTR_MAX);
  // }
  // else{
  //   pht[phtIndex] = SatDecrement(phtCounter);
  // }

  // update the GHR for global predictor
  // ghr = (ghr << 1);
  //
  // if(resolveDir == TAKEN){
  //   ghr++;
  // }


  // update the tournament counter
  //bool perceptron_pred_result = GetPerceptronPrediction(PC);
  //bool global_pred_result = GetGlobalPrediction(PC);
  bool global_pred_result = GetPerceptronPrediction(PC);
  bool local_pred_result = GetLocalPrediction(PC);
  UINT32 pCC   = PC >> (32-TOUR_LEN);
  //currently global predictor is in using
  if (predictorChooseCounter[pCC] < (TOURNAMENT_CTR_MAX/2 + 1)) {
        //if global predictor predicts not correct and local predictor predicts correct, will add 1
        if (global_pred_result != predDir && local_pred_result == predDir) predictorChooseCounter[pCC]++;
        if (global_pred_result == predDir && local_pred_result != predDir) {
            if (predictorChooseCounter[pCC] >0) predictorChooseCounter[pCC]--;
        }
  }else {
      //currently local predictor is in using
      if (local_pred_result != predDir &&  global_pred_result == predDir) predictorChooseCounter[pCC]--;
      if (global_pred_result != predDir && local_pred_result == predDir) {
        if (predictorChooseCounter[pCC] < TOURNAMENT_CTR_MAX) predictorChooseCounter[pCC]++;
      }
  }


  //update perceptron predictor result
  //void train(UINT32 PC, bool resolveDir, int output, int & w[][][]);
  //void train(UINT32 PC, bool resolveDir, int output, int   w[dim1][dim2][dim3]){
  //int output = 0;
  int theta =70;
  // int dim1 = 10
  int result;
  if (resolveDir==TAKEN){
    result = 1;
  } else{
    result = -1;
  }

  //output = 1;
  if (sig(output)!=result || (output < theta)){
    if (resolveDir == TAKEN){
    w[(PC & 0x3FF) ][0][0] = min(w[(PC & 0x3FF) ][0][0] + 1, 127);

    } else{
    w[(PC & 0x3FF) ][0][0] = max(w[(PC & 0x3FF) ][0][0] - 1, -128);
    }

    //UINT32 address = ((1<<8) & GA[i]) % 256;
    //address = 1;
    for(int i=0; i< DIM3; i++){
      UINT32 address = (0xff & GA[i]) % 256;
      // cout << "address is " << address << endl;
      //UINT32 address = 1;
      //if (((ghr >> i) & 1)==1){
      if ((GHR[i]==1)){
        // if (1==1){
      w[(PC & 0x3FF) ][address][i] = min(w[(PC & 0x3FF) ][address][i]+1, 127);
      // cout << "w at " << PC & 0x3FF << address << i << " is "
      // cout << w[(PC & 0x3FF)][address][i] <<endl;
      } else{
      w[(PC & 0x3FF) ][address][i] = max(w[(PC & 0x3FF) ][address][i]-1, -128);
      }
    }
  }


  int GA_temp[DIM2];
  for(int i=1; i <DIM3; i++){
    GA_temp[i]=GA[i-1];
  }
  UINT32 address = (PC & 0x3FF) ;
  // cout << "address is " << address << endl;
  GA_temp[0]=address;
  for (int i=0; i<DIM3; i++){
  GA[i] = GA_temp[i];
  // cout <<"GA " << i << "is" << GA[i] << endl;
}

  // std::cout << "ghr is " << ghr << '\n';
  for (int i=DIM3-1; i>0; i--){
    GHR[i] = GHR[i-1];
  }
  if (resolveDir == TAKEN){
    GHR[0] = 1;
  } else{
    GHR[0] = -1;
  }

  // ghr = (ghr << 1);
  // if(resolveDir == TAKEN){
  //   ghr++;
  // std::cout << "ghr is " << ghr << '\n';
  // std::cout << "ghr is " << ghr << '\n';


  //update the BHT and PHT for local branch predictor
  //update the PHT_LOCAL
  UINT32 bhtIndex   = (PC >> (32-bht_bit_size));
  // cout << "bhtIndex is " << bhtIndex << endl;
  UINT16 bht_result = bht[bhtIndex];
  UINT32 pht_local_index = (PC^(UINT32)(bht_result))% (numPhtLocalEntries);
  UINT32 pht_local_counter = pht_local[pht_local_index];
  if(resolveDir == TAKEN){
    pht_local[pht_local_index] = SatIncrement(pht_local_counter, PHT_LOCAL_CTR_MAX);
  }else{
    pht_local[pht_local_index] = SatDecrement(pht_local_counter);
  }

  //update the bht for local predictor
  bht[bhtIndex] = (bht[bhtIndex] << 1);
  if(resolveDir == TAKEN){
    bht[bhtIndex]++;
  }
}

/////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////

void    PREDICTOR::TrackOtherInst(UINT32 PC, OpType opType, UINT32 branchTarget){

  // This function is called for instructions which are not
  // conditional branches, just in case someone decides to desig
  // a predictor that uses information from such instructions.
  // We expect most contestants to leave this function untouched.

  return;
}

/////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////
