#ifndef SRC_CHANNELS_H_
#define SRC_CHANNELS_H_

/*---------------------------------------------------
 *                      Channels
 * --------------------------------------------------
 */

//-------------------MOTORS------------------
const static int CH_LEFTDRIVE = 2;//PWM *2
const static int CH_RIGHTDRIVE = 1;//PWM *2

const static int CH_SHOOTERA = 1;//CAN
const static int CH_SHOOTERB = 0;//CAN

const static int CH_ANGLEMOTOR = 2;//CAN

const static int CH_ARMMAIN = 3;//CAN
const static int CH_ARMSECONDARY = 3;//PWM

const static int CH_HANGA = 4;//CAN?
const static int CH_HANGB = 5;//CAN?

//-------------------LIMIT SWITCHES/SENSORS------------
const static int CH_SHOOTER_ANGLE_BOTTOM = 0;
const static int CH_SHOOTER_ANGLE_TOP = 1;

const static int CH_ENC_L_A = 2;
const static int CH_ENC_L_B = 3;
const static int CH_ENC_R_A = 4;
const static int CH_ENC_R_B = 5;

//--------------------HUMAN INPUT--------------
const static int CH_DRIVESTICK = 0;
const static int CH_SPECIALSTICK = 1;

//--------------------MISC---------------------
const static int CH_PDP = 0;

//-------------------PNEUMATICS------------------
const static int CH_PCMA = 0;
const static int CH_PCMB = 1;

const static int CH_SHIFTER_PCM = CH_PCMA;
const static int CH_SHIFTER_FW = 0;
const static int CH_SHIFTER_RV = 1;

const static int CH_FORKA_PCM = CH_PCMA;
const static int CH_FORKB_PCM = CH_PCMA;
const static int CH_FORK_FW = 2;
const static int CH_FORK_RV = 3;

const static int CH_SHOOTSTICK_PCM = CH_PCMA;
const static int CH_SHOOTSTICK_FW = 6;
const static int CH_SHOOTSTICK_RV = 7;


/* ----------------------------------------------------------------
 *                          Buttons
 * ----------------------------------------------------------------
 */

//------------------Driver Stick-------------------------
const static int BUT_SHIFTER = 1;
const static int BUT_BREACH2 = 7;
const static int BUT_BREACH3 = 8;
const static int BUT_BREACH4 = 9;
const static int BUT_BREACH5 = 10;

const static int BUT_NAVX_RESET = 4;
const static int BUT_ENCODERS_RESET = 6;

//-----------------Specials Stick-------------------------
const static int BUT_FIRE = 1;
const static int BUT_SHOOTER_OUT = 3;
const static int BUT_SHOOTER_IN = 2;
const static int BUT_AUTOAIMA = 4;
const static int BUT_AUTOAIMB = 5;

const static int BUT_ARMMAIN_FW = 6;
const static int BUT_ARMMAIN_RV = 7;
const static int BUT_ARMSEC_FW = 11;
const static int BUT_ARMSEC_RV = 10;

const static int BUT_FEELERS = 8;

#endif
