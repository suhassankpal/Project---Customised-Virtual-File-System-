//*********************************************************************************************************************
//
//	                     Customized Virtual File System Application
//
//*********************************************************************************************************************

// ##################################
//
//  Header Files
//
// ##################################

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <iostream>
#include <io.h>

// ##################################
//
//  Defining The Macros
//
// ##################################

#define MAXINODE 50

#define READ 1
#define WRITE 2

#define MAXFILESIZE 1024 // Maximum Size Of A File (1024 = 1kb)

#define REGULAR 1 // regular file means .txt file
#define SPECIAL 2 // special file means .c .cpp file

#define START 0
#define CURRENT 1
#define END 2

// ##################################
//
//  Creating SuperBlock Structure
//
// ##################################

typedef struct superblock
{
    int TotalInodes;
    int FreeInode;
} SUPERBLOCK, *PSUPERBLOCK;

// ##################################
//
//  Creating Inode Structure
//
// ##################################

typedef struct inode
{
    char FileName[50];
    int InodeNumber;
    int FileSize;
    int FileActualSize;
    int FileType;
    char *Buffer;
    int LinkCount;
    int ReferenceCount;
    int Permission; // 1+2=3;
    struct inode *next;
} INODE, *PINODE, **PPINODE;

// ##################################
//
//  Creating FileTable Structure
//
// ##################################

typedef struct filetable
{
    int readoffset;  //  From Where To Read
    int writeoffset; //  From Where To write
    int count;       //  Remains 1 Throught The Code
    int mode;        // mode of files 1 or 2 or  1+2 = 3;
    PINODE ptrinode; // pointer Point To Inode
} FILETABLE, *PFILETABLE;

// ##################################
//
//  Creating UFDT Structure
//
// ##################################

typedef struct ufdt
{
    PFILETABLE ptrfiletable; // Pointer Which Points To File Table
} UFDT;

UFDT UFDTArr[50];         // UFDTArr is array is of type struct ufdt 3 Global variables;
SUPERBLOCK SUPERBLOCKobj; // here we create object of structure superblock;
PINODE Head = NULL;       // Here we create head pointer is of type struct inode;

// ######################################################################################
//
//	Function Name	: 	man
//	Input			: 	char *
//	Output			: 	None
//	Description 	: 	Function Display The Description and usage For Each Commands
//	Author			: 	Suhas Dilip Sankpal
//	Date			:	24 january 2023
//
// ######################################################################################

void man(char *name)
{
    if (name == NULL)
    {
        return;
    }

    if (strcmp(name, "create") == 0)
    {
        printf("Description :Used to create new regular file\n");
        printf("Usage: create file_name, Permission\n");
        // Add more decription here
    }
    else if (strcmp(name, "read") == 0)
    {
        printf("Description :Used to read data from regular file\n");
        printf("Usage: read File_name,No of bytes_To_Read\n");
    }
    else if (strcmp(name, "write") == 0)
    {
        printf("Description :Used to write into regular file\n");
        printf("Usage: write File_name\n After this enter data that we want to write\n");
    }
    else if (strcmp(name, "ls") == 0)
    {
        printf("Description :Used to list all information of files\n");
        printf("Usage: ls\n");
    }
    else if (strcmp(name, "stat") == 0)
    {
        printf("Description :Used to display information of file\n");
        printf("Usage: stat File_name\n");
    }
    else if (strcmp(name, "fstat") == 0)
    {
        printf("Description :Used to display information of file\n");
        printf("Usage: stat File_Discriptor\n");
    }
    else if (strcmp(name, "truncate") == 0)
    {
        printf("Description :Used to remove data from file\n");
        printf("Usage: truncate File_name\n");
    }
    else if (strcmp(name, "open") == 0)
    {
        printf("Description :Used to open existing file\n");
        printf("Usage: open File_name, mode\n");
    }
    else if (strcmp(name, "close") == 0)
    {
        printf("Description :Used to close opened file\n");
        printf("Usage: close File_name\n");
    }
    else if (strcmp(name, "closeall") == 0)
    {
        printf("Description :Used to close all opened file\n");
        printf("Usage: closeall\n");
    }
    else if (strcmp(name, "lseek") == 0)
    {
        printf("Description :Used to change file offset\n");
        printf("Usage: lseek File_name,changeInOffset,StartPoint\n");
    }
    else if (strcmp(name, "rm") == 0)
    {
        printf("Description :Used to delete the file\n");
        printf("Usage: rm File_name\n");
    }
    else
    {
        printf("ERROR: No manual enrty available\n");
    }
}

