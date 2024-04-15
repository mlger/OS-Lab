#include <cstring>
#include <iostream>
#include <regex>
#include <string>
#include <vector>
#include "myString.h"
using namespace std;
#define mp(x, y) make_pair((x), (y))
#define pii pair<int, int>

const int N = 1e6 + 6;

myString imgData;
struct BPB {                  // 11-35  25Bytes
    uint16_t BPB_BytsPerSec;  // 每扇区字节数
    uint8_t BPB_SecPerClus;   // 每簇扇区数
    uint16_t BPB_RsvdSecCnt;  // Boot记录占用的扇区数
    uint8_t BPB_NumFATs;      // FAT表个数
    uint16_t BPB_RootEntCnt;  // 根目录最大文件数
    uint16_t BPB_TotSec16;    // 扇区总数
    uint8_t BPB_Media;        // 介质描述符
    uint16_t BPB_FATSz16;     // 每个FAT表所占扇区数
    uint16_t BPB_SecPerTrk;   // 每磁道扇区数（Sector/track）
    uint16_t BPB_NumHeads;    // 磁头数（面数）
    uint32_t BPB_HiddSec;     // 隐藏扇区数
    uint32_t BPB_TotSec32;    // 如果BPB_ToSec16为0，该值为扇区数
};

struct Entry {              // 32 Bytes
    char FILE_Name[11];     // 长度名+扩展名
    uint8_t FILE_Attr;      // 文件属性
    char reserved[10];      // 保留位
    uint16_t DIR_WrtTime;   // 最后一次写入时间
    uint16_t DIR_WrtDate;   // 最后一次写入日期
    uint16_t DIR_FstClus;   // 开始簇号
    uint32_t DIR_FileSize;  // 文件大小

    bool isDir() { return this->FILE_Attr & 0x10; }
};

struct LongFileNameEntry {  // unused temporarily
    uint8_t LFNOrd;         // 长文件名序号
    uint16_t LFNPart1[5];   // 长文件名的第一部分
    uint8_t LFNAttributes;  // 文件属性标志
    uint8_t LFNReserved1;   // 保留字段，应为0x00
    uint8_t LFNChecksum;    // 校验和
    uint16_t LFNPart2[6];   // 长文件名的第二部分
    uint16_t LFNReserved2;  // 保留字段，应为0x0000
    uint16_t LFNPart3[2];   // 长文件名的第三部分
};
BPB getBPB(myString str) {
    char* temp = str.toCharArray();
    BPB bpb;
    bpb.BPB_BytsPerSec = *((uint16_t*)&temp[0]);
    bpb.BPB_SecPerClus = *((uint8_t*)&temp[2]);
    bpb.BPB_RsvdSecCnt = *((uint16_t*)&temp[3]);
    bpb.BPB_NumFATs = *((uint8_t*)&temp[5]);
    bpb.BPB_RootEntCnt = *((uint16_t*)&temp[6]);
    bpb.BPB_TotSec16 = *((uint16_t*)&temp[8]);
    bpb.BPB_Media = *((uint8_t*)&temp[10]);
    bpb.BPB_FATSz16 = *((uint16_t*)&temp[11]);
    bpb.BPB_SecPerTrk = *((uint16_t*)&temp[13]);
    bpb.BPB_NumHeads = *((uint16_t*)&temp[15]);
    bpb.BPB_HiddSec = *((uint32_t*)&temp[17]);
    bpb.BPB_TotSec32 = *((uint32_t*)&temp[21]);
    return bpb;
}
Entry getEntery(myString str) {
    char* temp = str.toCharArray();
    Entry entry;

    strncpy(entry.FILE_Name, temp, 11);
    entry.FILE_Name[11] = '\0';
    entry.FILE_Attr = temp[11];
    strncpy(entry.reserved, &temp[12], 10);
    entry.reserved[10] = '\0';
    entry.DIR_WrtTime = *((uint16_t*)&temp[22]);
    entry.DIR_WrtDate = *((uint16_t*)&temp[24]);
    entry.DIR_FstClus = *((uint16_t*)&temp[26]);
    entry.DIR_FileSize = *((uint32_t*)&temp[28]);
    return entry;
}

BPB bpb;
vector<int> tabFAT;
vector<Entry> rootEntry, dataEntry;
vector<myString> command_list = {"ls", "cat"};
vector<char> ls_options = {'l'}, cat_options = {};

int beginData;
Entry rt, errorNode;

struct Command {
    Command()
        : opName(""),
          opParam(0),
          target(""),
          errorFlag(false),
          errorMessage("") {}
    myString opName;        // 操作名
    int opParam;            // 参数
    myString target;        // 目标目录
    bool errorFlag;         // 错误标志
    myString errorMessage;  // 错误信息
    void setError(myString str) {
        errorFlag = true;
        errorMessage = str;
    }
};

