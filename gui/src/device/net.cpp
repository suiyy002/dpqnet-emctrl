#include <cstdio>
#include <cstdlib>
#include <unistd.h>
#include <cstring>

#include "device.h"

static const char * FRcLocal = "/boyuu/save/rc.driver"; //系统启动配置文件
static const char * Fntpclient = "/boyuu/save/ntpclient.sh"; //ntp 脚本文件

//-------------------------------------------------------------------------
//Description: 获取网络参数
//Input: type,参数类型。1=ip; 2=netmask; 3=gateway; 4=mac
//Output: str, 相应网络参数的字符串形式
void get_net_para(int type, char *str)
{
    FILE *f_strm;
    char *chpi, *chpj;
    char stri[81];
    int findi = 0;

    chpj = "Not found";

    f_strm = fopen(FRcLocal, "r");
    if (f_strm == NULL) {
        printf("file:%s not exist!\n", FRcLocal);
        strcpy(str, "No exit");
        return;
    }
    switch (type) {
        case 1: //ip
            chpi = "IPADDR";
            break;
        case 2: //netmask
            chpi = "NETMASK";
            break;
        case 3: //gateway
            chpi = "GATEWAY";
            break;
        case 4: //MAC address
            chpi = "HWADDR";
            break;
        default:
            chpi = "UNDEFINE";
            break;
    }
    while (!findi) {
        if (!fgets(stri, 80, f_strm)) {
            if (!feof(f_strm)) {
                printf("Read file:%s error!\n", FRcLocal);
                chpj = "Read error";
            } else {
                printf("Read file:%s end!\n", FRcLocal);
            }
            break;
        }
        if (!strncmp(stri, chpi, strlen(chpi))) {
            findi = 1;
        }
    }
    if (findi) {
        switch (type) {
            case 1: //ip
                chpi = &stri[strlen(chpi)];
                chpj = strtok(chpi, "=\"");
                break;
            case 2: //netmask
                chpi = &stri[strlen(chpi)];
                chpj = strtok(chpi, "=\"");
                break;
            case 3: //gateway
                chpi = &stri[strlen(chpi)];
                chpj = strtok(chpi, "=\"");
                break;
            case 4: //mac address
                chpi = &stri[strlen(chpi)];
                chpj = strtok(chpi, "=\"");
                break;
            default:
                chpi = "UNDEFINE";
                break;
        }
    }
    strcpy(str, chpj);
    fclose(f_strm);
    return;
}

//-------------------------------------------------------------------------
//Description: 设置网络IP参数
//Input: type,参数类型。1=ip; 2=netmask; 3=gateway; 4=mac; 5=start sshd; 6=stop sshd
//  str, 相应网络参数的字符串形式
void set_net_para(int type, char *str)
{
    FILE *f_strm, *tmp_strm;
    char *chpi;
    char stri[81];
    int findi = 0;
    switch (type) {
        case 1: //ip
            chpi = "IPADDR";
            break;
        case 2: //netmask
            chpi = "NETMASK";
            break;
        case 3: //gateway
            chpi = "GATEWAY";
            break;
        case 4: //MAC address
            chpi = "HWADDR";
            break;
        case 5: //sshd start
            chpi = "/etc/init.d/S50sshd";
            break;
        case 6: //sshd stop
            chpi = "#/etc/init.d/S50sshd";
            break;
        default:
            chpi = "UNDEFINE";
            break;
    }

    int ipa1, ipa2, ipa3, ipa4;
    ipa1 = ipa2 = ipa3 = ipa4 = 666;
    //判断IP地址或子网掩码的格式是否合法
    if ((!strcmp(chpi, "GATEWAY") && strcmp(str, ""))
        || !strcmp(chpi, "IPADDR") || !strcmp(chpi, "NETMASK")) {
        sscanf(str, "%d.%d.%d.%d", &ipa1, &ipa2, &ipa3, &ipa4);
        if (ipa1 > 255 || ipa2 > 255 || ipa3 > 255 || ipa4 > 255) {
            return;
        }
    }

    tmp_strm = tmpfile();//fopen("tmpfile","w");
    f_strm = fopen(FRcLocal, "r");
    if (f_strm == NULL) {
        printf("file:%s not exist!\n", FRcLocal);
        return ;
    }
    while (!findi) { //查找包含chpi的行
        if (!fgets(stri, 80, f_strm)) {
            if (!feof(f_strm)) {
                printf("Read file:%s error!\n", FRcLocal);
            } else {
                printf("Read file:%s end, not found!\n", FRcLocal);
            }
            break;
        }
        if (!strncmp(stri, chpi, strlen(chpi))) {
            findi = 1;
        } else {
            if (fputs(stri, tmp_strm) < 0) {
                printf("write tmp file error!\n");
            }
        }
    }
    if (findi) {
        if (type<5) sprintf(stri, "%s=\"%s\"\n", chpi, str);
        else if (type==5) sprintf(stri, "%c%s stop\n", '#', chpi);
        else if (type==6) sprintf(stri, "%s stop\n", &chpi[1]);
        while (1) { //把剩余的行读至临时文件中
            if (fputs(stri, tmp_strm) < 0) {
                printf("write tmp file error!\n");
            }

            if (!fgets(stri, 80, f_strm)) {
                if (!feof(f_strm)) {
                    printf("Read file:%s error!\n", FRcLocal);
                }
                break;
            }
        }
    }
    fclose(f_strm);

    //把改动的文件从临时文件写回原文件
    f_strm = fopen(FRcLocal, "w");
    fseek(tmp_strm, 0, SEEK_END);
    int i = ftell(tmp_strm);
    chpi = new char[i+2];
    fseek(tmp_strm, 0, SEEK_SET);
    fread(chpi, 1, i, tmp_strm);
    fwrite(chpi, 1, i, f_strm);
    delete [] chpi;

    fclose(f_strm);
    fclose(tmp_strm);
    return;
}