// ######################################################################################
//
//	Function Name	: 	DisplayHelp
//	Input			: 	None
//	Output			: 	None
//	Description 	: 	Function Display all list of operation perform on file
//	Author			: 	Suhas Dilip Sankpal
//  Date			:	24 january 2023
//
// ######################################################################################

void DisplayHelp()
{
    printf("ls : To List out all files\n");
    printf("clear : To clear console\n");
    printf("create : To create new file\n");
    printf("open : To open the file\n");
    printf("close : To close the file\n");
    printf("closeall : To close all opened file\n");
    printf("read : To Read the content from the file\n");
    printf("write : To write the content into file\n");
    printf("exit : To terminate file system\n");
    printf("stat : To display the information of file using name\n");
    printf("fstat : To display the information of file using file descriptor\n");
    printf("truncate : To remove all data from file\n");
    printf("rm : To delete the file\n");
    printf("lseek : To change offset the file\n");
}

// ######################################################################################
//
//	Function Name	: 	GetFDFromName
//	Input			: 	char *
//	Output			: 	integer
//	Description 	: 	Function return value of file discriptor(fd) of given file
//	Author			: 	Suhas Dilip Sankpal
//	Date			:	24 january 2023
//
// ######################################################################################

int GetFDFromName(char *name)
{
    int i = 0;

    while (i < 50)
    {
        if (UFDTArr[i].ptrfiletable != NULL)
        {
            if (strcmp((UFDTArr[i].ptrfiletable->ptrinode->FileName), name) == 0)
            {
                break;
            }
            i++;
        }
    }

    if (i == 50)
    {
        return -1;
    }
    else
    {
        return i;
    }
}

// ######################################################################################
//
//	Function Name	: 	Get_Inode
//	Input			: 	char *
//	Output			: 	PINODE
//	Description 	: 	Function return inode value of given file
//	Author			: 	Suhas Dilip Sankpal
//	Date			:	24 january 2023
//
// ######################################################################################

PINODE Get_Inode(char *name)
{
    PINODE temp = Head;
    int i = 0;

    if (name == NULL)
    {
        return NULL;
    }

    while (temp != NULL)
    {
        if (strcmp(name, temp->FileName) == 0)
        {
            break;
        }
        temp = temp->next;
    }
    return temp;
}

// ######################################################################################
//
//	Function Name	: 	CreateDILB
//	Input			: 	none
//	Output			: 	none
//	Description 	: 	Function create DILB (Linked_list of inode);
//	Author			: 	Suhas Dilip Sankpal
//	Date			:	24 january 2023
//
// ######################################################################################

void CreateDILB()
{
    int i = 1;
    PINODE newn = NULL;
    PINODE temp = Head;

    while (i <= MAXINODE)
    {
        newn = (PINODE)malloc(sizeof(INODE)); // 94 Bytes Of Memory Get Allocated

        newn->LinkCount = 0;
        newn->ReferenceCount = 0;
        newn->FileType = 0;
        newn->FileSize = 0;

        newn->Buffer = NULL;
        newn->next = NULL;

        newn->InodeNumber = i;

        if (temp == NULL)
        {
            Head = newn;
            temp = Head;
        }
        else
        {
            temp->next = newn;
            temp = temp->next;
        }
        i++;
    }
    printf("DILB created successfully\n");
}

// ######################################################################################
//
//	Function Name	: 	InitialiseSupperBlock
//	Input			: 	none
//	Output			: 	none
//	Description 	: 	Function initialise super block member with default value
//	Author			: 	Suhas Dilip Sankpal
//	Date			:	24 january 2023
//
// ######################################################################################

void InitialiseSupperBlock()
{
    int i = 0;
    while (i <= MAXINODE)
    {
        UFDTArr[i].ptrfiletable = NULL; // this loop is used to set all the pointers at null
        i++;
    }

    SUPERBLOCKobj.TotalInodes = MAXINODE;
    SUPERBLOCKobj.FreeInode = MAXINODE;
}

// ######################################################################################
//
//	Function Name	: 	CreateFile
//	Input			: 	char *,int
//	Output			: 	integer
//	Description 	: 	Function create new file
//	Author			: 	Suhas Dilip Sankpal
//	Date			:	24 january 2023
//
// ######################################################################################

