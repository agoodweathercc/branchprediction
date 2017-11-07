#ifndef _PREDICTOR_H_
#define _PREDICTOR_H_

#include "utils.h"
#include "tracer.h"

#define UINT16      unsigned short int
#define HIST_LEN   6
#define PERCEPTRON_HIST_LEN 7
#define DIM1 2^PERCEPTRON_HIST_LEN
#define DIM2 PERCEPTRON_HIST_LEN
#define DIM3 PERCEPTRON_HIST_LEN


// #define HIST_LEN   5
// #define DIM1 2^HIST_LEN
// #define DIM2 256


/////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////

class PREDICTOR{

  // The state is defined for Gshare, change for your design

 private:


  UINT32  ghr;           // global history register
  UINT32  *pht;          // pattern history table
  UINT32  historyLength; // history length
  UINT32  numPhtEntries; // entries in pht
  UINT32 dim1;
  UINT32 dim2;
  UINT32 dim3;
  //UINT32 dim1 = DIM1;
  // UINT32 dim2 = DIM2;
  // UINT32 dim3 = HIST_LEN;
  //UINT32*** w;
  //UINT32 w[2^7][16][5];
  UINT32 *GA;
  int output;
  int w[DIM1][DIM2][DIM3];



  //int ***w;

  //add for local predictor
  //local pattern history table
  //UINT32 pht_local_bit_size;
  UINT32 *pht_local;
  UINT32 numPhtLocalEntries;

  //branch history table for local branch predictor
  UINT32 bht_history_length;
  UINT32 numBhtEntries;
  UINT32 bht_bit_size;
  UINT16 *bht;

  //for tournament counter
  UINT32 *predictorChooseCounter;
  UINT32 numTournamentCounter;




 public:

  // The interface to the four functions below CAN NOT be changed

  PREDICTOR(void);
  bool    GetPrediction(UINT32 PC);

  //add for tournament predictor
  bool    GetLocalPrediction(UINT32 PC);
  bool    GetGlobalPrediction(UINT32 PC);
  bool    GetPerceptronPrediction(UINT32 PC);

  void    UpdatePredictor(UINT32 PC, bool resolveDir, bool predDir, UINT32 branchTarget);
  void    TrackOtherInst(UINT32 PC, OpType opType, UINT32 branchTarget);


  // Contestants can define their own functions below

};


/***********************************************************/
#endif