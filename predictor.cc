#include "predictor.h"


#define PHT_CTR_MAX  3
//chuan: for tournament predictor
#define TOURNAMENT_CTR_MAX 3
#define PHT_CTR_INIT 2

#define HIST_LEN   16
#define TOUR_LEN   15
#define BHT_BIT_SIZE 10
#define BHT_HIST_LENGTH 16
#define PHT_LOCAL_CTR_INIT 2
#define PHT_LOCAL_CTR_MAX  3
#define UINT16      unsigned short int

#define PERCEPTRON_HIST_LEN 12
#define PERCEPTRON_BIT_SIZE 8
// #define DIM1 pow(2, PERCEPTRON_HIST_LEN)
// #define DIM2 pow(2, PERCEPTRON_BIT_SIZE)
// #define DIM1 4096
// #define hex_dim1 0xFFF
// #define DIM1 32768
// #define hex_dim1 0x7FFF
#define DIM1 6144
#define hex_dim1 0x1FFFF

#define DIM2 37
#define hex_dim2 0xFF
// #define DIM2 512
// #define hex_dim2 0x1FF
// #define DIM2 2048
// #define hex_dim2 0x7FF


#define DIM3 59
// #define H1 511387
#define H1 511361
#define H2 660509
#define H3 1289381
#define num_of_weights 64000
#define num_of_bias 1000
#define weight_threshold 128
#define bias_weight_threshold 128
#define OUTPUT_THRESHOLD 25
#define THETA 250;

/////////////// STORAGE BUDGET JUSTIFICATION ////////////////
// Total storage budget: 52KB + 32 bits

// Total PHT counters for Global predictor: 2^16
// Total PHT size for global predictor = 2^16 * 2 bits/counter = 2^17 bits = 16KB
// GHR size for global predictor: 32 bits

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
  GA = new UINT32[DIM3];

 for(UINT32 l=0; l< DIM3; l++){
   GA[l]=0;
 }

   GHR = new int[DIM3];
   for (UINT32 i=0; i<DIM3; i++){
     GHR[i]=0;
   }

   W = new int[num_of_weights];
   for(UINT32 l=0; l< num_of_weights; l++){
     W[l]=0;
   }
   w1 = new int[num_of_bias];
   for (UINT16 l=0; l< num_of_bias; l++){
     w1[l]=0;
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

}

/////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////
//
// bool   PREDICTOR::GetPrediction(UINT32 PC){
//   // double sum = GetLocalPrediction(PC)+GetGlobalPrediction(PC)+1*GetPerceptronPrediction(PC);
//   double sum = GetPerceptronPrediction(PC)*3;
//   // double sum = 3*GetLocalPrediction(PC);
//   // double sum = 3* GetGlobalPrediction(PC);
//   return sig(sum/3.0);
//
// }
bool   PREDICTOR::GetPrediction(UINT32 PC){

  //Add for tournament predictor: when 00, 01, use global predictor; when 10, 11, use local predictor
  UINT32 pCC   = PC >> (32-TOUR_LEN);
  if (predictorChooseCounter[pCC] < 2) {
        //use global predictor
        // std::cout << "Global" << '\n';
      //  double sum = GetLocalPrediction(PC)+2*GetGlobalPrediction(PC)+1*GetPerceptronPrediction(PC);

      //  return sig(sum/4.0);
      GetPerceptronPrediction(PC);
      // std::cout << "using perceptron!" << '\n';
  } else {
      //use local predictor
      // std::cout << "Local" << '\n';
      GetPerceptronPrediction(PC);
      // double sum = 2*GetLocalPrediction(PC)+GetGlobalPrediction(PC)+GetPerceptronPrediction(PC);
      // return sig(sum/4.0);
  }
}

