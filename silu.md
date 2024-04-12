#### 一、任务

制作一个 FAT12 镜像查看器。

支持命令：ls, ls -l, cat。

#### 二、知识

学长的文章（写得好好）： https://blog.csdn.net/qq_66026801/article/details/130520032

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
	char DIR_Name[11];      //长度名+扩展名
    u8   DIR_Attr;          //文件属性
    char reserved[10];      //保留位
    u16  DIR_WrtTime;       //最后一次写入时间
    u16  DIR_WrtDate;       //最后一次写入日期
    u16  DIR_FstClus;	    //开始簇号
    u32  DIR_FileSize;      //文件大小
};	//跟 RootEntry 一模一样
```

#### 三、代码设计

```
int getAddrFAT(int id){}	
int getAddrClus(int id){}
int getFileType(){}	//获取文件类型 文件/子目录
int countChildren(){}	//计算子文件/目录个数
int getFileSize(){}
```

