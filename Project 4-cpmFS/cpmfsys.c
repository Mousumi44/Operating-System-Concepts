#include "cpmfsys.h"
#include "disksimulator.h"


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

	if((i==9) && ch!=' ')
	{
		(d->name)[i-1] = '\0';
	}
	else
	{
		(d->name)[i-2] = '\0';
	}


	//copy extension from Block0 to DirStruct
	i=9;
	for( ; i<12; i++)
	{
		ch = (e+index*EXTENT_SIZE)[i];
		(d->extension)[i-9] = ch;

		if(ch == ' ') break;

	}

	if(ch == ' ')
	{
		(d->extension)[i-10] = '\0';
	}
	else
	{
		(d->extension)[i-9] = '\0';
	}

	//printf("File Name: %s.%s\n", d->name, d->extension);

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
	uint8_t dirStructBuffer[BLOCK_SIZE];

	//set all blocks free initially
	for (int i = 0; i < NUM_BLOCKS; i++)
	{
		freeList[i]=true;
		
	}

	//load block0 to main memory
	blockRead(dirStructBuffer, (uint8_t) 0);

	DirStructType *cpm_dir;
	for(int i=0; i<Extent_NO;i++ )
	{
		cpm_dir=mkDirStruct(i, dirStructBuffer);

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
	uint8_t dirStructBuffer[BLOCK_SIZE];
	
	//load block0 to main memory
	blockRead(dirStructBuffer, (uint8_t) 0);

	printf("DIRECTORY LISTING\n");

	DirStructType *cpm_dir;
	for(int i=0; i<Extent_NO;i++ )
	{
		cpm_dir=mkDirStruct(i, dirStructBuffer);
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

		if(name[i]!='\0' && j==3)
		{
			
			return false;
		} 

	}

	return true;
}