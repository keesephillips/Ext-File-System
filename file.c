#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "API.h"

int file_cat(char *name)
{
	int i;
	int inodeNum = search_cur_dir(name);
	Inode inode = read_inode(inodeNum);

	if (inodeNum < 0)
	{
		printf("cat failed:  %s does not exist.\n", name);
		return -1;
	} // if
	if (inode.type != file)
	{
		printf("cat failed:  %s is not a file.\n", name);
		return -1;
	} // if

	char *tmp = (char *)malloc(sizeof(char) * 512);

	for (i = 0; i < 15; i++)
	{
		if (i >= inode.blockCount)
		{
			break;
		} // if
		else
		{
			read_disk_block(inode.directBlock[i], tmp);
			printf("%s", tmp);
		} // if

	} // for
	if (inode.size > 7680)
	{
		read_disk_block(inode.indirectBlock, tmp);

		int *indirectBlockMap = (int *)malloc(sizeof(int) * 128);
		indirectBlockMap = (int *)tmp;
		int x[128];
		int numBlock = inode.size / BLOCK_SIZE;
		if (inode.size % BLOCK_SIZE > 0)
		{
			numBlock++;
		} // if

		for (i = 15; i < numBlock; i++)
		{
			x[i - 15] = indirectBlockMap[i - 15];
		} // for

		for (i = 15; i < numBlock; i++)
		{
			read_disk_block(x[i - 15], tmp);
			printf("%s", tmp);
		} // for

	} // if
	printf("\n");
	free(tmp);
	return 0;
} // file_cat(char *name)

int file_remove(char *name)
{
	int i;
	int block, inodeNum, numBlock;
	char *tmp = (char *)malloc(sizeof(char) * 512);

	if (strcmp(name, ".") == 0)
	{
		printf("rm failed: cannot delete %s \n", name);
		return -1;
	} // if
	else if (strcmp(name, "..") == 0)
	{
		printf("rm failed: cannot delete %s \n", name);
		return -1;
	} // if

	inodeNum = search_cur_dir(name);
	if (inodeNum < 0)
	{
		printf("rm failed:  %s does not exist.\n", name);
		return -1;
	} // if
	Inode inode = read_inode(inodeNum);
	if (inode.type != file)
	{
		printf("rm failed:  %s is not a file.\n", name);
		return -1;
	} // if

	for (i = 0; i < 15; i++)
	{
		free_block(inode.directBlock[i]);
	} // if

	if (inode.size > 7680)
	{
		read_disk_block(inode.indirectBlock, tmp);

		int *indirectBlockMap = (int *)malloc(sizeof(int) * 128);
		indirectBlockMap = (int *)tmp;

		int x[128];
		int numBlock = inode.size / BLOCK_SIZE;
		if (inode.size % BLOCK_SIZE > 0)
		{
			numBlock++;
		} // if
		for (i = 15; i < numBlock; i++)
		{
			x[i - 15] = indirectBlockMap[i - 15];
		} // for

		for (i = 15; i < numBlock; i++)
		{
			free_block(x[i - 15]);
		} // for
	}	  // if

	int found = 0;
	for (i = 0; i < curDir.numEntry; i++)
	{
		if (strcmp(curDir.dentry[i].name, name) == 0)
		{
			found = 1;
		} // if
		else if (found == 1)
		{
			curDir.dentry[i - 1] = curDir.dentry[i];
		} // if
	}	  // for
	if (found == 1)
	{
		curDir.numEntry--;
		write_disk_block(curDirBlock, (char *)&curDir);
	} // if

	if (inode.linkCount == 1)
	{
		free_inode(inodeNum);
	} // if
	else
	{
		inode.linkCount--;
		write_inode(inodeNum, inode);
	} // if
	free(tmp);
	return 0;
} // file_remove(char *name)