// int CreateFile(Demo.txt,       3);
int CreateFile(char *name, int permission)
{
    int i = 0;

    PINODE temp = Head;

    if ((name == NULL) || (permission == 0) || (permission > 3)) // filter 1)- name is empty or permission is not valid
    {
        return -1;
    }
    if (SUPERBLOCKobj.FreeInode == 0) // Filter 2)- ther is no free inode
    {
        return -2;
    }

    (SUPERBLOCKobj.FreeInode)--;

    if (Get_Inode(name) != NULL) // Filter 3)- file is already present
    {
        return -3;
    }

    while (temp != NULL)
    {
        if (temp->FileType == 0) // it is consider as node is vacant;
        {
            break;
        }
        temp = temp->next;
    }

    while (i < MAXINODE)
    {
        if (UFDTArr[i].ptrfiletable == NULL)
        {
            break;
        }
        i++;
    }

    UFDTArr[i].ptrfiletable = (PFILETABLE)malloc(sizeof(FILETABLE));

    if (UFDTArr[i].ptrfiletable == NULL)
    {
        return -4;
    }

    UFDTArr[i].ptrfiletable->count = 1; // count is always 1;
    UFDTArr[i].ptrfiletable->mode = permission;
    UFDTArr[i].ptrfiletable->readoffset = 0;
    UFDTArr[i].ptrfiletable->writeoffset = 0;

    UFDTArr[i].ptrfiletable->ptrinode = temp;

    strcpy(UFDTArr[i].ptrfiletable->ptrinode->FileName, name); // why stcpy because name is string(fun copy data from string to string)
    UFDTArr[i].ptrfiletable->ptrinode->FileType = REGULAR;
    UFDTArr[i].ptrfiletable->ptrinode->ReferenceCount = 1;
    UFDTArr[i].ptrfiletable->ptrinode->LinkCount = 1;
    UFDTArr[i].ptrfiletable->ptrinode->FileSize = MAXFILESIZE;
    UFDTArr[i].ptrfiletable->ptrinode->FileActualSize = 0;
    UFDTArr[i].ptrfiletable->ptrinode->Permission = permission;
    UFDTArr[i].ptrfiletable->ptrinode->Buffer = (char *)malloc(MAXFILESIZE);

    return i;
}

// ######################################################################################
//
//	Function Name	: 	rm_File
//	Input			: 	char *
//	Output			:   integer
//	Description 	: 	Function delete the created file
//	Author			: 	Suhas Dilip Sankpal
//	Date			:	24 january 2023
//
// ######################################################################################

int rm_File(char *name) // rm_File("Demo.txt")
{
    int fd = 0;

    fd = GetFDFromName(name);
    if (fd == -1)
    {
        return -1;
    }

    (UFDTArr[fd].ptrfiletable->ptrinode->LinkCount)--;

    if (UFDTArr[fd].ptrfiletable->ptrinode->LinkCount == 0)
    {
        UFDTArr[fd].ptrfiletable->ptrinode->FileType = 0;
        free(UFDTArr[fd].ptrfiletable->ptrinode->Buffer);
        free(UFDTArr[fd].ptrfiletable);
    }

    UFDTArr[fd].ptrfiletable = NULL;
    (SUPERBLOCKobj.FreeInode)++;
    printf("File succesfully deleted\n");
}

// ######################################################################################
//
//	Function Name	: 	ReadFile
//	Input			: 	integer, char *, integer
//	Output			: 	integer
//	Output			: 	integer
//	Description 	: 	Function read data from file
//	Author			: 	Suhas Dilip Sankpal
//	Date			:	24 january 2023
//
// ######################################################################################

