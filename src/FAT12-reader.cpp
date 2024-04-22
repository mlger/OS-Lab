#include <cstring>
#include <fstream>
#include <iostream>
#include <regex>
#include <string>
#include <vector>
#include "myString.h"
using namespace std;
#define mp(x, y) make_pair((x), (y))
#define pii pair<int, int>
#define ui unsigned int

const int N = 1e6 + 6;
bool isLower(char ch) {
    return ch >= 'a' && ch <= 'z';
}
bool isUpper(char ch) {
    return ch >= 'A' && ch <= 'Z';
}
bool isDigit(char ch) {
    return ch >= '0' && ch <= '9';
}
bool isDot(char ch) {
    return ch == '.';
}

int getChId(char ch) {
    if ('a' <= ch && ch <= 'z')
        return ch - 'a';
    else if ('A' <= ch && ch <= 'Z')
        return ch - 'A' + 26;
    else
        return -1;
}

myString imgData;
struct BPB {                  // 11-35  25Bytes
    uint16_t BPB_BytsPerSec;  // 每扇区字节数 512
    uint8_t BPB_SecPerClus;   // 每簇扇区数 1
    uint16_t BPB_RsvdSecCnt;  // Boot记录占用的扇区数 1
    uint8_t BPB_NumFATs;      // FAT表个数 2
    uint16_t BPB_RootEntCnt;  // 根目录最大文件数 224
    uint16_t BPB_TotSec16;    // 扇区总数 2880
    uint8_t BPB_Media;        // 介质描述符 240
    uint16_t BPB_FATSz16;     // 每个FAT表所占扇区数 9
    uint16_t BPB_SecPerTrk;   // 每磁道扇区数（Sector/track） 18
    uint16_t BPB_NumHeads;    // 磁头数（面数） 2
    uint32_t BPB_HiddSec;     // 隐藏扇区数 0
    uint32_t BPB_TotSec32;  // 如果BPB_ToSec16为0，该值为扇区数 1610612736
};