int hard_link(char *src, char *dest)
{
	int inodeNum = search_cur_dir(src);
	if (inodeNum < 0)
	{
		printf("ln failed:  %s does not exist.\n", src);
		return -1;
	} // if

	Inode inodeSrc = read_inode(inodeNum);
	inodeSrc.linkCount++;
	write_inode(inodeNum, inodeSrc);

	// add a new file into the current directory entry
	strncpy(curDir.dentry[curDir.numEntry].name, dest, strlen(dest));
	curDir.dentry[curDir.numEntry].name[strlen(dest)] = '\0';
	curDir.dentry[curDir.numEntry].inode = inodeNum;
	curDir.numEntry++;
	write_disk_block(curDirBlock, (char *)&curDir);
	printf("Hard link generated:  %s\n", dest);

	return 0;
} // hard_link(char *src, char *dest)

int file_copy(char *src, char *dest)
{
	int i;
	if (strcmp(dest, "..") == 0 || strcmp(dest, ".") == 0)
	{
		printf("cp failed:  %s is an illegal name\n", dest);
		return -1;
	} // if

	int inodeNumSrc = search_cur_dir(src);
	int inodeNumDest = search_cur_dir(dest);

	if (inodeNumDest > 0)
	{
		printf("cp failed:  %s file already exists.\n", dest);
		return -1;
	} // if
	if (inodeNumSrc < 0)
	{
		printf("cp failed:  %s does not exist.\n", src);
		return -1;
	} // if
	Inode inodeSrc = read_inode(inodeNumSrc);
	if (inodeSrc.type != file)
	{
		printf("cp failed:  %s is not a file.\n", src);
		return -1;
	} // if
	if (curDir.numEntry + 1 > MAX_DIR_ENTRY)
	{
		printf("cp failed: directory is full!\n");
		return -1;
	} // if
	if (superBlock.freeInodeCount < 1)
	{
		printf("cp failed: inode is full!\n");
		return -1;
	} // if
	inodeNumDest = get_inode();
	Inode inodeDest = read_inode(inodeNumDest);
	inodeDest.type = file;
	inodeDest.size = inodeSrc.size;
	inodeDest.blockCount = inodeSrc.blockCount;
	inodeDest.linkCount = 1;

	// add a new file into the current directory entry
	strncpy(curDir.dentry[curDir.numEntry].name, dest, strlen(dest));
	curDir.dentry[curDir.numEntry].name[strlen(dest)] = '\0';
	curDir.dentry[curDir.numEntry].inode = inodeNumDest;
	curDir.numEntry++;
	write_disk_block(curDirBlock, (char *)&curDir);

	char *tmp = (char *)malloc(sizeof(char) * 512);

	for (i = 0; i < 15; i++)
	{
		if (i >= inodeSrc.blockCount)
		{
			break;
		} // if
		else
		{
			read_disk_block(inodeSrc.directBlock[i], tmp);
			write_disk_block(inodeDest.directBlock[i], tmp);
		} // if
	}	  // for
	if (inodeSrc.size > 7680)
	{
		int block = get_block();
		if (block == -1)
		{
			printf("File_create error: get_block failed\n");
			return -1;
		} // if

		inodeDest.indirectBlock = block;
		int *indirectBlockMapDest = (int *)malloc(sizeof(int) * 128);

		for (i = 15; i < inodeSrc.blockCount; i++)
		{
			block = get_block();
			if (block == -1)
			{
				printf("File_create error: get_block failed\n");
				return -1;
			}
			// set direct block
			indirectBlockMapDest[i - 15] = block;
			write_disk_block(block, tmp + (i * BLOCK_SIZE));
		} // for

		write_disk_block(inodeDest.indirectBlock, (char *)indirectBlockMapDest);
		read_disk_block(inodeSrc.indirectBlock, tmp);

		int *indirectBlockMap = (int *)malloc(sizeof(int) * 128);
		indirectBlockMap = (int *)tmp;
		int x[128];
		int numBlock = inodeSrc.size / BLOCK_SIZE;
		if (inodeSrc.size % BLOCK_SIZE > 0)
		{
			numBlock++;
		} // if
		for (i = 15; i < numBlock; i++)
		{
			x[i - 15] = indirectBlockMap[i - 15];
		} // for

		for (i = 15; i < numBlock; i++)
		{
			read_disk_block(x[i - 15], tmp);
			write_disk_block(indirectBlockMapDest[i - 15], tmp);
		} // for
	}	  // if
	write_inode(inodeNumDest, inodeDest);
	printf("File copy succeed: from %s to %s\n", src, dest);
	free(tmp);
	return 0;
} // file_copy(char *src, char *dest)

