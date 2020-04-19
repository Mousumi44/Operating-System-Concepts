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
	d->status = (e+index*EXTENT_SIZE)[0];

	//printf("DirStruct %x, status=%x\n", index, d->status);
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
	d->XL = (e+index*EXTENT_SIZE)[12];
	d->BC = (e+index*EXTENT_SIZE)[13];
	d->XH = (e+index*EXTENT_SIZE)[14];
	d->RC = (e+index*EXTENT_SIZE)[15];

	//printf("XL=%x, RC= %x, XH=%x, BC=%x\n", d->XL, d->RC, d->XH, d->BC);

	//copy all 16 bytes of blocks
	memcpy(d->blocks, e+index*EXTENT_SIZE+FILE_BLOCK_SIZE , FILE_BLOCK_SIZE);

	return d;

}
void writeDirStruct(DirStructType *d, uint8_t index, uint8_t *e)
{
	(e+index*EXTENT_SIZE)[0] = d->status;

	//Copy fileName and extension

	(e+index*EXTENT_SIZE)[12] = d->XL;
	(e+index*EXTENT_SIZE)[13] = d->BC;
	(e+index*EXTENT_SIZE)[14] = d->XH;
	(e+index*EXTENT_SIZE)[15] = d->RC;

	//copy all 16 bytes of blocks
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

	DirStructType *d;
	for(int i=0; i<Extent_NO;i++ )
	{
		d=mkDirStruct(i, dirStructBuffer);

		//compute filesize for used extents
		if(d->status!=0xe5)
		{
			int NB=0;
			for(int offset=0;offset<FILE_BLOCK_SIZE;offset++)
			{
				if(d->blocks[offset] != EMPTY_BLOCK)
				{
					freeList[(int) d->blocks[offset]] = false;
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
			if(freeList[i*FILE_BLOCK_SIZE+offset]==false) printf("* ");
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

	DirStructType *d;
	for(int i=0; i<Extent_NO;i++ )
	{
		d=mkDirStruct(i, dirStructBuffer);
		int filesize = 0;

		//compute filesize for used extents
		if(d->status!=0xe5)
		{
			int NB=0;
			for(int offset=0;offset<FILE_BLOCK_SIZE;offset++)
			{
				if(d->blocks[offset] != EMPTY_BLOCK)
				{
					NB++;
				}

			}
			filesize = (NB-1)*BLOCK_SIZE + ((int) d->RC)*128 + (int)d->BC;
			printf("%s.%s %d\n", d->name,d->extension,filesize);

		}
	}
}