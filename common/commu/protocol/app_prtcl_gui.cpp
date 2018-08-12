#include <stdlib.h>
#include <string.h>

//#include "../../device/device.h"
#include "app_prtcl_gui.h"
#include "phy_prtcl_pqb.h"

/*!
Initailize head to default value
*/
void AppPrtclGui::IniHead(CommDataHead *head)
{
    head->prtcl_type = 3;
    head->prtcl_ver = 0;
    head->compress = 0;
    head->compress_alg = 1;
}