int ReadFile(int fd, char *arr, int isize)
{
    int read_size = 0;

    if (UFDTArr[fd].ptrfiletable == NULL)
    {
        return -1;
    }

    if (UFDTArr[fd].ptrfiletable->mode != READ && UFDTArr[fd].ptrfiletable->mode != READ + WRITE)
    {
        return -2;
    }

    if (UFDTArr[fd].ptrfiletable->ptrinode->Permission != READ && UFDTArr[fd].ptrfiletable->ptrinode->Permission != READ + WRITE)
    {
        return -2;
    }

    if (UFDTArr[fd].ptrfiletable->readoffset == UFDTArr[fd].ptrfiletable->ptrinode->FileActualSize)
    {
        return -3;
    }

    if (UFDTArr[fd].ptrfiletable->ptrinode->FileType != REGULAR)
    {
        return -4;
    }

    read_size = (UFDTArr[fd].ptrfiletable->ptrinode->FileActualSize) - (UFDTArr[fd].ptrfiletable->readoffset);

    if (read_size < isize)
    {
        strncpy(arr, (UFDTArr[fd].ptrfiletable->ptrinode->Buffer) + (UFDTArr[fd].ptrfiletable->readoffset), read_size);

        UFDTArr[fd].ptrfiletable->readoffset = UFDTArr[fd].ptrfiletable->readoffset + read_size;
    }
    else
    {
        strncpy(arr, (UFDTArr[fd].ptrfiletable->ptrinode->Buffer) + (UFDTArr[fd].ptrfiletable->readoffset), isize);

        UFDTArr[fd].ptrfiletable->readoffset = UFDTArr[fd].ptrfiletable->readoffset + isize;
    }

    return isize;
}

// ######################################################################################
//
//	Function Name	:   WriteFile
//	Input			: 	integer, char *, integer
//	Output			: 	integer
//	Description 	: 	Function write the data into file
//	Author			: 	Suhas Dilip Sankpal
//	Date			:	24 january 2023
//
// ######################################################################################

int WriteFile(int fd, char *arr, int isize)
{
    if ((UFDTArr[fd].ptrfiletable->mode != WRITE) && (UFDTArr[fd].ptrfiletable->mode != READ + WRITE))
    {
        return -1;
    }

    if ((UFDTArr[fd].ptrfiletable->ptrinode->Permission != WRITE) && (UFDTArr[fd].ptrfiletable->ptrinode->Permission != READ + WRITE))
    {
        return -1;
    }

    if (UFDTArr[fd].ptrfiletable->writeoffset == MAXFILESIZE)
    {
        return -2;
    }

    if (UFDTArr[fd].ptrfiletable->ptrinode->FileType != REGULAR)
    {
        return -3;
    }

    strncpy((UFDTArr[fd].ptrfiletable->ptrinode->Buffer) + (UFDTArr[fd].ptrfiletable->writeoffset), arr, isize);

    UFDTArr[fd].ptrfiletable->writeoffset = (UFDTArr[fd].ptrfiletable->writeoffset) + isize;

    UFDTArr[fd].ptrfiletable->ptrinode->FileActualSize = (UFDTArr[fd].ptrfiletable->ptrinode->FileActualSize) + isize;

    return isize;
}

// ######################################################################################
//
//	Function Name	: 	OpenFile
//	Input			: 	char *, integer
//	Output			: 	integer
//	Description 	: 	Function open the existing file
//	Author			: 	Suhas Dilip Sankpal
//	Date			:	24 january 2023
//
// ######################################################################################

int OpenFile(char *name, int mode)
{
    int i = 0;
    PINODE temp = NULL;

    if (name == NULL || mode <= 0)
    {
        return -1;
    }

    temp = Get_Inode(name);
    if (temp == NULL)
    {
        return -2;
    }

    if (temp->Permission < mode)
    {
        return -3;
    }

    while (i < MAXINODE)
    {
        if (UFDTArr[i].ptrfiletable == NULL)
        {
            break;
        }
        i++;
    }

    UFDTArr[i].ptrfiletable = (PFILETABLE)malloc(sizeof(FILETABLE));

    if (UFDTArr[i].ptrfiletable == NULL)
    {
        return -1;
    }

    UFDTArr[i].ptrfiletable->count = 1;
    UFDTArr[i].ptrfiletable->mode = mode;

    if (mode == READ + WRITE)
    {
        UFDTArr[i].ptrfiletable->readoffset = 0;
        UFDTArr[i].ptrfiletable->writeoffset = 0;
    }
    else if (mode == READ)
    {
        UFDTArr[i].ptrfiletable->readoffset = 0;
    }
    else if (mode == WRITE)
    {
        UFDTArr[i].ptrfiletable->writeoffset = 0;
    }

    return i;

    printf("File Opened Successfully\n");
}

