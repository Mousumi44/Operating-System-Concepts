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
	//create memory space for dir
	DirStructType *d;
	d = malloc(sizeof(DirStructType));
	
	//obtain status from in-memory Block0
	d->status = (e+index*EXTENT_SIZE)[0];

	//printf("DirStruct %x, status=%x\n", index, d->status);

	//obtain name from in-memory Block0
	//If name length <8, padded by ' '
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

	//obtain dir->extension from in-memory Block0
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
		(d->extension)[i-9] = '\0';
	}
	else
	{
		(d->extension)[i-8] = '\0';
	}

	//printf("Extension Name: %s\n", d->extension);

	//obtain XL,BC,XH,RC from from in-memory Block0
	d->XL = (e+index*EXTENT_SIZE)[12];
	d->BC = (e+index*EXTENT_SIZE)[13];
	d->XH = (e+index*EXTENT_SIZE)[14];
	d->RC = (e+index*EXTENT_SIZE)[15];

	//printf("XL=%x, RC= %x, XH=%x, BC=%x\n", d->XL, d->RC, d->XH, d->BC);

	//obtain all 16 fileblocks from in-memory Block0
	memcpy(d->blocks, e+index*EXTENT_SIZE+FILE_BLOCK_SIZE , FILE_BLOCK_SIZE);


	//return dir
	return d;

}

// function to write contents of a DirStructType struct back to the specified index of the extent
// in block of memory (disk block 0) pointed to by e