//-------------------------------------------------------------------------
///[WHF
//Description: 应用网络IP参数
void apply_net_para()
{
    char str[3][24];
    char stri[64];
    int i;

    for (i = 0;i < 3;i++) {
        get_net_para(i + 1, str[i]);
    }


    sprintf(stri, "ifconfig eth0 %s netmask %s", str[0], str[1]);
    system(stri);
    puts(stri);

    if (!strcmp(str[2], "")) return;
    system("route del default");
    sprintf(stri, "route add default gw %s metric 1", str[2]);
    puts(stri);
    system(stri);

    return;
}
///]

//取文档行数
//return -1:文档打开有误
static int get_file_linecount(const char * filename)
{
    int retval = 0;
    char str[1024], *pstr;
    FILE *fp;

    fp = fopen(filename, "r");
    if (fp == NULL) {
        printf("open file error: %s!\n", filename);
        return -1;
    }
    while (fgets(str, 1024, fp)) {
        retval++;
    }
    fclose (fp);
    return retval;
}

//检测网络文档
void chk_net_file(void)
{
    int org_linecnt, new_linecnt;
    char net_param[4][128];
    char cmd[128];
    int i;
    new_linecnt = get_file_linecount(FRcLocal);
    org_linecnt = get_file_linecount("/boyuu/rc.driver");
    if (org_linecnt <= 0 || new_linecnt <= 0 || new_linecnt >= org_linecnt )
        return;
    for (i = 1; i < 5; i++) {
        get_net_para(i, net_param[i-1]);
    }
    sprintf(cmd, "cp -a /boyuu/rc.driver %s", FRcLocal);
    printf("%s\n", cmd);
    system(cmd);
    sleep(1);
    for (i = 1; i < 5; i++) {
        set_net_para(i, net_param[i-1]);
    }
}

/*-------------------------------------------------------------------------
Description:获取ntp参数
Input:      type,参数类型。1=ntp server ip;2=port
Output:     str, 相应网络参数的字符串形式
-------------------------------------------------------------------------*/
void get_ntp_para(int type, char *str)
{
    FILE *f_strm;
    char *chpi, *chpj;
    char stri[81];
    int findi = 0;

    chpj = "Not found";

    f_strm = fopen(Fntpclient, "r");
    if (f_strm == NULL) {
        printf("file:%s not exist!\n", FRcLocal);
        strcpy(str, "No exit");
        return;
    }
    switch (type) {
        case 1: //NTP server ip
            chpi = "Addr";
            break;
        case 2: //NTP Server port
            chpi = "Port";
            break;
        default:
            chpi = "UNDEFINE";
            break;
    }
    while (!findi) {
        if (!fgets(stri, 80, f_strm)) {
            if (!feof(f_strm)) {
                printf("Read file:%s error!\n", Fntpclient);
                chpj = "Read error";
            } else {
                printf("Read file:%s end!\n", Fntpclient);
            }
            break;
        }
        if (!strncmp(stri, chpi, strlen(chpi))) {
            findi = 1;
        }
    }
    if (findi) {
        switch (type) {
            case 1: //NTP server ip
                chpi = &stri[strlen(chpi)];
                chpj = strtok(chpi, "=");
                break;
            case 2: //NTP Server port
                chpi = &stri[strlen(chpi)];
                chpj = strtok(chpi, "=");
                break;
            default:
                chpi = "UNDEFINE";
                break;
        }
    }
    strcpy(str, chpj);
    fclose(f_strm);
    return;
}

