#ifndef _USER_MANAGE_
#define _USER_MANAGE_

#include <stdint.h>
#include "loopbuf_sort.h"

struct UserInfo {
    char name[32];      //Username
    char pswd_local[36];    //Local password 
    char pswd_remote[36];   //Remote password 
    uint32_t permit;    //Access permission. not use
};

struct AuditEvntInf { char str[64]; };

class UserManage{
public:
	UserManage();
	~UserManage();

    int CheckPasswd(char *pswd, int id=1, int type=0);
    void GetPasswd(char *pswd, int id);
    void SetPasswd(char *pswd);
    void SetPasswd(char *pswd, int id, int type);
    void SavePasswd();
    int Login61850(uint32_t *pswd, int td);
    int AddUser(char *usr, char *pswdl, char *pswd);
    int DelUser(char *usr);
    void SetAudit(char *type, char *usr, char *detail, int result);
    void SaveAudit(char *inf);
    int FindUser(char *usr);
    int GetWarnEvt(int max, AuditEvntInf *info);
    int GetAlarmEvt(int max, AuditEvntInf *info);
    int ReadAuditRec(char *buf, char type, int pos, int max_num);

    int latest() { return latest_; };
    int usr_num() { return usr_num_; };
    char *pswd_remote(int idx) { return usr_inf_[idx].pswd_remote; };
    char *usr_name() { return usr_inf_[latest_].name; };
    char *usr_name(int idx) { return usr_inf_[idx].name; };
    void set_latest(int val) { latest_ = val; };
    bool audit_alarm() { return audit_alarm_; };
    bool reset_audit_alarm() { audit_alarm_=false; };
protected:
private:
    int CreatePasswd();
    void ReadPasswd();
    void SaveAudAlarm(char *inf);
    void BakAudit();
    void ReadWarnEvt();

    LoopBuffer<AuditEvntInf> * warn_evnt_;
    LoopBuffer<AuditEvntInf> * alarm_evnt_;

    int usr_num_;           //Number of user. <=8
    int latest_;            //latest user index
    UserInfo usr_inf_[8];   //User information
    LoopBufSort<long> *audit_state_; //store audit record filename in sequence. e.g. 20150809=2015-8-9
    bool audit_alarm_;
};

inline UserManage & usr_mng()
{
	static UserManage um;
	return um;
};
/*! default user&password
admin:38962558:Boyuu_17
operator:666666:Normal_17
auditor:7777777:Audit_17
*/

/*! passwd file format
 *  Username:Password_local(MD5):Password_remote(MD5):AccessPermission:Latest
 *   AccessPermission -- Hex, one permission per bit,total 32bit
 *   Latest -- latest access user id
 *  e.g.
 *   admin:68bedb1d00ca8a1e6200f662d24557b1:aa2c5bc031b860fb510742cd04b04a60:ffffffff:0
 *   operator:f379eaf3c831b04de153469d1bec345e:0188a40ecc69501cb0e2b5924fa2b840:000000ac:1
 *   auditor:dc0fa7df3d07904a09288bd2d2bb5f40:aa7ce806f8cb7fcd673c7b02e585a615:000000ac:1
 */

/*! audit.rec.log file format
 *  YYYYMMDDhhmmss type user detail result
 *   type -- login, setting, app, os, warning
 *   result -- ok, no
 *  e.g.
 *   20170613_093428 system admin "login from 192.168.1.3" no
 *   20170613_093553 system admin "login from 192.168.1.3" ok
 *   20170613_093805 setting admin "PT" ok
 *   20170613_094018 setting admin "Voltage level" ok
 */

#endif //_USER_MANAGE_

