#ifndef A_H
#define A_H
#include <string>
struct regular{bool rs,rt,rd,shamt;int funct;};
struct immediate{int op,rdnum;bool rd,rs;};
struct Dict{int PC;std::string Label;};
int RegisterIdentify(std::string r);//identify registers
int LabelIdentify(std::string r);//identify Labels
int Immediatiate(std::string rr,bool j);
int InsIdentify(std::string ins);
int PreScan(const char* file,bool data);//PreScan: scan labels
struct regular rinitialize();
struct immediate iinitialize();
std::string handle(std::string rr);//handle: remove',',remove comments
std::string reg(struct regular r,std::string numbers);//turn r to code
std::string imm(struct immediate i,std::string numbers);
std::string j(int num,std::string numbers);
std::string DecToBin(int intcode,int num);
std::string mt(int i,std::string numbers);
std::string CharToBin(char ch);
int BinToDec(std::string bin);
int disassemble(std::string filename,char *memory,bool b);
extern struct Dict InsDict[100];
extern int PC;
#endif