void writeDirStruct(DirStructType *d, uint8_t index, uint8_t *e)
{
	//printf("%s\n", d->name);
	//printf("%s\n",d->extension);

	//write status from DirStruct to in-memory Block0

	(e+index*EXTENT_SIZE)[0] = d->status;

	int i;
  	int extCount;

  	//write name from DirStruct to in-memory Block0
    //If name length <8, pad with ' '

  	i = 1;
	for(;d->name[i-1] != '\0';i++) 
	{
	    (e+index*EXTENT_SIZE)[i] = d->name[i-1];

	    if(d->name[i-1] == '.') break;
	}

	// pad with blanks
	if(i<9)
	{
		for(;i<9;i++) 
		{
		    (e+index*EXTENT_SIZE)[i] = ' ';
		}

	}

	//write extension from DirStruct to in-memory Block0

    extCount= 0;
    while(d->extension[extCount] != '\0') 
    {
    	(e+index*EXTENT_SIZE)[i] = d->extension[extCount];
    	i++;  
    	extCount++;
  	}

  	if(i<12)
  	{
  		for(;i<12;i++) 
	    { 
	    	(e+index*EXTENT_SIZE)[i] = ' ';
	  	}

  	}

	//write XL,BC,XH,RC from DirStruct to in-memory Block0
	(e+index*EXTENT_SIZE)[12] = d->XL;
	(e+index*EXTENT_SIZE)[13] = d->BC;
	(e+index*EXTENT_SIZE)[14] = d->XH;
	(e+index*EXTENT_SIZE)[15] = d->RC;

	//write all 16 file blocks from DirStruct to in-memory Block0
	memcpy(e+index*EXTENT_SIZE+FILE_BLOCK_SIZE, d->blocks, FILE_BLOCK_SIZE);

}
void makeFreeList()
{
	uint8_t block0[BLOCK_SIZE];

	//initially set all blocks as free
	for (int i = 0; i < NUM_BLOCKS; i++)
	{
		freeList[i]=true;
		
	}

	//set blocko as occupied
	freeList[0]=false;

	//load block0 to main memory
	blockRead(block0, (uint8_t) 0);

	DirStructType *cpm_dir;

	//for (all i extent in block 0)
	for(int i=0; i<Extent_NO;i++ )
	{
		cpm_dir= malloc(sizeof(DirStructType));
		cpm_dir=mkDirStruct(i, block0);

		//if (dir is used)
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
	
	//read block0 into cpm
	blockRead(block0, (uint8_t) 0);

	printf("DIRECTORY LISTING\n");

	DirStructType *cpm_dir;

	//for all the extent referred by index in cpm_block0
	for(int i=0; i<Extent_NO;i++ )
	{
		cpm_dir= malloc(sizeof(DirStructType));
		cpm_dir=mkDirStruct(i, block0);
		int filesize = 0;

		//if cpm_dir->status is used
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


			//compute partially used file block size
			int partial_flieblock = ((int) cpm_dir->RC)*128 + (int)cpm_dir->BC;

			//compute file length
			filesize = (NB-1)*BLOCK_SIZE + partial_flieblock;

			//print file name and length from cpm
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
		//name can only have letter and digit
		if(name[i]<'0' || (name[i]>'9' && name[i] <'A') || (name[i] > 'Z' && name[i]<'a') || name[i] > 'z')
		{

			return false;
		}
	}


	//check for too long file name
	if(i==8 && name[i]!='\0' && name[i]!='.')
	{		
		return false;
	} 
	

	//check for legal extension name	
	else if(name[i]=='.')
	{
		i++;	
		int j=0;
		
		//extension can only have letter and digit
		for(;j<3 && name[i]!='\0';j++)
		{
			if(name[i]<'0' || (name[i]>'9' && name[i] <'A') || (name[i] > 'Z' && name[i]<'a') || name[i] > 'z')
			{
					return false;
			}
			
			i++;

		}


		//check for too long extension name
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

	//check legal file name, no blanks/punctuation/control chars
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


	//check legal ext_name
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

	//for all dir entries in block0
	
	for(int j=0;j<Extent_NO;j++)
	{
		//obtain dir j from block0	
		DirStructType *cpm_dir;
		cpm_dir=mkDirStruct(j, block0);
		//printf("strcmp file name: %d \n", strcmp(cpm_dir->name,file_name));
		//printf("strcmp extension: %d \n", strcmp(cpm_dir->name,ext_name));


		//if dir->name==file_name
		if(!strcmp(cpm_dir->name,file_name))
		{
			//if dir->status==valid
			if(cpm_dir->status==0xe5) return -1;
			
			//only return the index of the extent
			//printf("Extent number: %d\n", j);
			return j;
		}

	}

	return -1;;
}

// delete the file named name, and free its disk blocks in the free list 
// return -1  if can't be deleted and 1 if delelted successfully
int  cpmDelete(char * name)
{
	uint8_t block0[BLOCK_SIZE];
	
	//load block0 to main memory
	blockRead(block0, (uint8_t) 0);
	int i;
	i=findExtentWithName(name,block0);

	//if file not found or illegal file name
	if(i<0)
	{
		//printf("file not found or illegal file name\n");
		//returns -1
		return i;
	}

	// else if file found
	else
	{
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

			//set all file blocks as empty block so that further can be used 
			cpm_dir->blocks[j] = EMPTY_BLOCK;

		}

		//set status as unused
		block0[i*EXTENT_SIZE] = 0xe5;

		//write modified block0 to disk
		blockWrite(block0,(uint8_t) 0);

		//printf("file deleted\n");
		

		//if extent deleted successfully return 1
		return 1;
	}
}

// modify the extent for file named oldName with newName, and write to the disk
//returns -1 if can't be modified or 0 if modified successfully
int cpmRename(char *oldName, char * newName)
{
	//if newName is illegal return -1 
	if(!checkLegalName(newName)) return -1;
	
	uint8_t *block0=malloc(BLOCK_SIZE);
	
	//load block0 from disk to in-memory
	blockRead(block0, (uint8_t) 0);

	int j = findExtentWithName(oldName,block0);


	//if extent with oldName doesn't exist return -1
	if(j<0) return -1;


	//else if extent with oldName found rename it
	else
	{
			
		//write extent with oldName from in-memory Block0 to DirStruct 
		DirStructType *cpm_dir;
		cpm_dir=mkDirStruct(j, block0);	

		//printf("old file name: %s\n",cpm_dir->name);
		//printf("old extension: %s\n", cpm_dir->extension);
		
		//modify file_name and extension of the extent of block0 according to newName

    	//Step1: separate fileName and extension of newName

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
		//printf("new file name: %s\n", file_name);


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
			//printf("new extension: %s\n", ext_name);
		}


		//Step2: modify oldName to newName in DirStruct

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

		//check whether DirStruct has been modified with newName
		//printf("modified file name: %s\n",cpm_dir->name);
		//printf("modified extension: %s\n", cpm_dir->extension);

		//Step3: write DirStruct to in-memory block0
		writeDirStruct(cpm_dir, j, block0);
		blockWrite(block0,0);



		//Step4: write modified in-memory block0 to disk	
		return 0;

	}

}