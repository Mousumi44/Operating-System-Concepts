#include "cpmfsys.h"
#include "disksimulator.h"
#include <string.h>


#define EMPTY_BLOCK 0
#define FILE_BLOCK_SIZE 16
#define Extent_NO BLOCK_SIZE/EXTENT_SIZE
bool freeList[NUM_BLOCKS];

//Copy DirStruct/Extent of index from Block0
DirStructType *mkDirStruct(int index,uint8_t *e)
{
	DirStructType *d;
	d = malloc(sizeof(DirStructType));
	
	//copy status from Block0 to DirStruct
	d->status = (e+index*EXTENT_SIZE)[0];

	//printf("DirStruct %x, status=%x\n", index, d->status);

	//copy name from Block0 to DirStruct
	int i=1;
	char ch;
	for(; i<9; i++)
	{
		ch = (e+index*EXTENT_SIZE)[i];
		(d->name)[i-1] = ch;
		if(ch == ' ') break;	
	}
	if(i<9 && ch ==' ')
	{
		d->name[i-1] = '\0';
	}
	if(i==9)
	{
		(d->name)[i] = '\0';
	}
	
	//printf("File Name: %s\n", d->name);

	//copy extension from Block0 to DirStruct
	int extCount=0;
	i=9;
	for( ; i<12; i++)
	{
		ch = (e+index*EXTENT_SIZE)[i];
		(d->extension)[i-9] = ch;
		extCount++;
		if(ch==' ') break;

	}
	if(extCount<3 && ch == ' ')
	{
		(d->extension)[i-10] = '\0';
	}
	if(extCount==3)
	{
		(d->extension)[i-9] = '\0';
	}

	//printf("Extension Name: %s\n", d->extension);

	//copy XL,BC,XH,RC from Block0 to DirStruct
	d->XL = (e+index*EXTENT_SIZE)[12];
	d->BC = (e+index*EXTENT_SIZE)[13];
	d->XH = (e+index*EXTENT_SIZE)[14];
	d->RC = (e+index*EXTENT_SIZE)[15];

	//printf("XL=%x, RC= %x, XH=%x, BC=%x\n", d->XL, d->RC, d->XH, d->BC);

	//copy all 16 bytes of fileblocks from Block0 to DirStruct
	memcpy(d->blocks, e+index*EXTENT_SIZE+FILE_BLOCK_SIZE , FILE_BLOCK_SIZE);

	return d;

}

// function to write contents of a DirStructType struct back to the specified index of the extent
// in block of memory (disk block 0) pointed to by e

void writeDirStruct(DirStructType *d, uint8_t index, uint8_t *e)
{
	////Copy status to Block0
	(e+index*EXTENT_SIZE)[0] = d->status;

	//Copy fileName to Block0
	int i=1;
	while(d->name[i-1]!='\0' && d->name[i-1] != '.')
	{
		(e+index*EXTENT_SIZE)[i] = d->name[i-1];
		i++;
	}

	//if name length<8 pad with ' '
	if(i<9)
	{
		while(i<9)
		{
			(e+index*EXTENT_SIZE)[i]=' ';
			i++;
		}
	}

	//Copy Extension to Block0
	int c=0;
	while(d->extension[c]!='\0')
	{
		(e+index*EXTENT_SIZE)[i] = d->extension[i-1];
		i++;
		c++;

	}

	//if extension length<3 pad with ' '
	if(i<12)
	{
		while(i<12)
		{
			(e+index*EXTENT_SIZE)[i]=' ';
			i++;
		}
	}


	////Copy XL,BC,XH,RC to Block0
	(e+index*EXTENT_SIZE)[12] = d->XL;
	(e+index*EXTENT_SIZE)[13] = d->BC;
	(e+index*EXTENT_SIZE)[14] = d->XH;
	(e+index*EXTENT_SIZE)[15] = d->RC;

	//copy all 16 bytes of file blocks to Block0
	memcpy(e+index*EXTENT_SIZE+FILE_BLOCK_SIZE, d->blocks, FILE_BLOCK_SIZE);
}
void makeFreeList()
{
	uint8_t block0[BLOCK_SIZE];

	//set all blocks free initially
	for (int i = 0; i < NUM_BLOCKS; i++)
	{
		freeList[i]=true;
		
	}

	//load block0 to main memory
	blockRead(block0, (uint8_t) 0);

	DirStructType *cpm_dir;
	for(int i=0; i<Extent_NO;i++ )
	{
		cpm_dir= malloc(sizeof(DirStructType));
		cpm_dir=mkDirStruct(i, block0);

		//compute filesize for used extents
		if(cpm_dir->status!=0xe5)
		{
			
			for(int j=0;j<FILE_BLOCK_SIZE;j++)
			{
				// set freeList[i] == false for the used block
				if(cpm_dir->blocks[j] != EMPTY_BLOCK)
				{
					freeList[(int) cpm_dir->blocks[j]] = false;
				}


			}

		}
	}

}

