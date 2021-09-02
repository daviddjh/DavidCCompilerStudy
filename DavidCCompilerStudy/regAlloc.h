#pragma once
// #include "codeGen.h"

typedef enum dcg_Reg dcg_Reg;
typedef struct OpTreeNode OpTreeNode;

typedef struct RegAlloc {
  //OpTreeNode* regArray[dcg_COUNT];
  OpTreeNode* regArray[32];
} RegAlloc;

void initRegAlloc(RegAlloc* regAlloc);
dcg_Reg getReg(RegAlloc* regAlloc, dcg_Reg reg, OpTreeNode* node);
void freeAllRegs(RegAlloc* regAlloc);
void putbackReg(RegAlloc* regAlloc, dcg_Reg reg);
dcg_Reg putbackRegFromOpNode(RegAlloc* regAlloc, OpTreeNode* node);
dcg_Reg peakRegFromOpNode(RegAlloc* regAlloc, OpTreeNode* node);
dcg_Reg findOpNodesReg(RegAlloc* regAlloc, OpTreeNode* node);
