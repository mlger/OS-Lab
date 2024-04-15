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

#### 三、代码设计

整体访问思路是：表项 => 数据区 => 下一表项

数据预处理，获取 BPB、表项、根目录及数据簇。

暂时先不考虑 LFN 的情况。

```c++
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
BPB bpb;
vector<int> tabFAT;
vector<Entry> rootEntry, dataEntry;

void init(char* str) {
    
}
```



**以下代码慎重参考，个人风格较强。**（好吧个人风格没写到 QwQ）。

使用一个全局的 vector 来存储答案：

```
vector<char> res;
```

将 `ls` 与 `ls -l` 集成：

```c++
void ls(int id, char dirName[], char args[]) {
	// 判定是否存在-l
	// 根据dirName提取簇号id
	visit(id);
}
void visit(int id) {
    // output information ID
    visit(getBeginClus(id));
	visit(getNextClus(id));
}
int getType(int id) {	// 判断目录/文件
    
}
char* getInformation(int id) {	
    
}
int getFileSize(int id){
    
}
int getNextClus(int id) {
    
}
int getBeginClus(int id){
    
}
```