// ######################################################################################
//
//	Function Name	: 	CloseFileByName
//	Input			: 	integer
//	Output			: 	none
//	Description 	: 	Function close existing file by its file discriptor
//	Author			: 	Suhas Dilip Sankpal
//	Date			:	24 january 2023
//
// ######################################################################################

void CloseFileByName(int fd)
{
    UFDTArr[fd].ptrfiletable->readoffset = 0;
    UFDTArr[fd].ptrfiletable->writeoffset = 0;
    (UFDTArr[fd].ptrfiletable->ptrinode->ReferenceCount)--;
    printf("File closed succesfully\n");
}

// ######################################################################################
//
//	Function Name	: 	CloseFileByName
//	Input			: 	char *
//	Output			: 	int
//	Description 	: 	Function close existing file by its name
//	Author			: 	Suhas Dilip Sankpal
//	Date			:	24 january 2023
//
// ######################################################################################

int CloseFileByName(char *name)
{
    int i = 0;
    i = GetFDFromName(name);

    if (i == -1)
    {
        return -1;
    }

    UFDTArr[i].ptrfiletable->readoffset = 0;
    UFDTArr[i].ptrfiletable->writeoffset = 0;
    (UFDTArr[i].ptrfiletable->ptrinode->ReferenceCount)--;
    printf("File Closed Succesfully\n");
    return 0;
}

// ######################################################################################
//
//	Function Name	:   CloseAllFile
//	Input			: 	none
//	Output			: 	none
//	Description 	: 	Function close all opened file
//	Author			: 	Suhas Dilip Sankpal
//	Date			:	24 january 2023
//
// ######################################################################################

void CloseAllFile()
{
    int i = 0;

    while (i < MAXINODE)
    {
        if (UFDTArr[i].ptrfiletable != NULL)
        {
            UFDTArr[i].ptrfiletable->readoffset = 0;
            UFDTArr[i].ptrfiletable->writeoffset = 0;
            (UFDTArr[i].ptrfiletable->ptrinode->ReferenceCount)--;
            // break;
        }
        i++;
    }

    printf("All files are closed Succesfully\n");
}

// ######################################################################################
//
//	Function Name	: 	LseekFile
//	Input			: 	integer, integer, integer
//	Output			: 	integer
//	Description 	: 	Function change the offset of file for reading or writing
//	Author			: 	Suhas Dilip Sankpal
//	Date			:	24 january 2023
//
// ######################################################################################

int LseekFile(int fd, int size, int from)
{
    if ((fd < 0) || (from > 2))
    {
        return -1;
    }

    if (UFDTArr[fd].ptrfiletable == NULL)
    {
        return -1;
    }

    if ((UFDTArr[fd].ptrfiletable->mode == READ || UFDTArr[fd].ptrfiletable->mode == READ + WRITE))
    {
        if (from == CURRENT)
        {
            if (((UFDTArr[fd].ptrfiletable->readoffset) + size) > UFDTArr[fd].ptrfiletable->ptrinode->FileActualSize)
            {
                return -1;
            }

            if (((UFDTArr[fd].ptrfiletable->readoffset) + size) < 0)
            {
                return -1;
            }

            (UFDTArr[fd].ptrfiletable->readoffset) = (UFDTArr[fd].ptrfiletable->readoffset) + size;
        }
        else if (from == START)
        {
            if (size > (UFDTArr[fd].ptrfiletable->ptrinode->FileActualSize))
            {
                return -1;
            }

            if (size < 0)
            {
                return -1;
            }

            (UFDTArr[fd].ptrfiletable->readoffset) = size;
        }
        else if (from == END)
        {
            if (((UFDTArr[fd].ptrfiletable->ptrinode->FileActualSize) + size) > MAXFILESIZE)
            {
                return -1;
            }

            if (UFDTArr[fd].ptrfiletable->readoffset + size < 0)
            {
                return -1;
            }

            (UFDTArr[fd].ptrfiletable->readoffset) = (UFDTArr[fd].ptrfiletable->ptrinode->FileActualSize) + size;
        }
    }
    else if (UFDTArr[fd].ptrfiletable->mode == WRITE)
    {
        if (from == CURRENT)
        {
            if (((UFDTArr[fd].ptrfiletable->writeoffset) + size) > MAXFILESIZE)
            {
                return -1;
            }

            if ((UFDTArr[fd].ptrfiletable->writeoffset) + size < 0)
            {
                return -1;
            }

            if (((UFDTArr[fd].ptrfiletable->writeoffset) + size) > (UFDTArr[fd].ptrfiletable->ptrinode->FileActualSize))
            {
                (UFDTArr[fd].ptrfiletable->ptrinode->FileActualSize) = (UFDTArr[fd].ptrfiletable->writeoffset) + size;
                (UFDTArr[fd].ptrfiletable->writeoffset) = (UFDTArr[fd].ptrfiletable->writeoffset) + size;
            }
        }
        else if (from == START)
        {
            if (size > MAXFILESIZE)
            {
                return -1;
            }

            if (size < 0)
            {
                return -1;
            }

            if (size > (UFDTArr[fd].ptrfiletable->ptrinode->FileActualSize))
            {
                UFDTArr[fd].ptrfiletable->ptrinode->FileActualSize = size;
                UFDTArr[fd].ptrfiletable->writeoffset = size;
            }
        }
        else if (from == END)
        {
            if (((UFDTArr[fd].ptrfiletable->ptrinode->FileActualSize) + size) > MAXFILESIZE)
            {
                return -1;
            }

            if (UFDTArr[fd].ptrfiletable->writeoffset + size < 0)
            {
                return -1;
            }

            UFDTArr[fd].ptrfiletable->writeoffset = (UFDTArr[fd].ptrfiletable->ptrinode->FileActualSize) + size;
        }
    }

    printf("Successfully changed\n");
}

