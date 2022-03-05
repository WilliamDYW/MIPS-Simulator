#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <cstdlib>
#include <cctype>
#include <cmath>
#include <malloc.h>
#include "A2Asm.h"
using namespace std;
int allLabels;
struct Dict InsDict[100];
string registers[32]={
"$zero","$at","$v0","$v1","$a0","$a1","$a2","$a3",
"$t0","$t1","$t2","$t3","$t4","$t5","$t6","$t7",
"$s0","$s1","$s2","$s3","$s4","$s5","$s6","$s7",
"$t8","$t9","$k0","$k1","$gp","$sp","$fp","$ra"
}; 
string instructions[71]={
"sll","srl","sra","sllv","srlv","srav","jr","jalr","mfhi","mthi","mflo",
"mtlo","mult","multu","div","divu","add","addu","sub","subu","and","or",
"xor","nor","slt","sltu","tge","tgeu","tlt","tltu","teq","tne","bltz","bgez",
"tgei","tgeiu","tlti","tltiu","teqi","tnei","bltzal","bgezal","beq","bne",
"blez","bgtz","addi","addiu","slti","sltiu","andi","ori","xori","lui","lb",
"lh","lwl","lw","lbu","lhu","lwr","sb","sh","swl","sw","swr","ll","sc","j","jal","syscall" 
};
int BinToDec(string bin)
{
	int dec=0;
	while(bin!="")
	{
		dec*=2;
		if(bin[0]=='1')dec++;
		bin=bin.substr(1);
	}
	return dec;
}
//except r, false means no room for data, vise versa
//r.shamt/rs/t/d: false=0,true=empty 
int disassemble(string filename,char *memory,bool data)
{
	string ins,numbers,code,line;
	struct regular r;
	struct immediate i;
	int insnum;
	ifstream infile;
	stringstream buffer;
	allLabels=PreScan(filename.c_str(),data);
	infile.open(filename.c_str());
	if(data)while(getline(infile,line))if(line.substr(0,5)==".text")break;
	while(!infile.fail())
	{
		infile>>ins;
		r=rinitialize();
		i=iinitialize();
		insnum=InsIdentify(ins);
		switch(insnum)
		{
			case(0):	r.funct-=2;
			case(1):	r.funct-=1;
			case(2):	r.funct-=1;r.shamt=true;r.rs=false;
			case(3):	r.funct-=2;
			case(4):	r.funct-=1;
			case(5):	r.funct-=1;
			case(6):	r.funct-=1;if(r.funct==53)r.rd=false;
			case(7):	r.funct-=7;
			case(8):	r.funct-=1;
			case(9):	r.funct-=1;if(r.funct==53){//mthi
				getline(infile,numbers);code=mt(17,numbers);break;
			}
			case(10):	r.funct-=1;
			case(11):	r.funct-=5;if(r.funct==49){//mtlo
				getline(infile,numbers);code=mt(19,numbers);break;
			}if(r.funct>=38)r.rt=false;if(r.funct>=46)r.rs=false;
			case(12):	r.funct-=1;
			case(13):	r.funct-=1;
			case(14):	r.funct-=1;
			case(15):	r.funct-=5;if(r.funct>=46)r.rd=false;
			case(16):	r.funct-=1;
			case(17):	r.funct-=1;
			case(18):	r.funct-=1;
			case(19):	r.funct-=1;
			case(20):	r.funct-=1;
			case(21):	r.funct-=1;
			case(22):	r.funct-=1;
			case(23):	r.funct-=3;
			case(24):	r.funct-=1;
			case(25):	r.funct-=5;
			case(26):	r.funct-=1;
			case(27):	r.funct-=1;
			case(28):	r.funct-=1;
			case(29):	r.funct-=1;
			case(30):	r.funct-=2;
			case(31):	if(r.funct>=48)r.rd=false;getline(infile,numbers);code=reg(r,numbers);break;
			case(32):	i.rdnum-=1;
			case(33):	i.rdnum-=7;
			case(34):	i.rdnum-=1;
			case(35):	i.rdnum-=1;
			case(36):	i.rdnum-=1;
			case(37):	i.rdnum-=1;
			case(38):	i.rdnum-=2;
			case(39):	i.rdnum-=2;
			case(40):	i.rdnum-=1;
			case(41):	i.op-=3;i.rd=false;
			case(42):	i.op-=1;
			case(43):	i.op-=1;
			case(44):	i.op-=1;
			case(45):	i.op-=1;if(i.op>=54){
				i.rd=false;
				i.rdnum=0;
			}
			case(46):	i.op-=1;
			case(47):	i.op-=1;
			case(48):	i.op-=1;
			case(49):	i.op-=1;
			case(50):	i.op-=1;
			case(51):	i.op-=1;
			case(52):	i.op-=1;
			case(53):	i.op-=17;if(i.op==39)i.rs=false;
			case(54):	i.op-=1;
			case(55):	i.op-=1;
			case(56):	i.op-=1;
			case(57):	i.op-=1;
			case(58):	i.op-=1;
			case(59):	i.op-=1;
			case(60):	i.op-=2;
			case(61):	i.op-=1;
			case(62):	i.op-=1;
			case(63):	i.op-=1;
			case(64):	i.op-=3;
			case(65):	i.op-=2;
			case(66):	i.op-=8;
			case(67):	getline(infile,numbers);code=imm(i,numbers);break;
			case(68):	getline(infile,numbers);code=j(2,numbers);break;
			case(69):	getline(infile,numbers);code=j(3,numbers);break;
			case(70): 	code="00000000000000000000000000001100";
			default: if(LabelIdentify(ins.substr(0,ins.length()-1))!=-1)break;
					 else getline(infile,numbers);break;
		}
		if(code!="")
		{
			for(int i=0;i<4;i++)*memory++=BinToDec(code.substr(8*i,8)); 
			code="";
			PC++;
		} 
		ins="";
	}
	return 0;
}
int RegisterIdentify(string r)
{
	for(int i=0;i<32;i++)if(registers[i]==r)return i;
	return -1;
}
int LabelIdentify(string r)
{
	for(int i=allLabels;i>=0;i--)if(r==InsDict[i].Label)return InsDict[i].PC;
	return -1;
}
int Immediatiate(string rr,bool j)
{
	int result=0,pos,temp;
	if((pos=rr.find('('))!=-1)
	{
		rr[pos]=' ';
		rr[rr.find(')')]=' ';
	}
	istringstream room (rr);
	string judge;
	while(!room.fail())
	{
		room>>judge>>ws;
		if(!room.fail())
		if(judge[0]=='$')result+=result>=0?65536*(RegisterIdentify(judge)+1):-65536*(RegisterIdentify(judge)+1);
		else if((judge[0]>='0'&&judge[0]<='9'))result+=atoi(judge.c_str());
		else if(judge[0]=='-')result=-(result-atoi(judge.c_str()));
		else 
		{
//cout<<PC<<endl;
			temp=LabelIdentify(judge)-PC-1;
//cout<<temp<<endl;
			result=j?LabelIdentify(judge):((temp>=0)?result+temp:-result+temp);
		}
	}
	return result;
}
int InsIdentify(string ins)
{
	for(int i=0;i<71;i++)if(instructions[i]==ins)return i;
	return -1;
}
int PreScan(const char* file,bool data)
{
	ifstream infile;
	int location,Now=0,Nownum=0;
	infile.open(file);
	string line;
	if(data)while(getline(infile,line))if(line.substr(0,5)==".text")break;
	while(getline(infile,line))
	{
		if((location=line.find("#"))!=-1)line=line.substr(0,location);
		if((location=line.find(":"))!=-1)
		{
			InsDict[Nownum].PC=Now;
			InsDict[Nownum].Label=line.substr(0,location);
			line=line.substr(location+1);
			Nownum++;
		}
		while(isspace(line[0]))line=line.substr(1);
		if(line!="")Now+=1;
	}
	infile.close();
	return Nownum-1;
}
struct regular rinitialize()
{
	struct regular r;
	r.rs=true;
	r.rt=true;
	r.rd=true;
	r.shamt=false;
	r.funct=54;
	return r;
}
struct immediate iinitialize()
{
	struct immediate i;
	i.op=56;
	i.rdnum=17;
	i.rd=true;
	i.rs=true;
	return i;
}
string handle(string rr)
{
	int pos;
	if((pos=rr.find("#"))!=EOF)rr=rr.substr(0,pos);
	while((pos=rr.find(","))!=EOF)rr[pos]=' ';
	return rr;
}
string reg(struct regular r,string numbers)
{
	int intcode[4]={0};
	string code="000000",rr;
	numbers=handle(numbers);
	istringstream coderoom (numbers);
	if((r.funct>=4&&r.funct<=7))for(int i=2;i>=0;i--)//sxxv 
	{
		coderoom>>rr;
		intcode[i]=RegisterIdentify(rr);
	}
	else if(r.funct==9)for(int i=0;i<2;i++)//jalr
	{
		coderoom>>rr;
		intcode[2*i]=RegisterIdentify(rr);
	}
	else while(true)
	{
		coderoom>>rr;
		if(r.rd)
		{
			intcode[2]=RegisterIdentify(rr);
			r.rd=false;
		}
		else if(r.rs)
		{
			intcode[0]=RegisterIdentify(rr);
			r.rs=false;
		}
		else if(r.rt)
		{
			intcode[1]=RegisterIdentify(rr);
			r.rt=false;
		}
		else 
		{
			if(r.shamt)intcode[3]=atoi(rr.c_str());
			break;
		}
	}
	for(int i=0;i<4;i++)code+=DecToBin(intcode[i],5);
	return code+DecToBin(r.funct,6);
}
string imm(struct immediate i,string numbers)
{
	int intcode;
	string code=DecToBin(i.op,6),rr;
	numbers=handle(numbers);
	istringstream coderoom (numbers);
	coderoom>>rr;
	if(i.op==4||i.op==5)//bne&beq
	{
		code+=DecToBin(RegisterIdentify(rr),5);
		coderoom>>rr;
		code+=DecToBin(RegisterIdentify(rr),5);
		coderoom>>rr;
//cout<<code+DecToBin(Immediatiate(rr,false),16)<<endl;
		return code+DecToBin(Immediatiate(rr,false),16);
	}
	else
	{
		intcode=Immediatiate(numbers.substr(numbers.find(rr)+rr.length()),false);
		if(!i.rs)code+="00000";
		if(abs(intcode)>=65536)code+=DecToBin(abs(intcode)/65536-1,5);
		code+=DecToBin(RegisterIdentify(rr),5);
		if(!i.rd)code+=DecToBin(i.rdnum,5);
		return code+DecToBin(intcode%65536,16);
	}
}
string j(int num,string numbers)
{
	return DecToBin(num,6)+DecToBin(Immediatiate(handle(numbers),true),26);
}
string DecToBin(int intcode,int num)
{
	string code="";
	char ch;
	unsigned short int mark=intcode;
	while(mark)
	{
		code=((mark%2)?'1':'0')+code;
		mark>>=1;
	}
	while(code.length()<num)code="0"+code;
	return code;
}
string CharToBin(char ch)
{
	return DecToBin(int((unsigned char)ch),8);
}
string mt(int i,string numbers)
{
	numbers=handle(numbers);
	string fin;
	istringstream n (numbers);
	n>>fin;
	return "000000"+DecToBin(RegisterIdentify(fin),5)+"000000000000000"+DecToBin(i,6);
}
