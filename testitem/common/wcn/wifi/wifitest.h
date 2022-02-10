// 
// Spreadtrum Auto Tester
//
// anli   2012-11-10
//
#ifndef _WIFITEST_20121110_H__
#define _WIFITEST_20121110_H__

//-----------------------------------------------------------------------------
#ifdef __cplusplus
extern "C" {
#endif // __cplusplus
//--namespace sci_wifi {
//-----------------------------------------------------------------------------
    
int wifiOpen( void );

int wifi_ScanAP( void );

int wifiClose( void );

//-----------------------------------------------------------------------------
//--};
#ifdef __cplusplus
}
#endif // __cplusplus
//-----------------------------------------------------------------------------

#endif // _WIFI_20121110_H__
