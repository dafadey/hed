#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int getGPUtemp(int i)
{
	char cmd[200];
	char name[200];
	sprintf(name,"GPUtemp.tmp.%d",getpid());
	sprintf(cmd, "nvidia-smi -d TEMPERATURE -i %d -q | grep \"GPU Current Temp\" | sed 's/.*: //' | sed 's/,* C//' > %s", i, name);
	//printf("command is :%s\n", cmd);
	system(cmd);
	FILE* fl=fopen(name,"r");
	int temp;
	fscanf(fl,"%d",&temp);
	fclose(fl);
	sprintf(cmd,"rm -f %s",name);
	system(cmd);
	return temp;
}

