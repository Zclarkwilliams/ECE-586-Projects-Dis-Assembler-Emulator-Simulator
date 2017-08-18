/*
*	File:	 P3Rev1.c
*	Authors: Dawit Amare, Zachary Clark-Williams
*	Dat Last Revised: 08/14/2017
*
*	ECE 586 Computer Architecture
*	Project 3 - Simulator
*
*	Description - 
*		This will simulate a basic 5 stage pipeline system. Stages will be
*	Instruction Fetch(IF), Instruction Decode(ID), Execute(EX), Memory Process(MEM),
*	and, Write Back (WB). This program will track and output: number of dynamic
*	instructions executed, number of clock cycles processor took to execute the
*	program took to execute, average CPI of the whole program, and, type of 
*	instruction execution count (i.e. blt, mult, j, etc.). 
*
*/

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

typedef struct
{
	int OpCode;
	int Stall;
	int LHS;
	int RHS;
	int Source1;
	int Source2;
	int Dest;
	int Mem_Acc;
	int ALU_Output;
	int Execution_Lat;
	
} Pipeline_Register_t;

int main(int argc, char** argv)
{
	// Variable Declarations
	uint32_t mem_space[256];	// 32-bit memory array of 256 length
	uint8_t	 Reg[8];			// 8 Register File
	int Quit_Flag	= 0;		// Quit Flag when finished program
	int PC			= 0;		// Program Counter
	int i			= 0;		// Index Counter
	int Result		= 0;		// Result:File load check
	int BranchFlag	= 0;
	int Branch_Disable_IF	= 0;
	char PrntChr;
	// Tracking counters and data for performance and output
	int INST_CNTR	= 0;
	int MULT_CNTR	= 0;
	int BRNCH_CNTR	= 0;
	int JMP_CNTR	= 0;
	int DIV_CNTR	= 0;
	int ADD_CNTR	= 0;
	int SUB_CNTR	= 0;
	int PRNT_CNTR	= 0;
	int MOD_CNTR	= 0;
	int LDR_CNTR	= 0;
	int STR_CNTR	= 0;
	int CYCLE_CNTR	= 0;
	double CPI_AVG	= 0;
		
	if (argc != 2)
	{// Is file valid to open i.e. is path and name correct?
		printf("\nPlease include 1 argument that is the file path to the FISA memory image. \n\n");
		exit(1);
	}

	// Open File from main pass-by reference value
	FILE* Fp = fopen(argv[1], "rb");
	
	if (Fp == NULL)
	{// Check if file opened is empty or contains data
		printf("\n**Error: File could not be opened.\n\n");
		exit(1);
	}

	for (i=0; i<256; i++)
	{// Read in 256 cell array from image file 
		mem_space[i] = 0;
		Result = fread(&(mem_space[i]), sizeof(uint32_t), 1, Fp);
		if (Result == 0)
		{// Check if Array Value is valid or 0
			printf("\n**Error: Problem reading file after successfully opening.\n\n");
			exit(1);
		}
	}

	for(i = 0; i < 8 ; i++)
	{// Set and Clear Register File
		Reg[i] = 0;
	}
	
	//Initialize Pipeline Registers
	/*
		pr[0] <--- IF stage
		...
		pr[4] <--- Register Write Stage
	*/
	Pipeline_Register_t pr[5];
	for(i = 0; i<5 ; i++)
	{// Set and Clear Pipeline Register Files
		pr[i].OpCode	= -1;				// Opcode
		pr[i].Stall		= 0;							
		pr[i].LHS		= -1;				// ID stage
		pr[i].RHS		= -1;				// ID stage
		pr[i].Source1	= -1;				// IF stage
		pr[i].Source2	= -1;				// IF stage
		pr[i].Dest		= -1;				// usedin EX
		pr[i].Mem_Acc	= -1;
		pr[i].ALU_Output	= -1;			// In execute stage. 
		pr[i].Execution_Lat = -1;
	}
	int Stall_IF_Timer = 0;
	
	while(1)
	{
		/****************************************************************************
		|					Register Writeback (WB) Stage							|
		****************************************************************************/
		
		if(Stall_IF_Timer > 0)
		{// If there is a stall we must skip WB stage and decrement latency counter!
			//printf("\nSTALL WB\n");
			Stall_IF_Timer--;	// Decrement and pass down the pipeline
		}
		else if (pr[4].OpCode >= 32 && pr[4].OpCode <= 82)
		{// No Stall Procceed to execute instruction
			switch(pr[4].OpCode){
				case 32:	// Load To Register From Register Contents Pointing to Memory Addr
							Reg[pr[4].Dest] = pr[4].Mem_Acc;
							break;
				case 33:	// Load To Register From Immediate Value Pointing to Memory Addr
							Reg[pr[4].Dest] = pr[4].Mem_Acc;
							break;
				case 64:	// Add Reg Contents of Second Two Reg's, Store in First Reg
							Reg[pr[4].Dest] = pr[4].ALU_Output;
							break;
				case 65:	// Add Immediate To Second Reg Contents, Store In First Reg
							Reg[pr[4].Dest] = pr[4].ALU_Output;
							break;
				case 66:	// Subtract Reg Contents of Second Two Reg's, Store in First Reg
							Reg[pr[4].Dest] = pr[4].ALU_Output;
							break;
				case 67:	// Subtract Immediate Value From Second Reg Contents, Store In First Reg
							Reg[pr[4].Dest] = pr[4].ALU_Output;
							break;
				case 68:	// Multiply Reg Contents of Second Two Reg's, Store in First Reg
							Reg[pr[4].Dest] = pr[4].ALU_Output;
							break;
				case 69:	// Multiply Immediate And Second Reg Contents, Store In First Reg
							Reg[pr[4].Dest] = pr[4].ALU_Output;
							break;
				case 72:	// Divide Second Reg Contents By Third Reg Contents, Store In First Reg
							Reg[pr[4].Dest] = pr[4].ALU_Output;
							break;
				case 73:	// Divide Second Reg Contents By Immediate Value, Store In First Reg
							Reg[pr[4].Dest] = pr[4].ALU_Output;
							break;
				case 80:	// Modulo Second Reg Contents By Third Reg Contents, Store In First Reg
							Reg[pr[4].Dest] = pr[4].ALU_Output;
							break;
				case 81:	// Modulus Second Reg Contents By Immediate Value, Store In First Reg
							Reg[pr[4].Dest] = pr[4].ALU_Output;
							break;
				default:	// Do Nothing in this stage!
							break;
			}
		//printf("\nWB\tPC:%d\tWB R[%d]:\t%d = %d",PC, pr[4].Dest, Reg[pr[4].Dest], pr[4].ALU_Output);
		}
		CYCLE_CNTR++;
		
		if(Quit_Flag == 1)
		{// Q instruction --> exit program after printing performance data!
			printf("\nDynamic Instruction Count:\t%d", INST_CNTR);
			
			printf("\nPRINT\tInstruction Count:\t%d", PRNT_CNTR);
			printf("\nLOAD\tInstruction Count:\t%d", LDR_CNTR);
			printf("\nSTORE\tInstruction Count:\t%d", STR_CNTR);
			printf("\nADD\tInstruction Count:\t%d", ADD_CNTR);
			printf("\nSUB\tInstruction Count:\t%d", SUB_CNTR);			
			printf("\nMULT\tInstruction Count:\t%d", MULT_CNTR);
			printf("\nDIVIDE\tInstruction Count:\t%d", DIV_CNTR);
			printf("\nMOD\tInstruction Count:\t%d", MOD_CNTR);
			printf("\nJUMP\tInstruction Count:\t%d", JMP_CNTR);
			printf("\nBRANCH\tInstruction Count:\t%d", BRNCH_CNTR);
			
			printf("\nCycles Taken to Execute Program:\t%d", CYCLE_CNTR);
			
			CPI_AVG = INST_CNTR * CYCLE_CNTR /(6 * MULT_CNTR + 2 * BRNCH_CNTR + 2 * JMP_CNTR + 15 * DIV_CNTR + ADD_CNTR + SUB_CNTR + PRNT_CNTR + 15 * MOD_CNTR + LDR_CNTR + STR_CNTR);
			printf("\nProgram Average CPI:\t%f\n", CPI_AVG);
			return 0;
		}
		
		/****************************************************************************
		|						Memory Access (MEM) Stage							|
		****************************************************************************/
		
		if(Stall_IF_Timer > 0)
		{// If there is a stall we must skip execution stage and decrement latency counter!
			 Stall_IF_Timer--;	// Decrement and pass down the pipeline
		}else
		{// No Stall Procceed to execute instruction
			switch (pr[3].OpCode){ // Operation Disassemble Table
				case 32:	// Load To Register From Register Contents Pointing to Memory Addr
							pr[3].Mem_Acc = mem_space[pr[3].LHS];
							break;
				case 33:	// Load To Register From Immediate Value Pointing to Memory Addr
							pr[3].Mem_Acc = mem_space[pr[3].LHS];
							break;
				case 34:	// Store Register Contents To Memory Addr Pointed To By Register Contents
							mem_space[pr[3].RHS] =  pr[3].LHS;
							break;
				case 35:	// Store Immediate Value To Memory Addr Pointed To By Register Contents
							mem_space[pr[3].RHS] =  pr[3].Source1;
							break;
				case 36:	// Store Register Contents To Memory Addr Pointed To By Immediate Address
							mem_space[pr[3].Dest] = pr[3].LHS;
							break;
				case 37:	// Store Immediate Value Oprnd2 into Mem Addr Pointed To By Immediate Value Addr Oprnd1
							mem_space[pr[3].Dest] = pr[3].Source1;
							break;
				default:	// Do Nothing in this stage!
							break;
			}
		}
		CYCLE_CNTR++;
		
		/****************************************************************************
		|							Execute (EX) Stage								|
		****************************************************************************/
		if(Stall_IF_Timer > 0 || BranchFlag == 1)
		{// If there is a stall we must skip execution stage and decrement latency counter!
			if (Stall_IF_Timer > 0)
			{
				Stall_IF_Timer--;	// Decrement and pass down the pipeline
			}
		}
		else{// No Stall Procceed to execute instruction
			switch (pr[2].OpCode){ // Operation Disassemble Table
				case -1: 	//First run through so OpCode initialized to -1
							break;
				case 0:		// Quit	--> exit program!
							Quit_Flag = 1;
							break;
				case 2:		// Print Register Contents As Number
							printf("\n%d", pr[2].Dest);
							//printf("\nPrint Op");
							PRNT_CNTR++;
							break;
				case 3:		// Print As Number Using Immediate
							printf("\n%d", pr[2].Dest);
							PRNT_CNTR++;
							break;
				case 4:		// Print Register Contents As ASCII Character 
							PrntChr = (char) pr[2].Dest;
							printf("\n%c", PrntChr);
							PRNT_CNTR++;
							break;
				case 5:		// Print As ASCII Character Using Immediate
							PrntChr = (char) pr[2].Dest;
							printf("\n%c", PrntChr);
							PRNT_CNTR++;
							break;
				case 32:	// Load To Register From Register Contents Pointing to Memory Addr
							LDR_CNTR++;
							break;
				case 33:	// Load To Register From Immediate Value Pointing to Memory Addr
							LDR_CNTR++;
							break;
				case 34:	// Store Register Contents To Memory Addr Pointed To By Register Contents
							STR_CNTR++;
							break;
				case 35:	// Store Immediate Value To Memory Addr Pointed To By Register Contents
							STR_CNTR++;
							break;
				case 36:	// Store Register Contents To Memory Addr Pointed To By Immediate Address
							STR_CNTR++;
							break;
				case 37:	// Store Immediate Value Oprnd2 into Mem Addr Pointed To By Immediate Value Addr Oprnd1
							STR_CNTR++;
							break;
				case 64:	// Add Reg Contents of Second Two Reg's, Store in First Reg
							pr[2].ALU_Output = (pr[2].LHS + pr[2].RHS) & 255;
							ADD_CNTR++;
							//printf("\n\t%d\t+\t%d\t=\t%d",pr[2].LHS, pr[2].RHS, pr[2].ALU_Output);
							break;
				case 65:	// Add Immediate To Second Reg Contents, Store In First Reg
							pr[2].ALU_Output = (pr[2].LHS + pr[2].RHS) & 255;
							ADD_CNTR++;
							break;
				case 66:	// Subtract Reg Contents of Second Two Reg's, Store in First Reg
							pr[2].ALU_Output = (pr[2].LHS - pr[1].RHS) & 255;
							SUB_CNTR++;
							break;
				case 67:	// Subtract Immediate Value From Second Reg Contents, Store In First Reg
							pr[2].ALU_Output = (pr[2].LHS - pr[2].RHS) & 255;
							SUB_CNTR++;
							break;
				case 68:	// Multiply Reg Contents of Second Two Reg's, Store in First Reg
							pr[2].ALU_Output = (pr[2].LHS * pr[2].RHS) & 255;
							Stall_IF_Timer = 6;	// 6-Cycle Stall Setup
							MULT_CNTR++;
							break;
				case 69:	// Multiply Immediate And Second Reg Contents, Store In First Reg
							pr[2].ALU_Output = (pr[2].LHS * pr[2].RHS) & 255;
							Stall_IF_Timer = 6;	// 6-Cycle Stall Setup
							MULT_CNTR++;
							break;
				case 72:	// Divide Second Reg Contents By Third Reg Contents, Store In First Reg
							pr[2].ALU_Output = pr[2].LHS / pr[2].RHS;
							Stall_IF_Timer = 15;	// 15-Cycle Stall Setup
							DIV_CNTR++;
							break;
				case 73:	// Divide Second Reg Contents By Immediate Value, Store In First Reg
							pr[2].ALU_Output = pr[2].LHS / pr[2].RHS;
							Stall_IF_Timer = 15;	// 15-Cycle Stall Setup
							DIV_CNTR++;
							break;
				case 80:	// Modulo Second Reg Contents By Third Reg Contents, Store In First Reg
							pr[2].ALU_Output = pr[2].LHS % pr[2].RHS;
							Stall_IF_Timer = 15;	// 15-Cycle Stall Setup
							MOD_CNTR++;
							break;
				case 81:	// Modulus Second Reg Contents By Immediate Value, Store In First Reg
							pr[2].ALU_Output = pr[2].LHS % pr[2].RHS;
							Stall_IF_Timer = 15;	// 15-Cycle Stall Setup
							MOD_CNTR++;
							break;
				case 128:	// Jump To Instruction Pointed To By Immediate Value (Operand1)
							PC = pr[2].Dest;
							Stall_IF_Timer = 2;	// 2-Cycle Stall Setup
							JMP_CNTR++;
							BranchFlag = 1;
							printf("\nJump Taken");
							break;
				case 130:	// Branch To Instruction Addr pointed top be Immediate value (Operand1) If Second Reg Contents is Equal to Contents of Third Reg
							if(pr[2].LHS == pr[2].RHS)
							{
								PC = pr[2].Dest;
							}
							Stall_IF_Timer = 2;	// 2-Cycle Stall Setup
							BRNCH_CNTR++;
							BranchFlag = 1;
							break;
				case 131:	// Branch To Instruction Addr pointed top be Immediate value (Operand1) If Second Reg Contents is Equal to Immediate Value
							
							if(pr[2].LHS == pr[2].RHS)
							{
								PC = pr[2].Dest;
								printf("\nBranch Taken");
							}
							Stall_IF_Timer = 2;	// 2-Cycle Stall Setup
							BRNCH_CNTR++;
							BranchFlag = 1;
							break;
				case 132:	// Branch To Instruction Addr pointed top be Immediate value (Operand1) If Second Reg Contents is Less Than Contents of Third Reg 
							if(pr[2].LHS < pr[2].RHS)
							{
								PC = pr[2].Dest;
							}
							Stall_IF_Timer = 2;	// 2-Cycle Stall Setup
							BRNCH_CNTR++;
							BranchFlag = 1;
							break;
				case 133:	// Branch To Instruction Addr pointed top be Immediate value (Operand1) If Second Reg Contents is Less Than Immediate Value
							if(pr[2].LHS < pr[2].RHS)
							{
								PC = pr[2].Dest;
							}
							Stall_IF_Timer = 2;	// 2-Cycle Stall Setup
							BRNCH_CNTR++;
							BranchFlag = 1;
							break;
				case 134:	// Branch To Instruction Addr pointed top be Immediate value (Operand1) If Second Reg Contents is Less Than Or Equal to Contents of Third Reg
							if(pr[2].LHS <= pr[2].RHS)
							{
								PC = pr[2].Dest;
							}
							Stall_IF_Timer = 2;	// 2-Cycle Stall Setup
							BRNCH_CNTR++;
							BranchFlag = 1;
							break;
				case 135:	// Branch To Instruction Addr pointed top be Immediate value (Operand1) If Second Reg Contents is Less Than Or Equal To Immediate Value
							if(pr[2].LHS <= pr[2].RHS)
							{
								PC = pr[2].Dest;
							}
							Stall_IF_Timer = 2;	// 2-Cycle Stall Setup
							BRNCH_CNTR++;
							BranchFlag = 1;
							break;
				case 136:	// Branch To Instruction Addr pointed top be Immediate value (Operand1) If Second Reg Contents is Greater Than Contents of Third Reg
							if(pr[2].LHS > pr[2].RHS)
							{
								PC = pr[2].Dest;
							}
							Stall_IF_Timer = 2;	// 2-Cycle Stall Setup
							BRNCH_CNTR++;
							BranchFlag = 1;
							break;
				case 137:	// Branch To Instruction Addr pointed top be Immediate value (Operand1) If Second Reg Contents is Greater Than Immediate Value
							if(pr[2].LHS > pr[2].RHS)
							{
								PC = pr[2].Dest;
							}
							Stall_IF_Timer = 2;	// 2-Cycle Stall Setup
							BRNCH_CNTR++;
							BranchFlag = 1;
							break;
				case 138:	// Branch To Instruction Addr pointed top be Immediate value (Operand1) If Second Reg Contents is Greater Than Or Equal to Contents of Third Reg
							if(pr[2].LHS >= pr[2].RHS)
							{
								PC = pr[2].Dest;
							}
							Stall_IF_Timer = 2;	// 2-Cycle Stall Setup
							BRNCH_CNTR++;
							BranchFlag = 1;
							break;
				case 139:	// Branch To Instruction Addr pointed top be Immediate value (Operand1) If Second Reg Contents is Greater Than Or Equal To Immediate Value
							if(pr[2].LHS >= pr[2].RHS)
							{
								PC = pr[2].Dest;
							}
							Stall_IF_Timer = 2;	// 2-Cycle Stall Setup
							BRNCH_CNTR++;
							BranchFlag = 1;
							break;
				default:	// If Nothing Matches We Have An **ERROR!!!
							printf("\n**Error: Unrecognizable Operation Code In EX:  %d.\n\n", pr[2].OpCode);
							exit(1);
							break;
			}
			Reg[0] = 0;
			//Stall_IF_Timer--;
			printf("\nEX\tPC:%d\tOP:%d\tDest:%d\tS1:%d\tS2%d\t",PC,pr[2].OpCode, pr[2].Dest, pr[2].Source1, pr[2].Source2);
		}
		CYCLE_CNTR++;
		
		if(BranchFlag == 1)
		{// Set and Clear Pipeline Register Files
			pr[1].OpCode	= -1;				// Opcode
			pr[1].Stall		= 0;							
			pr[1].LHS		= -1;				// ID stage
			pr[1].RHS		= -1;				// ID stage
			pr[1].Source1	= -1;				// IF stage
			pr[1].Source2	= -1;				// IF stage
			pr[1].Dest		= -1;				// usedin EX
			pr[1].Mem_Acc	= -1;
			pr[1].ALU_Output	= -1;			// In execute stage. 
			pr[1].Execution_Lat = -1;
			if (Stall_IF_Timer <= 0)
			{
				BranchFlag = 0;
			}
		}
		
		/****************************************************************************
		|						Instruction Decode (ID) Stage						|
		****************************************************************************/
		
		if(Stall_IF_Timer > 0)
		{// If there is a stall we must skip ID stage and decrement stall counter!
			Stall_IF_Timer--;	// Decrement and send down the pipeline
		}
		else{// No Stall Procceed to execute instruction
			 switch(pr[1].OpCode){
				case -1: 	//First run through so OpCode initialized to -1
							break;
				case 0:		// Quit	--> No Instruction Decode Needed
							break;
				case 2:		// Print Register Contents As Number
							pr[1].Dest = Reg[pr[1].Dest];
							break;
				case 3:		// Print As Number Using Immediate
							break;
				case 4:		// Print Register Contents As ASCII Character 
							pr[1].Dest = Reg[pr[1].Dest];
							break;
				case 5:		// Print As ASCII Character Using Immediate
							break;
				case 32:	// Load To Register From Register Contents Pointing to Memory Addr
							pr[1].LHS = Reg[pr[1].Source1];
							break;
				case 33:	// Load To Register From Immediate Value Pointing to Memory Addr
							pr[1].LHS = pr[1].Source1;
							break;
				case 34:	// Store Register Contents To Memory Addr Pointed To By Register Contents
							pr[1].RHS = Reg[pr[1].Dest];
							pr[1].LHS = Reg[pr[1].Source1];
							break;
				case 35:	// Store Immediate Value To Memory Addr Pointed To By Register Contents
							pr[1].RHS = Reg[pr[1].Dest];
							//pr[1].LHS = pr[1].Source1;
							break;
				case 36:	// Store Register Contents To Memory Addr Pointed To By Immediate Address
							pr[1].LHS = Reg[pr[1].Source1];
							break;
				case 37:	// Store Immediate Value Oprnd2 into Mem Addr Pointed To By Immediate Value Addr Oprnd1
							//pr[1].LHS = pr[1].Source1;
							break;
				case 64:	// Add Reg Contents of Second Two Reg's, Store in First Reg
							pr[1].LHS = Reg[pr[1].Source1];
							pr[1].RHS = Reg[pr[1].Source2];
							//pr[1].Dest = Reg[pr[1].Dest]; 
							break;
				case 65:	// Add Immediate To Second Reg Contents, Store In First Reg
							pr[1].LHS = Reg[pr[1].Source1];
							pr[1].RHS = pr[1].Source2;
							//pr[1].Dest = Reg[pr[1].Dest]; 
							break;
				case 66:	// Subtract Reg Contents of Second Two Reg's, Store in First Reg
							pr[1].LHS = Reg[pr[1].Source1];
							pr[1].RHS = Reg[pr[1].Source2];
							//pr[1].Dest = Reg[pr[1].Dest]; 
							break;
				case 67:	// Subtract Immediate Value From Second Reg Contents, Store In First Reg
							pr[1].LHS = Reg[pr[1].Source1];
							pr[1].RHS = pr[1].Source2;
							//pr[1].Dest = Reg[pr[1].Dest]; 
							break;
				case 68:	// Multiply Reg Contents of Second Two Reg's, Store in First Reg
							// Include Stall of 6 cycles
							pr[1].LHS = Reg[pr[1].Source1];
							pr[1].RHS = Reg[pr[1].Source2];
							//pr[1].Dest = Reg[pr[1].Dest]; 
							break;
				case 69:	// Multiply Immediate And Second Reg Contents, Store In First Reg
							// Include Stall of 6 cycles
							pr[1].LHS = Reg[pr[1].Source1];
							pr[1].RHS = pr[1].Source2;
							//pr[1].Dest = Reg[pr[1].Dest];
							break;
				case 72:	// Divide Second Reg Contents By Third Reg Contents, Store In First Reg
							// Include Stall of 15 cycles
							pr[1].LHS = Reg[pr[1].Source1];
							pr[1].RHS = Reg[pr[1].Source2];
							//pr[1].Dest = Reg[pr[1].Dest];
							break;
				case 73:	// Divide Second Reg Contents By Immediate Value, Store In First Reg
							// Include Stall of 15 cycles
							pr[1].LHS = Reg[pr[1].Source1];
							pr[1].RHS = pr[1].Source2;
							//pr[1].Dest = Reg[pr[1].Dest];
							break;
				case 80:	// Modulo Second Reg Contents By Third Reg Contents, Store In First Reg
							// Include Stall of 15 cycles
							pr[1].LHS = Reg[pr[1].Source1];
							pr[1].RHS = Reg[pr[1].Source2];
							//pr[1].Dest = Reg[pr[1].Dest]; 
							break;
				case 81:	// Modulus Second Reg Contents By Immediate Value, Store In First Reg
							// Include Stall of 15 cycles
							pr[1].LHS = Reg[pr[1].Source1];
							pr[1].RHS = pr[1].Source2;
							//pr[1].Dest = Reg[pr[1].Dest];
							break;
				case 128:	// Jump To Instruction Pointed To By Immediate Value (Operand1)
							Branch_Disable_IF = 1;
							break;
				case 130:	// Branch To Instruction Addr pointed top be Immediate value (Operand1) If Second Reg Contents is Equal to Contents of Third Reg
							pr[1].RHS = Reg[pr[1].Source2];
							pr[1].LHS = Reg[pr[1].Source1];
							Branch_Disable_IF = 1;
							break;
				case 131:	// Branch To Instruction Addr pointed top be Immediate value (Operand1) If Second Reg Contents is Equal to Immediate Value
							pr[1].RHS = pr[1].Source2;
							pr[1].LHS = Reg[pr[1].Source1];
							Branch_Disable_IF = 1;
							break;
				case 132:	// Branch To Instruction Addr pointed top be Immediate value (Operand1) If Second Reg Contents is Less Than Contents of Third Reg 
							pr[1].RHS = Reg[pr[1].Source2];
							pr[1].LHS = Reg[pr[1].Source1];
							Branch_Disable_IF = 1;
							break;
				case 133:	// Branch To Instruction Addr pointed top be Immediate value (Operand1) If Second Reg Contents is Less Than Immediate Value
							pr[1].RHS = pr[1].Source2;
							pr[1].LHS = Reg[pr[1].Source1];
							Branch_Disable_IF = 1;
							break;
				case 134:	// Branch To Instruction Addr pointed top be Immediate value (Operand1) If Second Reg Contents is Less Than Or Equal to Contents of Third Reg
							pr[1].RHS = Reg[pr[1].Source2];
							pr[1].LHS = Reg[pr[1].Source1];
							Branch_Disable_IF = 1;
							break;
				case 135:	// Branch To Instruction Addr pointed top be Immediate value (Operand1) If Second Reg Contents is Less Than Or Equal To Immediate Value
							pr[1].RHS = pr[1].Source2;
							pr[1].LHS = Reg[pr[1].Source1];
							Branch_Disable_IF = 1;
							break;
				case 136:	// Branch To Instruction Addr pointed top be Immediate value (Operand1) If Second Reg Contents is Greater Than Contents of Third Reg
							pr[1].RHS = Reg[pr[1].Source2];
							pr[1].LHS = Reg[pr[1].Source1];
							Branch_Disable_IF = 1;
							break;
				case 137:	// Branch To Instruction Addr pointed top be Immediate value (Operand1) If Second Reg Contents is Greater Than Immediate Value
							pr[1].RHS = pr[1].Source2;
							pr[1].LHS = Reg[pr[1].Source1];
							Branch_Disable_IF = 1;
							break;
				case 138:	// Branch To Instruction Addr pointed top be Immediate value (Operand1) If Second Reg Contents is Greater Than Or Equal to Contents of Third Reg
							pr[1].RHS = Reg[pr[1].Source2];
							pr[1].LHS = Reg[pr[1].Source1];
							Branch_Disable_IF = 1;
							break;
				case 139:	// Branch To Instruction Addr pointed top be Immediate value (Operand1) If Second Reg Contents is Greater Than Or Equal To Immediate Value
							pr[1].RHS = pr[1].Source2;
							pr[1].LHS = Reg[pr[1].Source1];
							Branch_Disable_IF = 1;
							break;
				default:	// If Nothing Matches We Have An **ERROR!!!
							printf("\n**Error: Unrecognizable Operation Code In ID:  %d.\n\n", pr[1].OpCode);
							exit(1);
							break;
			}
		printf("\nID\tPC:%d\tOP:%d\tDest:%d\tS1:%d\tS2%d\t",PC,pr[0].OpCode, pr[0].Dest, pr[0].LHS, pr[0].RHS);
		}
		CYCLE_CNTR++;
		
		/****************************************************************************
		|						Instruction Fetch (IF) Stage						|
		****************************************************************************/
		
		if(Stall_IF_Timer > 0 || Branch_Disable_IF == 1)
		{//Send a Stall down the pipe, -1 everything else in pr[0]
			if (Stall_IF_Timer > 0)
			{
				Stall_IF_Timer--;
			}
			Branch_Disable_IF = 0;
		}
		else{
			// Load the Above Into pr[0], -1 everything else in pr[0]
			pr[0].OpCode	= ((mem_space[PC])	 		& 255);	// Parse out First  byte: Operation Code
			pr[0].Dest		= ((mem_space[PC] >> 8)		& 255);	// Parse out Second byte: Operand 1
			pr[0].Source1	= ((mem_space[PC] >> 16)	& 255);	// Parse out Third  byte: Operand 2
			pr[0].Source2	= ((mem_space[PC] >> 24)	& 255);	// Parse out Fourth byte: Operand 3
			
			printf("\nIF\tPC:%d\tOP:%d\tDest:%d\tS1:%d\tS2%d\t",PC,pr[0].OpCode, pr[0].Dest, pr[0].Source1, pr[0].Source2);
			
			INST_CNTR++; // Dynamic Instruction Counter
		
			PC++; // Increment Program Counter
		}
		
		CYCLE_CNTR++;
		
		//move data between stages
		for(i = 4; i >= 1; i --)
		{
			pr[i].ALU_Output = pr[i-1].ALU_Output;
			pr[i].Stall = pr[i-1].Stall;
			pr[i].OpCode = pr[i-1].OpCode;
			pr[i].Dest = pr[i-1].Dest;
			pr[i].Source1 = pr[i-1].Source1;
			pr[i].Source2 = pr[i-1].Source2;
			pr[i].LHS = pr[i-1].LHS;
			pr[i].RHS = pr[i-1].RHS;
			pr[i].Mem_Acc = pr[i-1].Mem_Acc;
			pr[i].Execution_Lat = pr[i-1].Execution_Lat;
		}
		
		// if(CYCLE_CNTR > 80)
		// {
			// printf("\n\tEXITED!\n");
			// exit(1);
		// }
	}
	return 0;
}
