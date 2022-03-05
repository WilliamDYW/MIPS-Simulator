#include <iostream>
#include <malloc.h>
#include <fstream>
#include <sstream>
#include <string>
#include <cctype>
#include <cstdlib>
#include <cmath>
#include "A2Asm.h" 
using namespace std;
const unsigned int textspace=0x0,dataspace=0x500000,dynamic=0x400000,stack=0x5FFFFC;
char *memory=(char*)malloc(6*4*1024*1024),*dataPC=dataspace+memory,*stackPC=memory+stack;
int heapPC=dynamic,Registers[32]={0},PC=0,nowfile=0;
unsigned long int hilo[2]={0}; //hi&lo
FILE * fp[100];
string dataType[5]={".asciiz", ".ascii", ".word", ".half", ".byte"},files[100];
void event(int PC,int errorType)
{
	cerr<<"Error: ";
	switch(errorType)
	{
		case 0:cerr<<"trapped";break;
		case 1:cerr<<"overflow";break;
		case 2:cerr<<"unknown command";break;
		case 3:cerr<<"not align";break;
	}
	cout<<" in Line "<<PC/4<<endl;
	exit(1);
}
string stringpurify(string data)
{

	istringstream iss (data);
	string buffer,result;
	int len;
	while(iss)
	{
		iss>>buffer>>ws;
		if(iss)result+=buffer+' ';
	}
	len=result.length()-3;
	result=result.substr(1,len);
	return result;
}
string intpurify(string data)
{
	int pos;
	istringstream iss (data);
	string result,buffer;
	while(iss)
	{
		iss>>buffer>>ws;
		if(iss)
		result+=(buffer+' ');
	}
	while(pos=result.find(',')+1)result[pos-1]=' ';
	return result;
}
void storeword(int word, char *dataPC,int size)
{
	unsigned int mark=word,buffer=1;
	for(int i=1;i<size;i++)buffer*=256;
	while(buffer)
	{
		*dataPC++=(mark/buffer)%256;
		mark%=buffer;
		buffer/=256;
	}
	while((dataPC-memory)%size)*dataPC++='\0'; 
}
int findata(string datatype)
{
	for(int i=0;i<5;i++)if(dataType[i]==datatype)return i;
	return -1;
}
char* ascii(char *dataPC,string data)
{
	while((dataPC-memory)%4)*dataPC++='\0';
	data=stringpurify(data);
	for(int i=0;data[i]!='\0';i++)
	{
		*dataPC=data[i];
		dataPC++;
	}
	*dataPC='\0';
	return dataPC;
}
char *integer(char* dataPC,string data,int size)
{
	while((dataPC-memory)%size)*dataPC++='\0';
	int word;
	istringstream dataiss (intpurify(data));
	while(dataiss)
	{
		dataiss>>word>>ws;
		if(dataiss)
		{
			storeword(word,dataPC,size);
			dataPC+=size;
		}	
	}
	return dataPC;
}
void datastore(string numbers)
{
	if(numbers[0]=='.')return;
	string datatype,data;
	numbers=numbers.substr(numbers.find(':')+1);
	numbers=numbers.substr(0,numbers.find('#'));
	while(isspace(numbers[0]))numbers=numbers.substr(1);
	if(numbers=="")return;
	istringstream iss (numbers);
	iss>>datatype;
	getline(iss,data);
	int datanum=findata(datatype);
	switch(datanum)
	{
		case 0://.asciiz
		{
			dataPC=ascii(dataPC,data);
			while((dataPC-memory)%4!=3)*dataPC++='\0';
			break;
		}
		case 1://.ascii
		{
			dataPC=ascii(dataPC,data);
			break;
		}
		case 2://.word
		{
			dataPC=integer(dataPC,data,4);
			break;
		}
		case 3://.half
		{
			dataPC=integer(dataPC,data,2);
			break;
		}
		case 4://.byte
		{
			dataPC=integer(dataPC,data,1);
			break;
		}
		default:event(PC,2);
	}
}
string IF(char *Data)
{
	string result="";
	for(int i=0;i<4;i++)result+=CharToBin(*(Data+i));
	return result;
}
struct codeDisintegrate{int op,rs,rt,rd,shamt,funct;short imm;};
struct codeDisintegrate initialize(string mach)
{
	struct codeDisintegrate cm;
	cm.op=BinToDec(mach.substr(0,6));//op 
	cm.rs=BinToDec(mach.substr(6,5));//rs
	cm.rt=BinToDec(mach.substr(11,5));//rt
	cm.rd=BinToDec(mach.substr(16,5));//rd
	cm.shamt=BinToDec(mach.substr(21,5));//shamt
	cm.imm=BinToDec(mach.substr(16));//imm
	cm.funct=BinToDec(mach.substr(26));//funct
	return cm;
}
void hlm(int rs,int rt,bool unsign)
{
	unsigned long long int multiply=unsign?(unsigned int)Registers[rs]*(unsigned int)Registers[rt]:Registers[rs]*Registers[rt]; 
	hilo[0]=multiply/4294967296;
	hilo[1]=multiply%4294967296;
}
void hld(int rs,int rt,bool unsign)
{
	hilo[1]=unsign?(unsigned int)Registers[rs]/(unsigned int)Registers[rt]:Registers[rs]/Registers[rt]; 
	hilo[0]=unsign?(unsigned int)Registers[rs]%(unsigned int)Registers[rt]:Registers[rs]%Registers[rt]; 
}
void syscall()
{
	char *p=memory+Registers[4],*buf=memory+Registers[5];
	char sbuffer[100],ch;
	int buffer=0,i;
	switch(Registers[2])
	{
		case 1:cout<<Registers[4]<<endl;break;
		case 4:cout<<p<<endl;break;
		case 5:cin>>Registers[2];break;
		case 8:cin.get(sbuffer,Registers[5]);for(int i=0;i<Registers[5];i++)*p++=sbuffer[i];do{*p++='\0';}while((p-memory)%4); break;
		case 9:Registers[2]=(unsigned long long int)heapPC;heapPC+=Registers[4];break;//sbrk
		case 10:exit(0);
		case 11:cout<<(char)Registers[4]<<endl;break;
		case 12:cin.get(ch);Registers[2]=ch;break;
		case 13:switch(Registers[5]){
				case 0:fp[nowfile]=fopen(p,"r");
				case 1:fp[nowfile]=fopen(p,"w");
				case 2:fp[nowfile]=fopen(p,"a");
				}
				Registers[4]=nowfile;
				files[nowfile]=p;
				nowfile++;
				break;
		case 14:Registers[4]=fread((memory+Registers[5]),1,Registers[6],fp[Registers[4]]);break;
		case 15:Registers[4]=fwrite((memory+Registers[5]),1,Registers[6]-1,fp[Registers[4]]);break;
		case 16:fclose(fp[Registers[4]]);break;
		case 17:Registers[0]=1;
	}
}
int add(int a,int b,bool unsign=false)
{
	if((long long int)a+b<-2147483648||(long long int)a+b>2147483647)if(!unsign)event(PC,1);
	return a+b;
}
void MEM(int amount,char *address,int rd,bool unsign,bool align);
void WB(int amount,char *address,int rd);
int main()
{
	Registers[28]=dataspace;
	Registers[29]=stack;
	Registers[30]=stack;
	ifstream infile;
	string numbers,filename,mach,complex;
	do
	{
		cout<<"Please enter the testfile name. (e.g. a.asm)"<<endl;
		getline(cin,filename);
		infile.open(filename.c_str());
		if(!infile)cout<<"Invalid name."<<endl;
		else break;
	}
	while(true);
	int MEMR,MEMW;
	bool MEMunsign,MEMalign;
	char *ADD=NULL;
	struct codeDisintegrate cm;
	infile>>complex;
	if(complex==".data")
	do
	{
		getline(infile,numbers);
		datastore(numbers);
	}
	while(numbers!=".text");
	disassemble(filename,memory,complex==".data");
	PC=0;
	while(true)
	{
		MEMR=0;
		MEMW=0;
		MEMunsign=false;
		MEMalign=true;
		mach=IF(memory+PC);
		PC+=4;
		if(mach=="00000000000000000000000000000000")exit(0);
		cm=initialize(mach);
		switch(cm.op)
		{
			case 0: 
			{
				switch(cm.funct){//r
					case 0:Registers[cm.rd]=Registers[cm.rt]<<cm.shamt;break;
					case 2:Registers[cm.rd]=Registers[cm.rt]>=0?Registers[cm.rt]>>cm.shamt:(-Registers[cm.rt])>>cm.shamt;break;
					case 3:Registers[cm.rd]=Registers[cm.rt]>>cm.shamt;break;
					case 4:Registers[cm.rd]=Registers[cm.rt]<<Registers[cm.rs];break;
					case 6:Registers[cm.rd]=Registers[cm.rt]>=0?Registers[cm.rt]>>Registers[cm.rs]:(-Registers[cm.rt])>>Registers[cm.rs];break;
					case 7:Registers[cm.rd]=Registers[cm.rt]>>Registers[cm.rs];break;
					case 8:PC=Registers[cm.rs]*4;break;
					case 9:{
						Registers[cm.rd]=PC/4;
						PC=Registers[cm.rs]*4;
						break;
					}
					case 12:syscall();if(Registers[0]==1)return Registers[4];break;
					case 16:Registers[cm.rd]=hilo[0];break;
					case 17:hilo[0]=Registers[cm.rs];break;
					case 18:Registers[cm.rd]=hilo[1];break;
					case 19:hilo[1]=Registers[cm.rs];break;
					case 24:hlm(Registers[cm.rs],Registers[cm.rt],false);break;
					case 25:hlm(Registers[cm.rs],Registers[cm.rt],true);break;
					case 26:hld(Registers[cm.rs],Registers[cm.rt],false);break;
					case 27:hld(Registers[cm.rs],Registers[cm.rt],true);break;
					case 32:Registers[cm.rd]=add(Registers[cm.rt],Registers[cm.rs]);break;
					case 33:Registers[cm.rd]=add(Registers[cm.rt],Registers[cm.rs],true);break;
					case 34:Registers[cm.rd]=add(-Registers[cm.rt],Registers[cm.rs]);break;
					case 35:Registers[cm.rd]=add(-Registers[cm.rt],Registers[cm.rs],true);break;
					case 36:Registers[cm.rd]=Registers[cm.rt]&Registers[cm.rs];break;
					case 37:Registers[cm.rd]=Registers[cm.rt]|Registers[cm.rs];break;
					case 38:Registers[cm.rd]=(!(Registers[cm.rt]&Registers[cm.rs]))&(Registers[cm.rt]|Registers[cm.rs]);break;
					case 39:Registers[cm.rd]=!(Registers[cm.rt]&Registers[cm.rs]);break;
					case 42:Registers[cm.rd]=(Registers[cm.rt]>=Registers[cm.rs])?1:0;break;
					case 43:Registers[cm.rd]=((unsigned int)Registers[cm.rt]>=(unsigned int)Registers[cm.rs])?1:0;break;
					case 48:if(Registers[cm.rt]<=Registers[cm.rs])event(PC,0);
					case 49:if((unsigned int)Registers[cm.rt]<=(unsigned int)Registers[cm.rs])event(PC,0);
					case 50:if(Registers[cm.rt]>Registers[cm.rs])event(PC,0);
					case 51:if((unsigned int)Registers[cm.rt]>(unsigned int)Registers[cm.rs])event(PC,0);
					case 52:if(Registers[cm.rt]==Registers[cm.rs])event(PC,0);
					case 54:if(Registers[cm.rt]!=Registers[cm.rs])event(PC,0);
					default:event(PC,2);
				}break;
			}
			case 1:
			{
				switch(cm.rt){
					case 0:if(Registers[cm.rs]<0)PC+=4*cm.imm;break;
					case 1:if(Registers[cm.rs]>=0)PC+=4*cm.imm;break;
					case 8:if(Registers[cm.rs]>=cm.imm)event(PC,0);
					case 9:if((unsigned int)Registers[cm.rs]>=(unsigned int)cm.imm)event(PC,0);
					case 10:if(Registers[cm.rs]<cm.imm)event(PC,0);
					case 11:if((unsigned int)Registers[cm.rs]<(unsigned int)cm.imm)event(PC,0);
					case 12:if(Registers[cm.rs]==cm.imm)event(PC,0);
					case 14:if(Registers[cm.rs]!=cm.imm)event(PC,0);
					case 16:if(Registers[cm.rs]<0){
						Registers[31]=PC/4;
						PC+=4*cm.imm;
					}break;
					case 17:if(Registers[cm.rs]>=0){
						Registers[31]=PC/4;
						PC+=4*cm.imm;
					}break;
					default:event(PC,2);
				}	
			}break;
			case 2: PC=cm.imm*4;break;
			case 3:{
				Registers[31]=PC/4;
				PC=cm.imm*4;
				break;
			}
			case 4:if(Registers[cm.rt]==Registers[cm.rs])PC+=4*cm.imm;break;
			case 5:if(Registers[cm.rt]!=Registers[cm.rs])PC+=4*cm.imm;break;
			case 6:if(Registers[cm.rs]<=0)PC+=4*cm.imm;break;
			case 7:if(Registers[cm.rs]>0)PC+=4*cm.imm;break;
			case 8:Registers[cm.rt]=add(cm.imm,Registers[cm.rs]);break;
			case 9:Registers[cm.rt]=add(cm.imm,Registers[cm.rs],true);break;
			case 10:Registers[cm.rt]=(cm.imm>Registers[cm.rs])?1:0;break;
			case 11:Registers[cm.rt]=((unsigned int)cm.imm>(unsigned int)Registers[cm.rs])?1:0;break;
			case 12:Registers[cm.rt]=cm.imm&Registers[cm.rs];break;
			case 13:Registers[cm.rt]=cm.imm|Registers[cm.rs];break;
			case 14:Registers[cm.rt]=(!(cm.imm&Registers[cm.rs]))&(cm.imm|Registers[cm.rs]);break;
			case 15:Registers[cm.rt]=65536*cm.imm;break;
			case 36:MEMunsign=true;//us ch
			case 32:ADD=memory+cm.imm+Registers[cm.rs];MEMR=1;break;//s ch
			case 37:MEMunsign=true;//us s i
			case 33:ADD=memory+cm.imm+Registers[cm.rs];MEMR=2;break;//s s i
			case 48:
			case 35:ADD=memory+cm.imm+Registers[cm.rs];MEMR=4;break;//s i
			case 34:ADD=memory+cm.imm+Registers[cm.rs];MEMR=-(1+(cm.imm+Registers[cm.rs])%4);MEMalign=false;break;
			case 38:ADD=memory+cm.imm+Registers[cm.rs];MEMR=4-(cm.imm+Registers[cm.rs])%4;MEMalign=false;break;
			case 40:ADD=memory+cm.imm+Registers[cm.rs];MEMW=1;break;//s ch
			case 41:ADD=memory+cm.imm+Registers[cm.rs];MEMW=2;break;//s s i
			case 43:ADD=memory+cm.imm+Registers[cm.rs];MEMW=4;break;//s i
			case 42:ADD=memory+cm.imm+Registers[cm.rs];MEMW=-(1+(cm.imm+Registers[cm.rs])%4);MEMalign=false;break;
			case 46:ADD=memory+cm.imm+Registers[cm.rs];MEMW=4-(cm.imm+Registers[cm.rs])%4;MEMalign=false;break;
			case 56:ADD=memory+cm.imm+Registers[cm.rs];MEMW=5;break;//sc
			default:event(PC,2);
		}
		if(MEMR)MEM(MEMR,ADD,cm.rt,MEMunsign,MEMalign);
		else if(MEMW)WB(MEMW,ADD,cm.rt);
	}
	return 0;
}
void MEM(int amount,char *address,int rd,bool unsign,bool align)
{
	if(align)
	{
		if((int)(address-memory)%amount)event(PC,3);
		else
		{
			Registers[rd]=0;
			for(int i=0;i<amount;i++)
			{
				Registers[rd]*=256;
				Registers[rd]+=unsign?(unsigned)*(address+i):*(address+i);
			}
			
		}
	}
	else
	{
		if(amount>0)
		{
			int buffer=0;
			for(int i=0;i<amount;i++)
			{
				buffer*=256;
				buffer+=unsign?(unsigned)*(address+i):*(address+i);
			}
			Registers[rd]=Registers[rd]%(int)(pow(256,4-amount))+buffer*pow(256,amount);
		}
		else
		{
			amount=-amount;
			int buffer=0;
			for(int i=0;i<amount;i++)
			{
				buffer*=256;
				buffer+=unsign?(unsigned)*(address-amount+i+1):*(address-amount+i+1);
			}
			Registers[rd]=(Registers[rd]/(int)(pow(256,amount)))*pow(256,amount)+buffer;
		}
	}
}
void WB(int amount,char *address,int rd)
{
	int mark=Registers[rd];
	if(amount==5)
	{
		amount--;
		mark=1;
	}
	for(int i=0;i<amount;i++)
	{
		*(address+i)=Registers[rd]/(int)pow(256,amount-i-1);
		Registers[rd]%=(int)pow(256,amount-i-1);
	}
	Registers[rd]=mark;
}
