#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<stdlib.h>
#include<pthread.h>
#include<sys/types.h>
#include<unistd.h>
#include<fcntl.h>

// file size is 20.7MB = 20,748,043 Bytes

#define MAX_SPLIT_SIZE 20
#define FILE_SIZE 20748043

int n;
char arr[FILE_SIZE];
int offsets[MAX_SPLIT_SIZE];

int fd = open("input.txt",O_RDONLY);
int count[MAX_SPLIT_SIZE][3];
int sum[]={0,0,0};

int occur(char * str, const char * toSearch);
void *freq(void *index);
void *final_sum(void *index);

int main(int argc,char* argv[])
{	
	if(argc!=2) printf("Need 1 argument\n");
	else{
		n = atoi(argv[1]);
		pthread_t threads[n];
		int splitSize = FILE_SIZE/n + (FILE_SIZE % n != 0); // celing of number
		
		offsets[0]=0;
		for(int i=1;i<n;i++){
			offsets[i]=offsets[i-1]+splitSize;
			printf("This offset is %d\n",offsets[i]);
		}
		
		for(int i=0;i<n;i++){
			for(int j=0;j<3;j++){
				count[i][j]=0;
			}
		}
		
		int x;
		// n threads
		for(int i=0;i<n;i++){
			void *status;
			//printf("Creating thread %d\n",i);
			//x = pthread_create(&threads[i],NULL,printhello,&i);
			x = pthread_create(&threads[i],NULL, freq, &i);
			if(x!=0) exit(-1);
			pthread_join(threads[i], &status);
		}

		// 3 threads
		for(int i=0;i<3;i++){
			void *status;
			x = pthread_create(&threads[i],NULL,final_sum,&i);
			if(x!=0) exit(-1);
			pthread_join(threads[i], &status);
		}
		printf("The frequency of CMPS is %d\n",sum[0]);
		printf("The frequency of CCE is %d\n",sum[1]);
		printf("The frequency of ECE is %d\n",sum[2]);
		//printf("%d\n",count[0][0]+count[0][0]+count[2][0]+count[3][0]);

		pthread_exit(NULL);
	}
	
	return 0;
}

// return number of occurences of each word
int occur(char * str, const char * toSearch)
{
    int i, j, found, count;
    int stringLen, searchLen;

    stringLen = strlen(str);      // length of string
    searchLen = strlen(toSearch); // length of word to be searched

    count = 0;

    for(i=0; i <= stringLen-searchLen; i++)
    {
        /* Match word with string */
        found = 1;
        for(j=0; j<searchLen; j++)
        {
            if(str[i + j] != toSearch[j])
            {
                found = 0;
                break;
            }
        }

        if(found == 1)
        {
            count++;
        }
    }

    return count;
}

void *freq(void *index){
	int offset=offsets[*((int *)index)];
	printf("I am starting in offset %d byte\n",offset);
	lseek(fd,offset,SEEK_SET);
	read(fd,arr,5187011);

	count[*((int *)index)][0]+=occur(arr,"CMPS");
	count[*((int *)index)][1]+=occur(arr,"CCE");
	count[*((int *)index)][2]+=occur(arr,"ECE");
	
	pthread_exit(NULL);
}

void *final_sum(void *index){
	int k = *((int *)index);
	for(int i=0;i<n;i++){
		sum[k]+=count[i][k];
	}
}

/*
ali@ali-VirtualBox:~/Desktop/cmps272/homework/hmk2$ ./Optimized 4
This offset is 5187011
This offset is 10374022
This offset is 15561033
I am starting in offset 0 byte
I am starting in offset 5187011 byte
I am starting in offset 10374022 byte
I am starting in offset 15561033 byte
The frequency of CMPS is 1857729
The frequency of CCE is 1390714
The frequency of ECE is 1394133
*/