void printFreeList()
{
	printf("FREE BLOCK LIST: (* means in-use)\n");
	for(int i=0;i<FILE_BLOCK_SIZE;i++)
	{
		printf("%2x: ", i*FILE_BLOCK_SIZE);

		for(int offset=0; offset<FILE_BLOCK_SIZE;offset++)
		{
			// print used block with * 
			if(!freeList[i*FILE_BLOCK_SIZE+offset]) printf("* ");

			//print free block with .
			else printf(". ");

		}
		printf("\n");
	}
}

void cpmDir()
{
	uint8_t block0[BLOCK_SIZE];
	
	//load block0 to main memory
	blockRead(block0, (uint8_t) 0);

	printf("DIRECTORY LISTING\n");

	DirStructType *cpm_dir;
	for(int i=0; i<Extent_NO;i++ )
	{
		cpm_dir= malloc(sizeof(DirStructType));
		cpm_dir=mkDirStruct(i, block0);
		int filesize = 0;

		//compute filesize for used extents
		if(cpm_dir->status!=0xe5)
		{
			//count fully used file blocks
			int NB=0;
			for(int offset=0;offset<FILE_BLOCK_SIZE;offset++)
			{
				if(cpm_dir->blocks[offset] != EMPTY_BLOCK)
				{
					NB++;
				}

			}

			int partial_flieblock = ((int) cpm_dir->RC)*128 + (int)cpm_dir->BC;
			filesize = (NB-1)*BLOCK_SIZE + partial_flieblock;

			printf("%s.%s %d\n", cpm_dir->name,cpm_dir->extension,filesize);

		}
	}
}

//returns true for legal name (8.3 format), false for illegal
bool checkLegalName(char *name)
{
	int i=0;


	for(;i<8 && name[i]!='.' && name[i]!='\0';i++)
	{
		//name only have letter and digit
		if(name[i]<'0' || (name[i]>'9' && name[i] <'A') || (name[i] > 'Z' && name[i]<'a') || name[i] > 'z')
		{

			return false;
		}
	}


	//file name too long
	if(i==8 && name[i]!='\0' && name[i]!='.')
	{		
		return false;
	} 
	

	//check legal extension name	
	else if(name[i]=='.')
	{
		i++;	
		int j=0;
		//start checking extension name after .
		for(;j<3 && name[i]!='\0';j++)
		{
			if(name[i]<'0' || (name[i]>'9' && name[i] <'A') || (name[i] > 'Z' && name[i]<'a') || name[i] > 'z')
			{
					return false;
			}
			
			i++;

		}


		//extension name too long
		if(name[i]!='\0' && j==3)
		{
			
			return false;
		} 

	}

	return true;
}

//returns -1 for illegal name; otherwise returns extent number 0-31
int findExtentWithName(char *name, uint8_t *block0)
{
	//split up the name into file_name, ext_name
	char file_name[9];
	char ext_name[4];
	if(!checkLegalName(name))
	{
		return -1;
	}

	int i=0;
	for(;i<8;i++)
	{
		file_name[i]=name[i];
		if(name[i]=='\0' || name[i]=='.') break;
	}

	//pad '\0' to file name
	file_name[i]='\0';

	//check file name
	//printf("\nfile name: %s\n", file_name);


	//check ext_name
	if(name[i]=='.')
	{
		//ahead pointer to ext char
		i++;

		int extCount=0;
		for(;extCount<3;extCount++)
		{
			ext_name[extCount]=name[i];
			i++;
		}
		ext_name[extCount]='\0';

		//check extension name
		//printf("\nextension: %s\n\n", ext_name);
	}

	//for all extents in block0
	
	for(int j=0;j<Extent_NO;j++)
	{
		//obtain dir j from block0	
		DirStructType *cpm_dir;
		cpm_dir=mkDirStruct(j, block0);
		//printf("strcmp file name: %d \n", strcmp(cpm_dir->name,file_name));
		//printf("strcmp extension: %d \n", strcmp(cpm_dir->name,ext_name));


		if(!strcmp(cpm_dir->name,file_name))
		{
			if(cpm_dir->status==0xe5) return -1;

			//printf("Extent number: %d\n", j);
			return j;
		}

	}

	return -1;;
}