struct Entry {              // 32 Bytes
    char FILE_Name[11];     // 文件名+扩展名
    uint8_t FILE_Attr;      // 文件属性
    char reserved[10];      // 保留位
    uint16_t DIR_WrtTime;   // 最后一次写入时间
    uint16_t DIR_WrtDate;   // 最后一次写入日期
    uint16_t DIR_FstClus;   // 开始簇号
    uint32_t DIR_FileSize;  // 文件大小
    myString fileName;      // 文件名

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
vector<Entry> rootEntry;
vector<vector<Entry> > clusEntry;
vector<int> beginRootFile;
vector<myString> command_list = {"ls", "cat"};
vector<char> ls_options = {'l'}, cat_options = {};

int beginData;
Entry rt, errorNode;
myString colRed = "\033[31m", colRecover = "\033[0m";

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

void init(myString str);                     // 初始化
bool optionExist(char ch, myString opName);  // 判断选项是否存在
bool lExist(int opt);                        // 判断是否存在l选项
int getNextClus(int dataEntryId);            // 获取下一个簇
ui getSize(Entry entry);                     // 获取文件大小
vector<Entry> getChildrens(Entry, bool flag = false);  // 获取某个目录的所有孩子
pii countChildren(Entry entry);                        // 子目录计数
pii countChildrenRoot();                               // 根目录计数
myString getEntryInfo(Entry entry, bool flag = false);  // 获取文件信息
myString getEntryName(Entry entry);                     // 获取文件名
myString lsRoot(int opt);                               // 根目录ls
myString ls(Entry entry, myString path, int opt);       // ls
myString getClusData(int id);                           // 获取簇数据
myString cat(Entry entry);                              // cat
Entry getEntry(myString path);         // 根据最简化路径获取Entry
myString getStartPath(myString path);  // 根据完整路径获取起始最简化路径
Command get_command(myString input);  // 解析命令，获取操作名、参数、目标目录
myString getSpecialName(Entry entry);  // 获取特殊文件名(前一个目录的)
myString getNormalName(Entry entry);  // 获取普通文件名
bool shouldLowerCase(myString str);   // 判断是否需要获取小写名
void work();
void myOutput(myString str);
extern "C" void print(const char* str);
int main(int argc, char* argv[]) {
    // 打开镜像文件
    // myString imgPath = argv[1];
    string imgPath = "./a.img";
    ifstream inputFile(imgPath, ios::binary);
    if (!inputFile.is_open()) {
        cerr << "无法打开镜像文件" << std::endl;
        return 0;
    }
    // 读取镜像文件
    vector<char> buffer(std::istreambuf_iterator<char>(inputFile), {});
    inputFile.close();
    imgData = myString(buffer);
    // 初始化
    init(imgData);
    // 执行用户命令
    work();
    return 0;
}

bool shouldLowerCase(myString str) {
    if (!str.startWith("A"))
        return false;
    // 出现过小写字母
    int len = str.length();
    for (int i = 1; i < len; i += 2) {
        if (isLower(str.charAt(i))) {
            return true;
        } else if (!isUpper(str.charAt(i)) && !isDigit(str.charAt(i)) &&
                   !isDot(str.charAt(i))) {
            return false;
        }
    }
    return false;
}

myString getSpecialName(myString str) {
    myString res;
    int len = str.length();
    for (int i = 1; i < len; i += 2) {
        char c = str.charAt(i);
        if (isLower(c) || isUpper(c) || isDot(c) || isDigit(c)) {
            res.append(c);
        } else {
            return res;
        }
    }
    return res;
}

myString getNormalName(myString str) {
    if (str.startWith(".."))
        return "..";
    else if (str.startWith("."))
        return ".";
    myString res;
    for (int i = 0; i < 8; i++) {
        char c = str[i];
        if (!isUpper(c) && !isDigit(c)) {
            break;
        }
        res.append(c);
    }
    if (res.isEmpty())
        return res;  // 文件名为空
    for (int i = 8; i < 11; i++) {
        char c = str[i];
        if (!isUpper(c) && !isDigit(c)) {
            break;
        }
        if (i == 8) {
            res.append('.');
        }
        res.append(c);
    }
    return res;
}

void init(myString str) {
    bpb = getBPB(str.subString(11, 35));
    int beginFAT, lenFAT, beginRoot, maxRootLen;
    beginFAT = bpb.BPB_RsvdSecCnt * bpb.BPB_BytsPerSec;
    lenFAT = bpb.BPB_FATSz16 * bpb.BPB_BytsPerSec;
    beginRoot = beginFAT + (lenFAT * bpb.BPB_NumFATs);
    maxRootLen = bpb.BPB_RootEntCnt * 32;
    beginData = beginRoot + maxRootLen;

    // FAT:
    for (int i = 0, a = 0, b = 0; i < lenFAT; i++) {
        // ABCDEF

        /*
                bytes[] = ABCDEF
                FAT[28]=29 DAB
                FAT[29]=30 EFC
        */
        if (i % 3 == 0) {
            a = str.charAt(beginFAT + i);
        } else if (i % 3 == 1) {
            a |= (((str.charAt(beginFAT + i)) & 0x0F) << 8);
            tabFAT.push_back(a);
            b = (str.charAt(beginFAT + i) & 0xF0) >> 4;

        } else if (i % 3 == 2) {
            b |= (int)str.charAt(beginFAT + i) << 4;
            tabFAT.push_back(b);
        }
    }

    // rootEntry:
    for (int i = 0; (i << 5) < maxRootLen; i++) {
        int pos = beginRoot + (i << 5);
        char c = str.subString(pos, pos + 32).charAt(0);
        if (!isUpper(c) && !isDigit(c))
            break;
        rootEntry.push_back(getEntery(str.subString(pos, pos + 32)));
        if (i > 0 && shouldLowerCase(str.subString(pos - 32, pos))) {
            rootEntry[i].fileName =
                getSpecialName(str.subString(pos - 32, pos));
            rootEntry[i - 1].fileName = "";
        } else {
            rootEntry[i].fileName = getNormalName(str.subString(pos, pos + 32));
        }
        beginRootFile.push_back(pos);
    }
    // for (int i = 0; i < rootEntry.size(); i++) {
    //     if (rootEntry[i].fileName.isEmpty()) {
    //         rootEntry.erase(rootEntry.begin() + i);
    //		beginRootFile.erase(beginRootFile.begin() + i);
    //        i--;
    //    }
    //}

    // dataEntry:
    int len = str.length();
    for (int i = beginData; i < len; i += 512) {
        vector<Entry> dataEntry;
        for (int j = i; j < i + 512 && j < len; j += 32) {
            dataEntry.push_back(getEntery(str.subString(j, j + 32)));
        }
        for (int j = 0, pos = i; j < dataEntry.size(); j++, pos += 32) {
            if (j > 0 && shouldLowerCase(str.subString(pos - 32, pos))) {
                dataEntry[j].fileName =
                    getSpecialName(str.subString(pos - 32, pos));
                dataEntry[j - 1].fileName = "";
            } else {
                dataEntry[j].fileName =
                    getNormalName(str.subString(pos, pos + 11));
            }
        }
        for (int j = 0; j < dataEntry.size(); j++) {
            if (dataEntry[j].fileName.isEmpty()) {
                dataEntry.erase(dataEntry.begin() + j);
                j--;
            }
        }
        clusEntry.push_back(dataEntry);
    }
    // errorNode:
    errorNode.DIR_FstClus = -1;
    rt.FILE_Attr = 0x10;
    // 剔除无关数据
    for (int i = 0; i < tabFAT.size(); i++) {
        if (tabFAT[i] == 0 || tabFAT[i] >= 0xff8) {
            tabFAT[i] = -1;
        }
    }

    // errorNode.DIR_FstClus = -1;
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

bool lExist(int opt) {
    int lId = getChId('l');
    return opt >> lId & 1;
}

int getNextClus(int dataEntryId) {
    int temp = tabFAT[dataEntryId];
    if (temp >= 0xff7 || temp <= 0)
        return -1;
    return temp;
}

pii countChildren(Entry entry) {
    int cntDir = 0, cntFile = 0;
    int now = entry.DIR_FstClus;
    vector<Entry> children = getChildrens(entry);
    for (auto child : children) {
        if (child.fileName.equals(".") || child.fileName.equals(".."))
            continue;
        if (child.isDir())
            ++cntDir;
        else
            ++cntFile;
    }
    return mp(cntDir, cntFile);
}

ui getSize(Entry entry) {
    return (ui)entry.DIR_FileSize;
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

myString getEntryInfo(Entry entry, bool flag) {
    myString res;
    pii temp;
    if (!flag && !entry.isDir()) {  // 不是根且不是目录,是文件f
        res = myString(getSize(entry));
    } else {
        if (flag) {
            temp = countChildrenRoot();
        } else {
            temp = countChildren(entry);
        }
        res = myString(temp.first);
        res.append(' ');
        res.append(myString(temp.second));
    }
    return res;
}

myString getEntryName(Entry entry) {
    return entry.fileName;
}

myString lsRoot(int opt) {
    myString res("/");
    if (lExist(opt)) {
        res.append(' ');
        res.append(getEntryInfo(rt, true));
    }
    res.append(":\n");
    if (lExist(opt))
        res.append(colRed + "." + colRecover + "\n" + colRed + ".." +
                   colRecover + "\n");
    else
        res.append(colRed + "." + colRecover + " " + colRed + ".." +
                   colRecover + " ");
    for (auto entry : rootEntry) {
        if (entry.isDir()) {
            res.append(colRed + entry.fileName + colRecover);
        } else {
            res.append(entry.fileName);
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
            res.append(ls(entry, path.aggregate(entry.fileName), opt));
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
    vector<Entry> children = getChildrens(entry);
    for (auto child : children) {
        if (child.isDir()) {
            res.append(colRed + child.fileName + colRecover);
        } else {
            res.append(child.fileName);
        }
        res.append(' ');
        if (lExist(opt)) {
            if (!child.fileName.equals(".") && !child.fileName.equals(".."))
                res.append(getEntryInfo(child));
            res.append('\n');
        }
    }
    res.removeTail(' ');
    res.append('\n');

    for (auto child : children) {
        if (child.fileName.equals(".") || child.fileName.equals(".."))
            continue;

        if (child.isDir()) {
            res.append(
                ls(child, path.aggregate(myString("/") + child.fileName), opt));
        }
    }
    return res;
}

myString getClusData(int id) {
    int len = bpb.BPB_SecPerClus * bpb.BPB_BytsPerSec;
    int beg = beginData + (id - 2) * len;
    return imgData.subString(beg, beg + len);
}

myString cat(Entry entry) {  // entry is a file not a dir
    myString res;
    int len = getSize(entry);
    if (len == 0)
        return res;
    int cnt = 0;
    int now = entry.DIR_FstClus;
    while (now != -1 && cnt < len) {
        myString rem = getClusData(now);
        int lenstr = rem.length();
        for (int i = 0; i < lenstr && cnt < len; i++) {
            res.append(rem.charAt(i));
            cnt++;
        }
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
    for (int i = 0; i < dirs.size(); i++) {
        bool flag = false;
        vector<Entry> childrens = getChildrens(res, i == 0);
        for (auto entry : childrens) {
            if (entry.fileName.equals(dirs[i])) {
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

vector<Entry> getChildrens(Entry entry, bool flag) {
    if (flag) {
        return rootEntry;
    }
    vector<Entry> res;
    int now = entry.DIR_FstClus;
    while (now != -1) {
        for (auto children : clusEntry[now - 2]) {
            res.push_back(children);
        }
        now = getNextClus(now);
    }
    return res;
}

myString getStartPath(myString path) {
    // 需要/开头
    if (!path.startWith("/")) {
        return path + " is not a valid path\n";
    }
    // 多个/合并成一个
    path.replace(std::regex("/+"), "/");
    // 若是根直接返回
    if (path.equals("/")) {
        return "/";
    }
    vector<myString> dirs = path.split('/');
    Entry res = rt;
    myString resPath = "/";
    for (int i = 0; i < dirs.size(); i++) {
        // 若定位到文件了，但是还有子文件/目录
        if (i != dirs.size() - 1 && !res.isDir()) {
            resPath = resPath + " is not a directory\n";
            break;
        }
        bool flag = false;
        // 获取子文件
        vector<Entry> childrens = getChildrens(res, resPath.equals("/"));
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
                if (entry.fileName.equals(dirs[i])) {
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
    if (temp.size() == 1 && (temp[0].equals("q") || temp[0].equals("quit"))) {
        res.opName = "quit";
        return res;
    }
    bool flag = false;
    for (auto command : command_list) {
        if (res.opName.equals(command)) {
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
                    res.setError(myString("Invalid option -- '") +
                                 str.charAt(j) + "'");
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
    if (res.target.isEmpty()) {
        res.target = "/";
    }
    return res;
}

void work() {
    myString input;
    myString res;
    while (true) {
        // printf(">");
        myOutput(">");
        input.readLine();
        Command command = get_command(input);
        if (command.errorFlag) {
            // printf("%s\n", command.errorMessage.toCharArray());
            myOutput(command.errorMessage + '\n');
            continue;
        } else if (command.opName.isEmpty()) {
            continue;
        } else if (command.opName.equals("quit")) {
            break;
        } else if (command.opName.equals("ls")) {
            if (command.target.equals("/")) {
                res = lsRoot(command.opParam);
                res.removeTail('\n');
                res.append('\n');
                // printf("%s\n", res.toCharArray());
                myOutput(res + "\n");
            } else {
                Entry entry = getEntry(command.target);
                if (!entry.isDir()) {
                    // printf("%s is not a directory\n",
                    //        command.target.toCharArray());
                    myOutput(command.target + " is not a directory\n");
                } else {
                    res = ls(entry, command.target, command.opParam);
                    res.removeTail('\n');
                    res.append('\n');
                    // printf("%s\n", res.toCharArray());
                    myOutput(res + "\n");
                }
            }
        } else if (command.opName.equals("cat")) {
            Entry entry = getEntry(command.target);
            if (entry.DIR_FstClus == -1) {
                // printf("%s\n", command.target.toCharArray());
                myOutput(command.target + "\n");
            } else if (entry.isDir()) {
                // printf("%s is a directory\n", command.target.toCharArray());
                myOutput(command.target + " is a directory\n");
            } else {
                // printf("%s\n", cat(entry).toCharArray());
                myOutput(cat(entry) + "\n");
            }
        }
    }
    return;
}

void myOutput(myString str) {
    print(str.toCharArray());
}