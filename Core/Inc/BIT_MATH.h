/***********************************************************************************************/
/***********************************************************************************************/
/********************************** Author: Mahmoud Abdelmoneim*********************************/
/***************************************** Version: 1.0 ****************************************/
/***********************************************************************************************/
/***********************************************************************************************/
#ifndef BIT_MATH_H
#define BIT_MATH_H


#define SET_BET(VAR,BET)       VAR |= (1 << (BET))
#define CLR_BET(VAR,BET)       VAR &= ~(1 << (BET))
#define GET_BET(VAR,BET)       ((VAR >> BET) & 1)
#define TOG_BET(VAR,BET)       VAR ^= (1 << (BET))


#endif