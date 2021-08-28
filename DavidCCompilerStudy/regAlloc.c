#include "regAlloc.h"
#include "codeGen.h"

void initRegAlloc(RegAlloc* regAlloc){

  for(int i = 0; i < dcg_COUNT; i++){
    regAlloc->regArray[i] = {0};
    //regAlloc->regArray[i] = NULL;
  }
}

dcg_Reg getReg(RegAlloc* regAlloc, dcg_Reg reg, OpTreeNode* node){

  if( reg != dcg_none){

    if(reg < dcg_COUNT && reg >= 0){

      if(regAlloc->regArray[reg] != NULL){

        // This register is already allocated, so it needs to be moved

        // Create move op to move register contents
        OpTreeNode* movNode = malloc(sizeof(OpTreeNode));
        movNode->left        = NULL;
        movNode->right       = NULL;
        movNode->dcg_OpCode  = dcg_MOV; 
        movNode->ast_node    = regAlloc->regArray[reg]->ast_node;     // IDK what this should be?
        movNode->arg_struct  = malloc(sizeof(dcg_ArgStruct));

        movNode->arg_struct->arg_types      = 0;
        movNode->arg_struct->arg_types     |= dcg_OpCodeArgType_REG;
        movNode->arg_struct->arg_types     |= (dcg_OpCodeArgType_REG << 4);
        movNode->arg_struct->OpArg1.arg_reg = getReg(regAlloc, dcg_none, regAlloc->regArray[reg]);
        movNode->arg_struct->OpArg2.arg_reg = reg;

        


        return reg;

      } else {

        // This register isn't allcated, so just set it and return it
        regAlloc->regArray[reg] = node;
        return reg;

      }
    } else {
    
      // The register was out of the range of registers available
      printf("RegAlloc: Error, Trying to allocate invalid register, exiting");
      exit(1);

    }
  }
  // Node doens't need to allocate a specific node, 
  
  // For now, just allocate General use 32bit registers
  /*
     Order for register allocation:
     eax
     ecx
     r10d
     r11d
     r12d
     r13d
     r14d
     r15d
   */

  // Try eax first
  if(regAlloc->regArray[dcg_eax] == NULL){
    regAlloc->regArray[dcg_eax] = node;
    return dcg_eax;
  }

  // Then ecx
  if(regAlloc->regArray[dcg_ecx] == NULL){
    regAlloc->regArray[dcg_ecx] = node;
    return dcg_ecx;
  }

  // Then the rest of the available regs
  for(int i = dcg_r10d; i <= dcg_r15d; i++){

    if(regAlloc->regArray[i] == NULL){
      regAlloc->regArray[i] = node;
      return i;
    }
  }
}

void putbackReg(RegAlloc* regAlloc, dcg_Reg reg){
  regAlloc->regArray[reg] = NULL;
}










