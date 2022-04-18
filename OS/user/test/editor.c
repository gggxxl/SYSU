#include "stdio.h"
char path[100];//
char buffer[1024];

void test(){
	int num1,num2;
	printf("[input two number]:");
	scanf("%d %d",&num1,&num2);
	printf("%08d/%8i=%-8.6f\n",num1,num2,(float)num1/(float)num2);
	puts("[enter file path]:");
	scanf("%s",path);
	printf("[try open]:%s\n",path);
	FILE* fp=fopen(path,"wb");
	int totalSize=0;
	if(fp==NULL) printf("[can't open]:%s\n",path);
	else{
		while(1){
			int size=fread(buffer,1,1024,fp);
			if(size==0) break;
			totalSize+=size;
			buffer[size]='\0';
			printf("%s",buffer);
		}
		printf("\n[file size=%d]\n[ADD SOMETHING HERE]:(<q>to quit)\n",totalSize);
		while(1){
			gets(buffer);
			if(strcmp(buffer,"<q>")==0) break;
			printf("writing %d\n",fwrite(buffer,1,strlen(buffer),fp));
		}
		printf("[good bye]!");
		fclose(fp);
	}
}

void delay(){
	static int x=0;
	for(int i=0;i<1000;i++){
		for(int j=0;j<10000;j++){
			x++;
		}
	}
}

int test_INT(){
	for(int i=0;i<100;i++){
		//INT 22H
		__asm__(
			"int $0x22;"
			:
			:
			:
		);
		delay();
	}
}


int main(){
	delay();
	test();
	test_INT();
	return 0;
}