// ######################################################################################
//
//	Function Name	: 	ls_file
//	Input			: 	none
//	Output			: 	none
//	Description 	: 	Function display all existing file name
//	Author			: 	Suhas Dilip Sankpal
//	Date			:	24 january 2023
//
// ######################################################################################

void ls_file()
{
    int i = 0;
    PINODE temp = Head;

    if (SUPERBLOCKobj.FreeInode == MAXINODE)
    {
        printf("ERROR: There are no files\n");
        return;
    }

    printf("\nFile Name\tInode Number\tFile Size\tLink Count\n");
    printf("--------------------------------------------------------\n");

    while (temp != NULL) //(loop travel MAXINODE - FreeInode)
    {
        if (temp->FileType != 0)
        {
            printf("%s\t\t %d\t\t %d\t\t %d\n", temp->FileName, temp->InodeNumber, temp->FileActualSize, temp->LinkCount);
        }

        temp = temp->next;
    }
    printf("---------------------------------------------------------\n");
}

// ######################################################################################
//
//	Function Name	: 	fstat_file
//	Input			: 	integer
//	Output			: 	integer
//	Description 	: 	Function display statistical information of file by using file discriptor
//	Author			: 	Suhas Dilip Sankpal
//	Date			:	24 january 2023
//
// ######################################################################################

int fstat_file(int fd)
{
    PINODE temp = Head;

    if (fd < 0)
    {
        return -1;
    }

    if (UFDTArr[fd].ptrfiletable == NULL)
    {
        return -2;
    }

    temp = UFDTArr[fd].ptrfiletable->ptrinode;

    printf("\n------Statistical Information about file------\n");
    printf("File Name : %s\n", temp->FileName);
    printf("Inode Number : %d\n", temp->InodeNumber);
    printf("File Size : %d\n", temp->FileSize);
    printf("Actual File size : %d\n", temp->FileActualSize);
    printf("Link Count : %d\n", temp->LinkCount);
    printf("Referance Count : %d\n", temp->ReferenceCount);

    if (temp->Permission == 1)
    {
        printf("File Permission:Read Only\n");
    }
    else if (temp->Permission == 2)
    {
        printf("File Permission : Write Only\n");
    }
    else if (temp->Permission == 3)
    {
        printf("File Permission : Read & Write \n");
    }
    printf("-------------------------------------------------\n");

    return 0;
}

// ######################################################################################
//
//	Function Name	: 	stat_file
//	Input			: 	char *
//	Output			: 	integer
//	Description 	: 	Function display statistical information of file by using file name
//	Author			: 	Suhas Dilip Sankpal
//	Date			:	24 january 2023
//
// ######################################################################################