/* =============================================================*/

int file_create(char *name, int size)
{
	int i;
	int block, inodeNum, numBlock;
	if (strcmp(name, "..") == 0 || strcmp(name, ".") == 0)
	{
		printf("cp failed:  %s is an illegal name\n", name);
		return -1;
	} // if

	if (size <= 0 || size > 73216)
	{
		printf("File create failed: file size error\n");
		return -1;
	}

	inodeNum = search_cur_dir(name);
	if (inodeNum >= 0)
	{
		printf("File create failed:  %s exist.\n", name);
		return -1;
	}

	if (curDir.numEntry + 1 > MAX_DIR_ENTRY)
	{
		printf("File create failed: directory is full!\n");
		return -1;
	}

	if (superBlock.freeInodeCount < 1)
	{
		printf("File create failed: inode is full!\n");
		return -1;
	}

	numBlock = size / BLOCK_SIZE;
	if (size % BLOCK_SIZE > 0)
		numBlock++;

	if (size > 7680)
	{
		if (numBlock + 1 > superBlock.freeBlockCount)
		{
			printf("File create failed: data block is full!\n");
			return -1;
		}
	}
	else
	{
		if (numBlock > superBlock.freeBlockCount)
		{
			printf("File create failed: data block is full!\n");
			return -1;
		}
	}

	char *tmp = (char *)malloc(sizeof(int) * size + 1);

	rand_string(tmp, size);
	printf("File contents:\n%s\n", tmp);

	// get inode and fill it
	inodeNum = get_inode();
	if (inodeNum < 0)
	{
		printf("File_create error: not enough inode.\n");
		return -1;
	}

	Inode newInode;

	newInode.type = file;
	newInode.size = size;
	newInode.blockCount = numBlock;
	newInode.linkCount = 1;

	// add a new file into the current directory entry
	strncpy(curDir.dentry[curDir.numEntry].name, name, strlen(name));
	curDir.dentry[curDir.numEntry].name[strlen(name)] = '\0';
	curDir.dentry[curDir.numEntry].inode = inodeNum;
	curDir.numEntry++;
	write_disk_block(curDirBlock, (char *)&curDir);

	// get data blocks
	for (i = 0; i < 15; i++)
	{
		if (i >= numBlock)
			break;
		block = get_block();
		if (block == -1)
		{
			printf("File_create error: get_block failed\n");
			return -1;
		}
		// set direct block
		newInode.directBlock[i] = block;

		write_disk_block(block, tmp + (i * BLOCK_SIZE));
	}

	if (size > 7680)
	{
		// get an indirect block
		block = get_block();
		if (block == -1)
		{
			printf("File_create error: get_block failed\n");
			return -1;
		}

		newInode.indirectBlock = block;
		int indirectBlockMap[128];

		for (i = 15; i < numBlock; i++)
		{
			block = get_block();
			if (block == -1)
			{
				printf("File_create error: get_block failed\n");
				return -1;
			}
			// set direct block
			indirectBlockMap[i - 15] = block;
			write_disk_block(block, tmp + (i * BLOCK_SIZE));
		}
		write_disk_block(newInode.indirectBlock, (char *)indirectBlockMap);
	}

	write_inode(inodeNum, newInode);
	printf("File created. name: %s, inode: %d, size: %d\n", name, inodeNum, size);

	free(tmp);
	return 0;
}

int file_stat(char *name)
{
	char timebuf[28];
	Inode targetInode;
	int inodeNum;

	inodeNum = search_cur_dir(name);
	if (inodeNum < 0)
	{
		printf("file cat error: file is not exist.\n");
		return -1;
	}

	targetInode = read_inode(inodeNum);
	printf("Inode = %d\n", inodeNum);
	if (targetInode.type == file)
		printf("type = file\n");
	else
		printf("type = directory\n");
	printf("size = %d\n", targetInode.size);
	printf("linkCount = %d\n", targetInode.linkCount);
	printf("num of block = %d\n", targetInode.blockCount);
}