// delete the file named name, and free its disk blocks in the free list 
int  cpmDelete(char * name)
{
	uint8_t block0[BLOCK_SIZE];
	
	//load block0 to main memory
	blockRead(block0, (uint8_t) 0);
	int i;
	i=findExtentWithName(name,block0);
	if(i<0)
	{
		//printf("file not found or illegal file name\n");
		//file not found or illegal file name
		return i;
	}
	else
	{
		//file found
		//printf("file found in extent %d\n",i);
		DirStructType *cpm_dir;
		cpm_dir=mkDirStruct(i, block0);	

		//mark 16 file blocks free in free list
		for(int j=0;j<FILE_BLOCK_SIZE;j++)
		{
			// set freeList[i] == true for the used block
			if(cpm_dir->blocks[j] != EMPTY_BLOCK)
			{
				freeList[(int) cpm_dir->blocks[j]] = true;
			}

			//set all file blocks as empty block
			cpm_dir->blocks[j] = EMPTY_BLOCK;

		}

		//set status as unused
		block0[i*EXTENT_SIZE] = 0xe5;

		//write modified block0 to disk
		blockWrite(block0,(uint8_t) 0);

		//printf("file deleted\n");
		//file found and deleted successfully
		return 1;
	}
}

// modify the extent for file named oldName with newName, and write to the disk
int cpmRename(char *oldName, char * newName)
{
	if(!checkLegalName(newName)) return -1;
	
	uint8_t block0[BLOCK_SIZE];
	
	//load block0 to main memory
	blockRead(block0, (uint8_t) 0);

	int j = findExtentWithName(oldName,block0);

	if(j<0) return -1;

	//load extent with oldName from Block0 to DirStruct 
		// fileExtentwithName finds the index(i) of extent with oldName
		//mkDirStruct copies ith extent to DirStruct
	DirStructType *cpm_dir;
	cpm_dir=mkDirStruct(j, block0);	

	printf("old file name: %s\n",cpm_dir->name);
	printf("old extension: %s\n", cpm_dir->extension);
	
	//modify file_name and extension of the extent of block0 according to newName
		//Step1: separate fileName and extension of newName
		//Step2: modify old fileName to the extent of block0
		//Step3:  modify old extension to the extent of block0

		//Step1
	char file_name[9];
	char ext_name[4];
	int i=0;
	for(;i<8;i++)
	{
		file_name[i]=newName[i];
		if(newName[i]=='\0' || newName[i]=='.') break;
	}

	//pad '\0' to file name
	file_name[i]='\0';

	//check file name
	printf("new file name: %s\n", file_name);


	//check ext_name
	if(newName[i]=='.')
	{
		//ahead pointer to ext char
		i++;

		int extCount=0;
		for(;extCount<3;extCount++)
		{
			ext_name[extCount]=newName[i];
			i++;
		}
		ext_name[extCount]='\0';

		//check extension name
		printf("new extension: %s\n", ext_name);
	}


		//Step2	
	int o=0;
	while(file_name[o]!='\0' && file_name[o] != '.')
	{
		cpm_dir->name[o]=file_name[o];
		o++;
	}

	//if name length<8 pad with ' '
	if(o<8)
	{
		while(o<8)
		{
			cpm_dir->name[o]=' ';
			o++;
		}
	}
	
		//step3
	int c=0;
	while(ext_name[c]!='\0')
	{
		cpm_dir->extension[c]=ext_name[c];
		c++;

	}

	//if extension length<3 pad with ' '
	if(c<3)
	{
		while(c<3)
		{
			cpm_dir->extension[c]=' ';
			c++;
		}
	}
	printf("modified file name: %s\n",cpm_dir->name);
	printf("modified extension: %s\n", cpm_dir->extension);



	//Write Block0 to Disk (blockWrite does this)	
	return 0;

}