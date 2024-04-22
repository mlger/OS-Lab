---
title: OS-Lab0x02 思路
date: 2024-04-13 01:14
categories: OS
tag: 专业课
mathjax: true
---

#### 一、任务

制作一个 FAT12 镜像查看器。

支持命令：ls, ls -l, cat。

#### 二、知识

学长的文章（写得好好）： [FAT12镜像查看工具\_解析fat32镜像文件-CSDN博客](https://blog.csdn.net/qq_66026801/article/details/130520032)

cat 需要访问数据区。

ls、ls -l 需要访问目录区。

##### 1. MBR 区

从第 `11` 个字节开始的 `25` 个字节构成一个较为特殊的结构 `BPB(Bios Parameter Block)`。

```
typedef struct BPB {
    u16  BPB_BytsPerSec;	//每扇区字节数
    u8   BPB_SecPerClus;	//每簇扇区数
    u16  BPB_RsvdSecCnt;	//Boot记录占用的扇区数
    u8   BPB_NumFATs;	        //FAT表个数
    u16  BPB_RootEntCnt;	//根目录最大文件数
    u16  BPB_TotSec16;		//扇区总数
    u8   BPB_Media;             //介质描述符
    u16  BPB_FATSz16;	        //每个FAT表所占扇区数
    u16  BPB_SecPerTrk;         //每磁道扇区数（Sector/track）
    u16  BPB_NumHeads;	        //磁头数（面数）
    u32  BPB_HiddSec;	        //隐藏扇区数
    u32  BPB_TotSec32;	        //如果BPB_ToSec16为0，该值为扇区数
} BPB;                          //25字节

```

##### 2. FAT 表

`FAT1` 与 `FAT2` 互为备份，所以理论上两张表是一样的。

FAT 表项的值的含义：

- 通常情况下代表文件下一簇号。
- 值 `>= 0xFF8`，该簇已经是文件最后一个簇。
- 值 `= 0xFF7`，表示一个坏簇。

**簇的编号与 FAT 表中的索引**：每个簇都有一个唯一的编号，FAT 表中的项索引与这些簇的编号对应。例如，第一个簇的编号对应 FAT 表中的第一个项，第二个簇对应第二个项，依此类推。

##### 3. 根目录区

一个目录项占据 `32` 字节。

```
typedef struct RootEntry {
    char DIR_Name[11];      //长度名+扩展名
    u8   DIR_Attr;          //文件属性
    char reserved[10];      //保留位
    u16  DIR_WrtTime;       //最后一次写入时间
    u16  DIR_WrtDate;       //最后一次写入日期
    u16  DIR_FstClus;	    //开始簇号
    u32  DIR_FileSize;      //文件大小
} RootEntry;                //32字节

```

由此，归结起来，FAT12访问文件的基本操作为：

1. 首先通过根目录文件查找文件名，确定是哪一个条目，接着在条目中访问 DIR_FstClus 对应的开始簇号。
2. 当一个簇号访问完后，通过FAT表项查询下一簇号，决定是结束还是继续访问下一簇号，重复第二条。

##### 4. 数据区

```
typedef struct Entry{
	char FILE_NAME[11];      //长度名+扩展名
    u8   FILE_Attr;          //文件属性
    char reserved[10];      //保留位
    u16  DIR_WrtTime;       //最后一次写入时间
    u16  DIR_WrtDate;       //最后一次写入日期
    u16  DIR_FstClus;	    //开始簇号
    u32  DIR_FileSize;      //文件大小
};	//跟 RootEntry 一模一样
```

![FAT12.png](https://raw.githubusercontent.com/mlger/Pict/main/newPath/5a90e9eb1d9186eaad167bb31fd1ce9d.png)

##### 5. File Attribution

| 位   | 掩码 | 描述                           |
| ---- | ---- | ------------------------------ |
| 0    | 0x01 | 只读                           |
| 1    | 0x02 | 隐藏                           |
| 2    | 0x04 | 系统                           |
| 3    | 0x08 | 卷标                           |
| 4    | 0x10 | 子目录                         |
| 5    | 0x20 | 档案                           |
| 6    | 0x40 | 设备（内部使用，磁盘上看不到） |
| 7    | 0x80 | 没有使用                       |

0x0F 是 LFN。

#### 三、注意事项

整体是做一个深度优先遍历。从本质上来说，本次实验难度仅存在于定位数据。这里列出一些值得注意的点。

##### 1. 数据区起始

**数据区起始于簇 2**。事实上这句话我到现在还没读懂，我是通过插入空簇来实现下标与需求匹配的。

##### 2. FAT 表项的计算

FAT 每个表项占 1.5 个字节，因此每次提取三个字节计算两个表项。

若这三个字节十六进制表示为 AB CD EF（按地址从小到大排列）

那么有

```
前一个FAT = DAB
后一个FAT = EFC
```

##### 3. 关于无关的 Entry

由于 FAT12 文件名仅支持至多 8 位大写的文件名与至多 3 位大写的后缀，若文件名出现小写，在镜像中可能会出现一些 Entry 作为补丁。我们可以通过这些项获取原文件名，但在访问的适合需要将这些项剔除。

##### 4. tool

插件 Hex-editor。

##### 5. 其它

```c++
myString getSpecialName(Entry entry);  // 获取特殊文件名(前一个目录的)
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
```

**框架中出现的这些属于扩展功能，不需要实现。（某人没看题QwQ）**

#### 三、代码设计

暂时先不考虑 LFN 的情况。

```c++
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
pii countChildren(Entry entry);              // 子目录计数
pii countChildrenRoot();                     // 根目录计数
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
```