/*-------------------------------------------------------------------------
Description:设置ntp参数
Input:      type,参数类型。1=ntp server ip;2=port
            str, 相应网络参数的字符串形式
-------------------------------------------------------------------------*/
void set_ntp_para(int type, char *str)
{
    FILE *f_strm, *tmp_strm;
    char *chpi, *chpj;
    char stri[81];
    int findi = 0;

    switch (type) {
        case 1: //NTP server ip
            chpi = "Addr";
            break;
        case 2: //NTP Server port
            chpi = "Port";
            break;
        default:
            chpi = "UNDEFINE";
            break;
    }

    int ipa1, ipa2, ipa3, ipa4;
    ipa1 = ipa2 = ipa3 = ipa4 = 666;
    //判断IP地址的格式是否合法
    if (!strcmp(chpi, "Addr")) {
        sscanf(str, "%d.%d.%d.%d", &ipa1, &ipa2, &ipa3, &ipa4);
        if (ipa1 > 255 || ipa2 > 255 || ipa3 > 255 || ipa4 > 255) {
            return;
        }
    }

    tmp_strm = tmpfile();//fopen("tmpfile","w");
    f_strm = fopen(Fntpclient, "r");
    if (f_strm == NULL) {
        printf("file:%s not exist!\n", Fntpclient);
        return ;
    }
    while (!findi) { //查找包含chpi的行
        if (!fgets(stri, 80, f_strm)) {
            if (!feof(f_strm)) {
                printf("Read file:%s error!\n", FRcLocal);
                chpj = "Read error";
            } else {
                printf("Read file:%s end!\n", FRcLocal);
                chpj = "Not found";
            }
            break;
        }
        if (!strncmp(stri, chpi, strlen(chpi))) {
            findi = 1;
        } else {
            if (fputs(stri, tmp_strm) < 0) {
                printf("write tmp file error!\n");
            }
        }
    }
    if (findi) {
        sprintf(stri, "%s=%s\n", chpi, str);
        while (1) { //把剩余的行读至临时文件中
            if (fputs(stri, tmp_strm) < 0) {
                printf("write tmp file error!\n");
            }

            if (!fgets(stri, 80, f_strm)) {
                if (!feof(f_strm)) {
                    printf("Read file:%s error!\n", FRcLocal);
                    chpj = "Read error";
                }
                break;
            }
        }
    }
    fclose(f_strm);

    //把改动的文件从临时文件写回原文件
    f_strm = fopen(Fntpclient, "w");
    fseek(tmp_strm, 0, SEEK_END);
    int i = ftell(tmp_strm);
    chpi = new char[i+2];
    fseek(tmp_strm, 0, SEEK_SET);
    fread(chpi, 1, i, tmp_strm);
    fwrite(chpi, 1, i, f_strm);
    delete [] chpi;

    fclose(f_strm);
    fclose(tmp_strm);

    return;
}

/*!
Update ntpclient.sh
*/
void UpNtpParaFile()
{
    char stri[128];
    sprintf(stri,"%s -v", Fntpclient);
    int ret = system(stri);
    //printf("upntp ret=%x\n", ret);
    if ((ret>>8)<2) {
        get_ntp_para(1, stri);
        system("cp /boyuu/.recover/ntpclient.sh /boyuu/save/");
        set_ntp_para(1, stri);
        apply_ntp_para();
    }
}
//-------------------------------------------------------------------------
//Description: 应用ntp参数
void apply_ntp_para()
{
    char stri[128];
    sprintf(stri,". %s", Fntpclient);
    system(stri);

    return;
}