bool  PREDICTOR::GetPerceptronPrediction(UINT32 PC){
  //int DIM1 = 2^PERCEPTRON_HIST_LEN;
  // int hash_index = ((PC & (UINT32)(hex_dim1)) ^ 0 ^ 0) % (num_of_weights);
  int hash_index=1000;
  // int hash_index = ((UINT32)((PC % DIM1)*H1)^0^0) % (num_of_weights);
  // int index = (UINT32) ((PC^ghr) % num_of_bias);
  // int output;
  output = W[hash_index];
  // output = 0;
  // std::cout << "output is " << w1[index] << " at hash_index " << index << '\n';
  // output = w1[index];
output = 40;
  for (UINT32 i=0; i<DIM3; i++){
    // UINT32 address = hex_dim2 & GA[i];
    UINT32 address = (GA[i])%DIM2;
    // std::cout << "address is "<< address << '\n';
    // int hi = (PC & hex_dim1)*H1;
    int hi = (PC % DIM1)* H1;
    int hj = address * H2;
    int hk = i * H3;
    // int hash_index = (hi ^ hj ^ hk) % (num_of_weights);
    hash_index = ((UINT32)(hi) ^ (UINT32) (hj) ^(UINT32) (hk)) % (num_of_weights);
    // std::cout << "hash_index is " <<  hash_index << '\n';
    if (GHR[i]==1){
    // if ((ghr & (1<<(DIM3-i-1)))==1){
      // sleep(0.1);
      // output = output + w[(PC & hex_dim1) ][address][i];
      output = output + W[hash_index];
      // std::cout << "weight+ is " << W[hash_index]<< '\n';
      // std::cout << "output in + is " << output << '\n';
      // cout << "output+ in GetPerceptronPrediction is " << output << endl;
    } else{
      // output = output - w[(PC & hex_dim1) ][address][i];
      output = output - W[hash_index];
      // std::cout << "weight- is " << W[hash_index]<< '\n';
      // cout << "output- in GetPerceptronPrediction is " << output << endl;

    }
    // count(W, (double)(DIM1*DIM2*DIM3));
  }
  // cout << "output in GetPerceptronPrediction is " << output << endl;
  if (output >= OUTPUT_THRESHOLD){
  //  count(W, (double)(DIM1*DIM2*DIM3));
  //  std::cout << "PC" << PC << "is being TAKEN"<< '\n';
   return TAKEN;
  } else {
    // count(W, (double)(DIM1*DIM2*DIM3));
  //  std::cout << "PC" << PC << "is NOT_TAKEN"<< '\n';
   return NOT_TAKEN;
  }

}

//for global predictor
bool   PREDICTOR::GetGlobalPrediction(UINT32 PC){
    UINT32 phtIndex   = (PC^ghr) % (numPhtEntries);
    UINT32 phtCounter = pht[phtIndex];
    if(phtCounter > PHT_CTR_MAX/2){
        return TAKEN;
    }else{
        return NOT_TAKEN;
    }
}

//for local predictor
bool   PREDICTOR::GetLocalPrediction(UINT32 PC){
    UINT32 bhtIndex   = (PC >> (32-bht_bit_size));
    UINT16 bht_result = bht[bhtIndex];
    UINT32 pht_local_index = (PC^(UINT32)(bht_result))% (numPhtLocalEntries);

    if(pht_local[pht_local_index] > PHT_LOCAL_CTR_MAX/2){
        return TAKEN;
    }else{
        return NOT_TAKEN;
    }
}

/////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////
int PREDICTOR::sig(double val){
  return (val>0)-(val<0);
}

