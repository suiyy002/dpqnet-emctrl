#ifndef _PRTCL_INTERFACE_H_
#define _PRTCL_INTERFACE_H_

#include "app_prtcl_gps.h"
#include "app_prtcl_pqb.h"
#include "app_prtcl_guic.h"
#include "app_prtcl_guis.h"

#include "phy_prtcl_gps.h"
#include "phy_prtcl_pqb.h"
#include "phy_prtcl_modbus.h"

enum kAppPrtclType { kAppPrtclPqB, kAppPrtclGps, kAppPrtclGuiS, kAppPrtclGuiC};
enum kPhyPrtclType { kPhyPrtclPqB, kPhyPrtclGps, kPhyPrtclModbus};

/*!
Creat physical layer protocal object
*/
inline PhyPrtclBase *CreatePhyPrtcl( kPhyPrtclType type)
{
	PhyPrtclBase * prtcl;
	switch (type) {
	    case kPhyPrtclPqB:
	        prtcl = new PhyPrtclPqB;
	        break;
	    case kPhyPrtclGps:
	        //prtcl = new PhyPrtclGps;
	        break;
	    case kPhyPrtclModbus:
	        //prtcl = new PhyPrtclModbus;
	        break;
	    default:
	        return NULL;
	}
	return prtcl;
};

/*!
Creat application layer protocal object

    Input:  type -- protocol type
            idx -- index of communication object
*/
inline AppPrtclBase *CreateAppPrtcl( kAppPrtclType type, int idx)
{
	AppPrtclBase * prtcl;
	switch (type) {
	    case kAppPrtclPqB:
	        //prtcl = new AppPrtclPqB;
	        break;
	    case kAppPrtclGps:
	        //prtcl = new AppPrtclGps;
	        break;
	    case kAppPrtclGuiC:
	        prtcl = new AppPrtclGuiC(idx);
	        break;
	    case kAppPrtclGuiS:
	        prtcl = new AppPrtclGuiS(idx);
	        break;
	    default:
	        return NULL;
	}
	return prtcl;
};

#endif // _PRTCL_INTERFACE_H_ 
