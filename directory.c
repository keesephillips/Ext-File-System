#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "API.h"

int dir_make(char *name)
{
	if (strcmp(name, "..") == 0 || strcmp(name, ".") == 0)
	{
		printf("mkdir failed:  %s is an illegal name\n", name);
		return -1;
	} // if
	int inodeNum = search_cur_dir(name);
	if (inodeNum > 0)
	{
		printf("mkdir failed:  %s directory already exists\n", name);
		return -1;
	} // if
	if (curDir.numEntry + 1 > MAX_DIR_ENTRY)
	{
		printf("mkdir create failed: directory is full!\n");
		return -1;
	} // if
	if (superBlock.freeInodeCount < 1)
	{
		printf("mkdir create failed: inode is full!\n");
		return -1;
	} // if

	int currentInode = search_cur_dir(".");
	int newDirInode = get_inode();
	int newDirBlock = get_block();

	Dentry dirDentry;
	Inode newDir;

	newDir.type = directory;
	newDir.size = 1;
	newDir.blockCount = 1;
	newDir.directBlock[0] = newDirBlock;
	write_inode(newDirInode, newDir);

	dirDentry.numEntry = 2;
	strncpy(dirDentry.dentry[0].name, ".", 1);
	dirDentry.dentry[0].name[1] = '\0';
	dirDentry.dentry[0].inode = newDirInode;
	write_disk_block(newDirBlock, (char *)&dirDentry);

	strncpy(dirDentry.dentry[1].name, "..", 2);
	dirDentry.dentry[1].name[3] = '\0';
	dirDentry.dentry[1].inode = currentInode;
	write_disk_block(newDirBlock, (char *)&dirDentry);

	strncpy(curDir.dentry[curDir.numEntry].name, name, strlen(name));
	curDir.dentry[curDir.numEntry].name[strlen(name)] = '\0';
	curDir.dentry[curDir.numEntry].inode = newDirInode;
	curDir.numEntry++;
	write_disk_block(curDirBlock, (char *)&curDir);

	return 0;
} // dir_make(char *name)

int dir_remove(char *name)
{
	int i;
	Dentry dir;
	int inodeNum = search_cur_dir(name);
	Inode inode = read_inode(inodeNum);
	int dirBlock = inode.directBlock[0];
	read_disk_block(dirBlock, (char *)&dir);

	if (inodeNum == 0)
	{
		printf("rmdir failed:  %s cannot remove root directory\n", name);
		return -1;
	} // if
	if (inode.type != directory)
	{
		printf("rmdir failed:  %s is not a directory\n", name);
		return -1;
	} // if
	if (inodeNum < 0)
	{
		printf("rmdir failed:  %s does not exist\n", name);
		return -1;
	} // if
	if (strcmp(name, "..") == 0 || strcmp(name, ".") == 0)
	{
		printf("rmdir failed:  %s cannot be deleted\n", name);
		return -1;
	} // if
	if (dir.numEntry != 2)
	{
		printf("rmdir failed:  %s is not empty\n", name);
		return -1;
	} // if

	for (i = 0; i < 15; i++)
	{
		free_block(inode.directBlock[i]);
	} // if
	char *tmp = (char *)malloc(sizeof(char) * 512);
	if (inode.size > 7680)
	{
		read_disk_block(inode.indirectBlock, tmp);

		int *indirectBlockMap = (int *)malloc(sizeof(int) * 128);
		indirectBlockMap = (int *)tmp;
		int x[128];

		for (i = 15; i < inode.blockCount; i++)
		{
			x[i - 15] = indirectBlockMap[i - 15];
		} // for
		for (i = 15; i < inode.blockCount; i++)
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
	free_inode(inodeNum);
	free(tmp);

	return 0;
} // dir_remove(char *name)

int dir_change(char *name)
{
	int inodeNum = search_cur_dir(name);
	if (inodeNum < 0)
	{
		printf("cd failed:  %s does not exist\n", name);
		return -1;
	} // if

	Inode inode = read_inode(inodeNum);
	if (inode.type != directory)
	{
		printf("cd failed:  %s is not a directory\n", name);
		return -1;
	} // if
	curDirBlock = inode.directBlock[0];
	read_disk_block(curDirBlock, (char *)&curDir);

	return 0;
} // dir_change(char *name)

/* ===================================================== */

int ls()
{
	int i;
	int inodeNum;
	Inode targetInode;

	for (i = 0; i < curDir.numEntry; i++)
	{
		inodeNum = curDir.dentry[i].inode;
		targetInode = read_inode(inodeNum);
		if (targetInode.type == file)
			printf("type: file, ");
		else
			printf("type: dir, ");
		printf("name \"%s\", inode %d, size %d byte\n", curDir.dentry[i].name, curDir.dentry[i].inode, targetInode.size);
	}

	return 0;
}