void init(myString str);                           // 初始化
bool optionExist(char ch, myString opName);        // 判断选项是否存在
int getChId(char ch);                              // 获取选项的id
bool lExist(int opt);                              // 判断是否存在l选项
int getNextClus(int dataEntryId);                  // 获取下一个簇
pii countChildren(Entry entry);                    // 子目录计数
unsigned int getSize(Entry entry);                 // 获取文件大小
pii countChildrenRoot();                           // 根目录计数
myString getEntryInfo(Entry entry, bool flag);     // 获取文件信息
myString getEntryName(Entry entry);                // 获取文件名
myString lsRoot(int opt);                          // 根目录ls
myString ls(Entry entry, myString path, int opt);  // ls
myString getClusData(int id);                      // 获取簇数据
myString cat(Entry entry);                         // cat
Entry getEntry(myString path);         // 根据最简化路径获取Entry
myString getStartPath(myString path);  // 根据完整路径获取起始最简化路径
vector<Entry> getChildrens(Entry, bool flag);  // 获取某个目录的所有孩子
Command get_command(myString input);	// 解析命令，获取操作名、参数、目标目录


int main(int argc, char* argv[]) {
    return 0;
}

void init(myString str) {
    bpb = getBPB(str.subString(11, 35));
    int beginFAT, endFAT, beginRoot;
    beginFAT = bpb.BPB_RsvdSecCnt * bpb.BPB_BytsPerSec;
    endFAT = beginFAT + bpb.BPB_FATSz16 * bpb.BPB_BytsPerSec;
    beginRoot =
        beginFAT + bpb.BPB_NumFATs * bpb.BPB_FATSz16 * bpb.BPB_BytsPerSec;
    beginData = beginRoot + bpb.BPB_RootEntCnt * 32;
    // FAT:
    int lenFAT = endFAT - beginFAT;
    for (int i = 0, a = 0, b = 0; i < lenFAT; i++) {
        if (i % 3 == 0) {
            a = (int)str.charAt(beginFAT + i);
        } else if (i % 3 == 1) {
            a <<= 4;
            a += ((int)str.charAt(beginFAT + i)) >> 4;
            tabFAT.push_back(a);
            b = ((int)str.charAt(beginFAT + i) & 0x0F);

        } else if (i % 3 == 2) {
            b <<= 8;
            b += (int)str.charAt(beginFAT + i);
            tabFAT.push_back(b);
        }
    }
    // rootEntry:
    for (int i = beginRoot; i < beginData; i += 32) {
        if (i + 32 > beginData ||
            getEntery(str.subString(i, i + 32)).FILE_Name[0] == 0x00)
            break;
        rootEntry.push_back(getEntery(str.subString(i, i + 32)));
    }
    // dataEntry:
    int len = str.length();
    for (int i = beginData; i < len; i += 32) {
        if (i + 32 > len)
            break;
        dataEntry.push_back(getEntery(str.subString(i, i + 32)));
    }
}

bool optionExist(char ch, myString opName = "ls") {
    if (opName == "ls") {
        for (auto c : ls_options) {
            if (c == ch)
                return true;
        }
        return false;
    } else if (opName == "cat") {
        for (auto c : cat_options) {
            if (c == ch)
                return true;
        }
        return false;
    }
    return false;
}

int getChId(char ch) {
    if ('a' <= ch && ch <= 'z')
        return ch - 'a';
    else if ('A' <= ch && ch <= 'Z')
        return ch - 'A' + 26;
    else
        return -1;
}

bool lExist(int opt) {
    int lId = getChId('l');
    return opt >> lId & 1;
}

int getNextClus(int dataEntryId) {
    int temp = tabFAT[dataEntryId];
    if (temp >= 0xff7)
        return -1;
    return temp;
}

pii countChildren(Entry entry) {
    int beg = entry.DIR_FstClus;
    int cntDir = 0, cntFile = 0;
    while (beg != -1) {
        if (dataEntry[beg].isDir())
            ++cntDir;
        else
            ++cntFile;
        beg = getNextClus(beg);
    }
    return mp(cntDir, cntFile);
}

unsigned int getSize(Entry entry) {
    return entry.DIR_FileSize;
}

pii countChildrenRoot() {
    int cntDir = 0, cntFile = 0;
    for (auto entry : rootEntry) {
        if (entry.isDir())
            ++cntDir;
        else
            ++cntFile;
    }
    return mp(cntDir, cntFile);
}

myString getEntryInfo(Entry entry, bool flag = false) {
    myString res;
    pii temp;
    if (!flag && !entry.isDir()) {
        res = myString(getSize(entry));
    } else {
        if (flag) {
            temp = countChildrenRoot();
        } else {
            temp = countChildren(entry);
        }
        res = myString(temp.first);
        res.append(' ');
        res.append(temp.second);
    }
    return res;
}

myString getEntryName(Entry entry) {
    return myString(entry.FILE_Name);
}

myString lsRoot(int opt) {
    myString res("/");
    if (lExist(opt)) {
        res.append(' ');
        res.append(getEntryInfo(rt, true));
    }
    res.append(":\n");
    for (auto entry : rootEntry) {
        if (entry.isDir()) {
            res.append(myString(entry.FILE_Name));
        } else {
            res.append(myString(entry.FILE_Name));
        }
        res.append(' ');
        if (lExist(opt)) {
            res.append(getEntryInfo(entry));
            res.append('\n');
        }
    }
    res.removeTail(' ');
    res.append('\n');
    myString path("/");
    for (auto entry : rootEntry) {
        if (entry.isDir()) {
            res.append(
                ls(entry, path.aggregate(myString(entry.FILE_Name)), opt));
        }
    }
    return res;
}