int stat_file(char *name)
{
    PINODE temp = Head;

    if (name == NULL)
    {
        return -1;
    }

    while (temp != NULL)
    {
        if (strcmp(name, temp->FileName) == 0)
        {
            break;
        }

        temp = temp->next;
    }

    if (temp == NULL)
    {
        return -2;
    }

    printf("\n------Statistical Information about file------\n");
    printf("File Name : %s\n", temp->FileName);
    printf("Inode Number : %d\n", temp->InodeNumber);
    printf("File Size : %d\n", temp->FileSize);
    printf("Actual File size : %d\n", temp->FileActualSize);
    printf("Link Count : %d\n", temp->LinkCount);
    printf("Referance Count : %d\n", temp->ReferenceCount);

    if (temp->Permission == 1)
    {
        printf("File Permission:Read Only\n");
    }
    else if (temp->Permission == 2)
    {
        printf("File Permission : Write Only\n");
    }
    else if (temp->Permission == 3)
    {
        printf("File Permission : Read & Write \n");
    }
    printf("-------------------------------------------------\n");

    return 0;
}

// ######################################################################################
//
//	Function Name	: 	truncate
//	Input			: 	char *
//	Output			: 	integer
//	Description 	: 	Function delete all data from the file
//	Author			: 	Suhas Dilip Sankpal
//	Date			:	24 january 2023
//
// ######################################################################################

int truncate_File(char *name)
{
    int fd = GetFDFromName(name);

    if (fd == -1)
    {
        return -1;
    }

    // used to fill block of memory(pointer is void pointer)
    memset(UFDTArr[fd].ptrfiletable->ptrinode->Buffer, 0, 1024); //(starting addr,value filled,No of bytes to fill)
    UFDTArr[fd].ptrfiletable->readoffset = 0;
    UFDTArr[fd].ptrfiletable->writeoffset = 0;
    UFDTArr[fd].ptrfiletable->ptrinode->FileActualSize = 0;
    printf("Data Succesfully Removed\n");
}

// ######################################################################################
//
//	Function Name	: 	main
//	Input			: 	none
//	Output			: 	integer
//	Description 	: 	Entry point function (from here Execution of application will start )
//	Author			: 	Suhas Dilip Sankpal
//	Date			:	24 january 2023
//
// ######################################################################################