void  PREDICTOR::UpdatePredictor(UINT32 PC, bool resolveDir, bool predDir, UINT32 branchTarget){
  theta = THETA;
  int result;
  if (resolveDir==TAKEN){
    result = 1;
  } else{
    result = -1;
  }

  // cout << "output in update function is " << output << endl;

  // int hash_index = ((UINT32)(PC & hex_dim1) ^ 0 ^ 0) % (num_of_weights);
  // int hash_index = ((UINT32)((PC % DIM1)*H1) ^ 0 ^ 0) % (num_of_weights);
  int index = ((UINT32)(PC^ghr) % num_of_bias);
  // std::cout << "hash_index is " << hash_index << '\n';
  if (sig(output)!=result || (abs(output) < theta)){
    if (result == 1){

    // w[(PC & hex_dim1) ][0][0] = min(w[(PC & hex_dim1) ][0][0]+1, 127.0);
    //  W[hash_index] = min(W[hash_index]+1*result, weight_threshold);
    // W[hash_index] = W[hash_index];
    // w1[index] = min(w1[index], bias_weight_threshold);
    W[0] = min(W[0]+1, 64);
    //  std::cout << "Weight+ is " << w1[index] << "at index" <<  index << '\n';
    } else{
      // W[hash_index] = max(W[hash_index]-1*result, -weight_threshold);
      // W[hash_index] = W[hash_index];
      // w1[index] = max(w1[index], -bias_weight_threshold);
      W[0] = max(W[0]-1, -63);
      // std::cout << "Weight- is " << w1[index] << '\n';

    // w[(PC & hex_dim1) ][0][0] = max(w[(PC & hex_dim1) ][0][0]-1, -128.0);
    // cout << "1: resolverDir NOT taken!" << endl;
    }

    //address = 1;
    for(int i=0; i< DIM3; i++){
      // UINT32 address = (hex_dim2 & GA[i]) ;
      UINT32 address = (GA[i])%DIM2;
      // std::cout << "address is " << address << '\n';
      // int hi = (PC & hex_dim1)*H1;
      int hi =(PC % DIM1)*H1;
      int hj = address * H2;
      int hk = i * H3;
      int hash_index = ((UINT32)(hi) ^ (UINT32) (hj) ^(UINT32) (hk)) % (num_of_weights);
      if ((GHR[i]==1)){
      // if ((ghr & (UINT32)(1<<(DIM3-i-1)))== 1){
      // w[(PC & hex_dim1) ][address][i] = min(w[(PC & hex_dim1) ][address][i], 127.0);
      // std::cout << "Weightt+ is " << W[hash_index] << '\n';
      // std::cout << "Weight+ is " << W[hash_index] << "at hash_index" <<  hash_index << '\n';
      W[hash_index] = max(min(W[hash_index]+1*result,weight_threshold), -weight_threshold);

      } else{
      // w[(PC & hex_dim1) ][address][i] = max(w[(PC & hex_dim1) ][address][i], -128.0);
      W[hash_index] = min(max(W[hash_index]-1*result, -weight_threshold), weight_threshold);
      // std::cout << "Weight- is " << W[hash_index] << '\n';

      }
    }
  }


  int GA_temp[DIM3];
  for(int i=1; i <DIM3; i++){
    GA_temp[i]=GA[i-1];
  }
  // UINT32 address = (PC & hex_dim1) ;
  // UINT32 address = PC % DIM1;
  UINT32 address = PC;
  // cout << "address is " << address << endl;
  GA_temp[0]=address%DIM2;
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

  UINT32 phtIndex   = (PC^ghr) % (numPhtEntries);
  UINT32 phtCounter = pht[phtIndex];

  // update the PHT for global predictor
  if(resolveDir == TAKEN){
    pht[phtIndex] = SatIncrement(phtCounter, PHT_CTR_MAX);
  }else{
    pht[phtIndex] = SatDecrement(phtCounter);
  }

  // update the GHR for global predictor
  ghr = (ghr << 1);
  // std::cout << "ghr now is " << ghr << '\n';
  if(resolveDir == TAKEN){
    ghr++;
  }

  // update the tournament counter
  bool global_pred_result = GetGlobalPrediction(PC);
  bool local_pred_result = GetLocalPrediction(PC);
  UINT32 pCC   = PC >> (32-TOUR_LEN);
  //currently global predictor is in using
  if (predictorChooseCounter[pCC] < (TOURNAMENT_CTR_MAX/2 + 1)) {
        //if global predictor predicts not correct and local predictor predicts correct, will add 1
        if (global_pred_result != predDir && local_pred_result == predDir)
        predictorChooseCounter[pCC]++;
        // std::cout << "p is right" << '\n';
        if (global_pred_result == predDir && local_pred_result != predDir) {
            // std::cout << "percetron is right" << '\n';
            if (predictorChooseCounter[pCC] >0) predictorChooseCounter[pCC]--;
        }
  } else {
      //currently local predictor is in using
      if (local_pred_result != predDir &&  global_pred_result == predDir)
      {
        // std::cout << "perceptron is ritaoght" << '\n';
        predictorChooseCounter[pCC]--;
      }
      if (global_pred_result != predDir && local_pred_result == predDir) {


        // std::cout << "p is right" << '\n';
        if (predictorChooseCounter[pCC] < TOURNAMENT_CTR_MAX) predictorChooseCounter[pCC]++;
      }
  }

  //update the BHT and PHT for local branch predictor
  //update the PHT_LOCAL
  UINT32 bhtIndex   = (PC >> (32-bht_bit_size));
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

  return;
}

/////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////
