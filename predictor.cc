#include "predictor.h"
#include "help.h"
#include <algorithm>    // std::max
#include <fstream>
#include <iostream>
#include <cmath>
#include <unistd.h>

#define PHT_CTR_MAX  3
//chuan: for tournament predictor
#define TOURNAMENT_CTR_MAX 3
#define PHT_CTR_INIT 2

// #define HIST_LEN   6
// int HIST_LEN;
// int theta;
#define UINT16   unsigned short int

#define PERCEPTRON_HIST_LEN 12
#define PERCEPTRON_BIT_SIZE 8
#define OUTPUT_THRESHOLD 4
// #define DIM1 pow(2, PERCEPTRON_HIST_LEN)
// #define DIM2 pow(2, PERCEPTRON_BIT_SIZE)
// #define DIM1 4096
// #define hex_dim1 0xFFF
#define DIM1 2048
#define hex_dim1 0x7FF

// #define DIM2 256
// #define hex_dim2 0xFF
#define DIM2 512
#define hex_dim2 0x1FF
// #define DIM2 2048
// #define hex_dim2 0x7FF


#define DIM3 80
// #define H1 511387
#define H1 511361
#define H2 660509
#define H3 1289381
#define num_of_weights 32000
#define weight_threshold 128
#define THETA 200
/////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////

PREDICTOR::PREDICTOR(void){
  // ifstream fin;
  // // int HIST_LEN;
  // fin.open("intList1.txt", ios::in);
  // if (!fin.is_open()){
  //   std::cerr << "unable to open file intList1.txt" << '\n';
  //   exit(10);
  // }
  //
  // fin >> HIST_LEN;
  // std::cout << "HIST_LEN is " << HIST_LEN << '\n';
  // fin.close();

  historyLength    = HIST_LEN;
  ghr              = 0;
  numPhtEntries    = (1<< HIST_LEN);
  GA = new int[DIM3];

  for(UINT32 l=0; l< DIM3; l++){
    GA[l]=0;
  }

  // Initialization for percetron predictor
  w = new double **[DIM1];
  // std::cout << "first level w" << '\n';
  for (UINT32 i =0; i< DIM1; i++){
    w[i] = new double*[DIM2];
    // std::cout << "second level w" << '\n';
    for(UINT32 j=0; j<DIM2; j++){
      w[i][j] = new double[DIM3];
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
      GHR[i]=0;
    }

    W = new int[DIM1*DIM2*DIM3];
    for(UINT32 l=0; l< DIM1*DIM2*DIM3; l++){
      W[l]=0;
    }

}

/////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////

bool PREDICTOR::GetPrediction(UINT32 PC){
  GetPerceptronPrediction(PC);
}


bool PREDICTOR::GetPerceptronPrediction(UINT32 PC){
  //int DIM1 = 2^PERCEPTRON_HIST_LEN;
  int hash_index = ((PC & (UINT32)(hex_dim1)) ^ 0 ^ 0) % (num_of_weights);
  int output = 0;
  output = W[hash_index];
  for (UINT32 i=0; i<DIM3; i++){
    UINT32 address = hex_dim2 & GA[i];
    int hi = (PC & hex_dim1)*H1;
    int hj = address * H2;
    int hk = i * H3;
    // int hash_index = (hi ^ hj ^ hk) % (num_of_weights);
    hash_index = ((UINT32)(hi) ^ (UINT32) (hj) ^(UINT32) (hk)) % (num_of_weights);
    // std::cout << "hash_index is " <<  hash_index << '\n';
    if (GHR[i]==1){
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
  // // cout << "output in GetPerceptronPrediction is " << output << endl;
  //   for (int i=0; i<DIM3; i++){
  //     std::cout << "GHR " << i << "is " << GHR[i] << '\n';
  //   }


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


/////////////////////////////////////////////////////////////
int PREDICTOR::sig(double val){
  return (val>0)-(val<0);
}

void  PREDICTOR::UpdatePredictor(UINT32 PC, bool resolveDir, bool predDir, UINT32 branchTarget){

  // ifstream fin;
  // // int num_of_weights;
  // // int HIST_LEN;
  // fin.open("intList1.txt", ios::in);
  // if (!fin.is_open()){
  //   std::cerr << "unable to open file intList1.txt" << '\n';
  //   exit(10);
  // }

  // fin >> num_of_weights;
  // num_of_weights = 10000;
  // std::cout << "num_of_weights is now" << num_of_weights << '\n';
  // fin.close();

  //update perceptron predictor result
  theta = THETA;
  int result;
  if (resolveDir==TAKEN){
    result = 1;
  } else{
    result = -1;
  }

  // cout << "output in update function is " << output << endl;

  int hash_index = ((UINT32)(PC & hex_dim1) ^ 0 ^ 0) % (num_of_weights);
  // std::cout << "hash_index is " << hash_index << '\n';
  if (sig(output)!=result || (abs(output) < theta)){
    if (result == 1){

    // w[(PC & hex_dim1) ][0][0] = min(w[(PC & hex_dim1) ][0][0]+1, 127.0);
     W[hash_index] = min(W[hash_index]+1*result, weight_threshold);
    } else{
      W[hash_index] = max(W[hash_index]-1*result, -weight_threshold);
    // w[(PC & hex_dim1) ][0][0] = max(w[(PC & hex_dim1) ][0][0]-1, -128.0);
    // cout << "1: resolverDir NOT taken!" << endl;
    }

    //address = 1;
    for(int i=0; i< DIM3; i++){
      UINT32 address = (hex_dim2 & GA[i]) ;
      int hi = (PC & hex_dim1)*H1;
      int hj = address * H2;
      int hk = i * H3;
      int hash_index = ((UINT32)(hi) ^ (UINT32) (hj) ^(UINT32) (hk)) % (num_of_weights);
      if ((GHR[i]==1)){
      // w[(PC & hex_dim1) ][address][i] = min(w[(PC & hex_dim1) ][address][i], 127.0);
      W[hash_index] = max(min(W[hash_index]+1*result,weight_threshold), -weight_threshold);
      // std::cout << "Weight+ is " << W[hash_index] << '\n';

      } else{
      // w[(PC & hex_dim1) ][address][i] = max(w[(PC & hex_dim1) ][address][i], -128.0);
      W[hash_index] = min(max(W[hash_index]-1*result, -weight_threshold), weight_threshold);
      // std::cout << "Weight- is " << W[hash_index] << '\n';

      }
    }
  }


  int GA_temp[DIM2];
  for(int i=1; i <DIM3; i++){
    GA_temp[i]=GA[i-1];
  }
  UINT32 address = (PC & hex_dim1) ;
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

}

/////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////

void    PREDICTOR::TrackOtherInst(UINT32 PC, OpType opType, UINT32 branchTarget){
  return;
}