myString ls(Entry entry, myString path, int opt) {
    // root root/dir data
    myString res(path);
    if (lExist(opt)) {
        res.append(' ');
        res.append(getEntryInfo(entry));
    }
    res.append(":\n");
    int now = entry.DIR_FstClus;
    Entry child;
    while (now != -1) {
        child = dataEntry[now];
        if (child.isDir()) {
            res.append(myString(child.FILE_Name));
        } else {
            res.append(myString(child.FILE_Name));
        }
        res.append(' ');
        if (lExist(opt)) {
            res.append(getEntryInfo(child));
            res.append('\n');
        }
        now = getNextClus(now);
    }
    res.removeTail(' ');
    res.append('\n');

    now = entry.DIR_FstClus;
    Entry child;
    while (now != -1) {
        child = dataEntry[now];
        if (child.isDir()) {
            res.append(
                ls(child, path.aggregate(myString(child.FILE_Name)), opt));
        }
        now = getNextClus(now);
    }
    return res;
}

myString getClusData(int id) {
    int len = bpb.BPB_SecPerClus * bpb.BPB_BytsPerSec;
    int beg = beginData + (id - 1) * len;
    return imgData.subString(beg, beg + len);
}

myString cat(Entry entry) {
    myString res;
    int now = entry.DIR_FstClus;
    while (now != -1) {
        res.append(getClusData(now));
        now = getNextClus(now);
    }
    return res;
}

Entry getEntry(myString path) {
    if (path.equals("/")) {
        return rt;
    }
    vector<myString> dirs = path.split('/');
    Entry res = rt;
    for (int i = 1; i < dirs.size(); i++) {
        bool flag = false;
        for (auto entry : dataEntry) {
            if (myString(entry.FILE_Name).equals(dirs[i])) {
                res = entry;
                flag = true;
                break;
            }
        }
        if (!flag) {
            res.DIR_FstClus = -1;
            break;
        }
    }
    return res;
}

vector<Entry> getChildrens(Entry entry, bool flag = false) {
    if (flag) {
        return rootEntry;
    }
    vector<Entry> res;
    int now = entry.DIR_FstClus;
    Entry child;
    while (now != -1) {
        child = dataEntry[now];
        res.push_back(child);
        now = getNextClus(now);
    }
    return res;
}

myString getStartPath(myString path) {
    path.replace(std::regex("/+"), "/");
    if (path.equals("/")) {
        return "/";
    }
    vector<myString> dirs = path.split('/');
    Entry res = rt;
    myString resPath = "/";
    for (int i = 1; i < dirs.size(); i++) {
        if (i != dirs.size() - 1 && !res.isDir()) {
            res.DIR_FstClus = -1;
            resPath = resPath + " is not a directory\n";
            break;
        }
        bool flag = false;
        vector<Entry> childrens = getChildrens(res);
        if (dirs[i].equals(".")) {
            continue;
        } else if (dirs[i].equals("..")) {
            if (!resPath.equals("/")) {
                resPath.remove(std::regex(".*/([^/]+)$"));
                if (resPath.isEmpty())
                    resPath = "/";
                res = getEntry(resPath);
            }
        } else {
            for (auto entry : childrens) {
                if (myString(entry.FILE_Name).equals(dirs[i])) {
                    res = entry;
                    if (resPath.equals("/"))
                        resPath = resPath + dirs[i];
                    else
                        resPath = resPath + "/" + dirs[i];
                    flag = true;
                    break;
                }
            }
            if (!flag) {
                res.DIR_FstClus = -1;
                resPath = resPath + " child " + dirs[i] + " not found\n";
                break;
            }
        }
    }
    return resPath;
}

Command get_command(myString input) {
    Command res;
    vector<myString> temp = input.split(' ');
    if (temp.size() == 0)
        return res;
    res.opName = temp[0];
    bool flag = false;
    for (auto command : command_list) {
        if (command.equals(res.opName)) {
            flag = true;
            break;
        }
    }
    if (!flag) {
        res.setError("Command not found");
        return res;
    }
    for (int i = 1; i < temp.size(); i++) {
        myString str = temp[i];
        if (str.startWith("-")) {
            if (str.length() == 1) {
                res.setError("Can't access '-':No such file or directory");
                return res;
            }
            for (int j = 1; j < str.length(); j++) {
                int id = getChId(str.charAt(j));
                if (id == -1 || !optionExist(str.charAt(j), res.opName)) {
                    res.setError(myString("Invalid option -- '")+str.charAt(j)+"'");
                    return res;
                }
                res.opParam |= 1 << id;
            }
        } else {
            if (res.target.isEmpty()) {
                res.target = getStartPath(str);
                if (!res.target.match(regex("/|(/[^/\\s]+)+")))
                    res.setError(res.target);
                return res;
            } else {
                res.setError("Please input only one target a time");
                return res;
            }
        }
    }
}
