#include<stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <string.h>

int occur(const char str[],int n);

int main(int argc,char* argv[])
{	
	if(argc!=2) printf("Need 1 argument\n");
	else{
		//number of slits
		int n = atoi(argv[1]);
		printf("The number of slits %d\n",n);
		int lines = 320000/n;

		//lines per slit
		char l[33];
		sprintf(l,"%d",lines);
		printf("The number of lines per file is %s\n",l);

		//forking process to divide file into n slits
		pid_t childpid;
		childpid = fork();
		if(childpid==0){ // child process			
			execl("/usr/bin/split","/usr/bin/split","-l",l,"-a","1","-d","input.txt","output",NULL);
		}
		else if (childpid>0){ // parent process
			int status;
			waitpid(childpid, &status, 0);
			printf("Done dividing into %d files\n",n);
		}

		//allocating shared memory
		int SHMSZ = 4*sizeof(long);
		int shmid[n];
		int keys[]={123,345,567,789};
		for(int i=0;i<n;i++){
			shmid[i] = shmget(keys[i], SHMSZ, S_IRUSR | S_IWUSR | IPC_CREAT);
		}
		printf("Done allocating shared memory\n");

		// n worker processes
		for(int i=0;i<n;i++){
			pid_t childpid = fork();
			if(childpid==0){
				const char *words[] = {"CMPS","CCE","ECE"};
				int *ptr=(int *)shmat(shmid[i],NULL,0);
				for(int j=0;j<3;j++){
					ptr[j]=occur(words[j],i);
				}
				ptr[3]=999;
				shmdt(ptr);
				printf("Done with process/file %d\n",i);
				exit(0);
			} else if (childpid>0){
				wait(NULL);
			} else {
				printf("ERROR");
			}
		}

		// create 3 reducer processes
		int sum[3]={0,0,0};
		for(int i=0;i<3;i++){
			pid_t childpid = fork();
			if(childpid==0){
				int *ptr;
				for(int j=0;j<4;j++){
					ptr=(int *)shmat(shmid[j],NULL,0);
					if(ptr[3]==999){
						sum[i]+=ptr[i];
					}
					shmdt(ptr);
					if(j==3){
						if(i==0) printf("Occurence of CMPS is %d\n",sum[0]);
						else if (i==1) printf("Occurence of CCE is %d\n",sum[1]);
						else if (i==2) printf("Occurence of ECE is %d\n",sum[2]);
					}
				}
				exit(0);
			} else if (childpid>0){
				wait(NULL);
			} else {
				printf("ERROR");
			}
		}
	}
	
	return 0;
}

// return number of occurences of each word
int occur(const char str[], int n){

	// exec grep -o -i "CMPS" output0 | wc -l
	char arg[27+strlen(str)];
	strcpy(arg,"grep -o -i ");
	strcat(arg,str);
	strcat(arg," output");
	char k[10];
	sprintf(k,"%d",n);
	strcat(arg,k);
	strcat(arg," | wc -l");
	FILE *cmd;
	cmd = popen(arg, "r");
	if (cmd == NULL) {
		perror("popen");
		exit(EXIT_FAILURE);
		pclose(cmd);
		return -1;
	}
	char result[1024];
	while (fgets(result, sizeof(result), cmd)) {
		pclose(cmd);
		return atoi(result);
	}
}




/*
SAMPLE RUN

ali@ali-VirtualBox:~/Desktop/cmps272/homework/hmk2$ ./WorkerReducer 4
The number of slits 4
The number of lines per file is 80000
Done dividing into 4 files
Done allocating shared memory
Done with process/file 0
Done with process/file 1
Done with process/file 2
Done with process/file 3
Occurence of CMPS is 1857731
Occurence of CCE is 1390714
Occurence of ECE is 1394133
ali@ali-VirtualBox:~/Desktop/cmps272/homework/hmk2$ grep -o -i "CMPS" input.txt | wc -l
1857731
ali@ali-VirtualBox:~/Desktop/cmps272/homework/hmk2$ grep -o -i "CCE" input.txt | wc -l
1390714
ali@ali-VirtualBox:~/Desktop/cmps272/homework/hmk2$ grep -o -i "ECE" input.txt | wc -l
1394133
*/