int main()
{
    char *ptr = NULL;
    int ret = 0, fd = 0, count = 0;
    char command[4][80], str[80], arr[1024];

    InitialiseSupperBlock();
    CreateDILB();

    while (1)
    {
        fflush(stdin);
        strcpy(str, "");

        printf("\nMarvellous VFS :> ");

        fgets(str, 80, stdin); // scanf("%[^'\n']s",str);

        count = sscanf(str, "%s %s %s %s", command[0], command[1], command[2], command[3]);

        if (count == 1)
        {
            if (strcmp(command[0], "ls") == 0) // stricmp is used to compare string case insensitive
            {
                ls_file();
                continue;
            }
            else if (strcmp(command[0], "closeall") == 0)
            {
                CloseAllFile();
                printf("All file closed successfully\n");
                continue;
            }
            else if (strcmp(command[0], "clear") == 0)
            {
                system("clear");
                continue;
            }
            else if (strcmp(command[0], "help") == 0)
            {
                DisplayHelp();
                continue;
            }
            else if (strcmp(command[0], "exit") == 0)
            {
                printf("Terminating the marvellous virtual file system\n");
                break;
            }
            else
            {
                printf("\n ERROR: Command not found !!!\n");
                continue;
            }
        }
        else if (count == 2)
        {
            if (strcmp(command[0], "stat") == 0)
            {
                ret = stat_file(command[1]);

                if (ret == -1)
                {
                    printf("ERROR : Incorrect parameter\n");
                }
                if (ret == -2)
                {
                    printf("ERROR: There is no such file\n");
                    continue;
                }
            }
            else if (strcmp(command[0], "fstat") == 0)
            {
                ret = fstat_file(atoi(command[1]));

                if (ret == -1)
                {
                    printf("ERROR: Incorrect paramater\n");
                }
                if (ret == -2)
                {
                    printf("ERROR:There is no such file\n");
                    continue;
                }
            }
            else if (strcmp(command[0], "close") == 0)
            {
                ret = CloseFileByName(command[1]);

                if (ret == -1)
                {
                    printf("ERROR: Incorrect paramater\n");
                    continue;
                }
            }
            else if (strcmp(command[0], "rm") == 0)
            {
                ret = rm_File(command[1]);

                if (ret == -1)
                {
                    printf("ERROR: Incorrect paramater\n");
                    continue;
                }
            }
            else if (strcmp(command[0], "man") == 0)
            {
                man(command[1]);
                continue;
            }
            else if (strcmp(command[0], "write") == 0)
            {
                fd = GetFDFromName(command[1]);

                if (fd == -1)
                {
                    printf("ERROR:Incorrect parameter\n");
                    continue;
                }

                printf("Enter the data:\n");
                scanf("%[^'\n']s", arr);

                ret = strlen(arr);

                if (ret == 0)
                {
                    printf("ERROR:Incorrect parameter\n");
                    continue;
                }

                ret = WriteFile(fd, arr, ret); //(3,address,5)

                if (ret == -1)
                {
                    printf("ERROR: Permission denied\n");
                }
                if (ret == -2)
                {
                    printf("ERROR: There is no sufficient memory to write\n");
                }
                if (ret == -3)
                {
                    printf("ERROR: It is not regular file\n");
                }
                if (ret > 0)
                {
                    printf("Sucessfully : %d bytes written\n", ret);
                }
            }
            else if (strcmp(command[0], "truncate") == 0)
            {
                ret = truncate_File(command[1]);

                if (ret == -1)
                {
                    printf("ERROR: Incorrect parameter\n");
                }
            }
            else
            {
                printf("\nERROR:Command not found!!!\n");
                continue;
            }
        }
        else if (count == 3)
        {

            if (strcmp(command[0], "create") == 0)
            {
                // create    Demo.txt    3          on terminal
                // ret = CreateFile(Demo.txt,      3);
                ret = CreateFile(command[1], atoi(command[2])); // atoi Ascii to Integer (used to convert data from string to integer)

                if (ret >= 0)
                {
                    printf("File is successfully created with file descriptor: %d\n", ret);
                }
                if (ret == -1)
                {
                    printf("ERROR:Incorrect parameter\n");
                }
                if (ret == -2)
                {
                    printf("ERROR:There is no inodes\n");
                }
                if (ret == -3)
                {
                    printf("File is already exists\n");
                }
                if (ret == -4)
                {
                    printf("ERROR: Memory allocation failure\n");
                }

                continue;
            }
            else if (strcmp(command[0], "open") == 0)
            {
                ret = OpenFile(command[1], atoi(command[2]));

                if (ret >= 0)
                {
                    printf("File is successfully opened with file descriptor: %d\n", ret);
                }
                if (ret == -1)
                {
                    printf("ERROR:Incorrect parameter\n");
                }
                if (ret == -2)
                {
                    printf("ERROR:File not present\n");
                }
                if (ret == -3)
                {
                    printf("Permission denied\n");
                }

                continue;
            }
            else if (strcmp(command[0], "read") == 0)
            {
                fd = GetFDFromName(command[1]);

                if (fd == -1)
                {
                    printf("ERROR: Incorrect parameter\n");
                    continue;
                }

                ptr = (char *)malloc(sizeof(atoi(command[2])) + 1);

                if (ptr == NULL)
                {
                    printf("ERROR: Memory allocation failure\n");
                    continue;
                }

                ret = ReadFile(fd, ptr, atoi(command[2]));

                if (ret == -1)
                {
                    printf("ERROR: File not existing\n");
                }
                if (ret == -2)
                {
                    printf("ERROR: Permission denied\n");
                }
                if (ret == -3)
                {
                    printf("ERROR: Reached at the end of file\n");
                }
                if (ret == -4)
                {
                    printf("ERROR: It is not regular file\n");
                }
                if (ret == 0)
                {
                    printf("ERROR: File empty\n");
                }
                if (ret > 0)
                {
                    write(2, ptr, ret);
                }

                continue;
            }
            else
            {
                printf("\nERROR : Command not found!!!\n");
                continue;
            }
        }
        else if (count == 4)
        {
            if (strcmp(command[0], "lseek") == 0)
            {
                fd = GetFDFromName(command[1]);

                if (fd == -1)
                {
                    printf("ERROR:Incorrect parameter\n");
                    continue;
                }

                ret = LseekFile(fd, atoi(command[2]), atoi(command[3]));

                if (ret == -1)
                {
                    printf("ERROR:Unable to prform lseek\n");
                }
            }
            else
            {
                printf("\nERROR:Command not found!!!\n");
                continue;
            }
        }
        else
        {
            printf("\nERROR:Command not found!!!\n");
            continue;
        }
    }
    return 0;
}