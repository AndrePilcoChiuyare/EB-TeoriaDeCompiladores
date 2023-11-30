#include "exprVisitorImpl.h"
#include "libs/exprLexer.h"

#include <any>
#include <llvm-17/llvm/IR/Constant.h>
#include <llvm-17/llvm/Support/raw_ostream.h>
#include <llvm/ADT/APFloat.h>
#include <llvm/IR/BasicBlock.h>
#include <llvm/IR/Constants.h>
#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Instructions.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Type.h>
#include <llvm/IR/Value.h>
#include <string>
#include <typeinfo>

void exprVisitorImpl::IRFunctionSysDecl(const char *nameFn, std::vector<llvm::Type *> argTy, bool isVar){
    std::vector<llvm::Type *> argTypes = argTy;

	llvm::FunctionType *funcType = llvm::FunctionType::get(
		int32Type,
		argTypes,
		isVar
	);

	llvm::Function *func = llvm::Function::Create(
		funcType,
		llvm::Function::ExternalLinkage,
		nameFn,
		module.get()
	);
}

std::any exprVisitorImpl::visitProg(exprParser::ProgContext *ctx){
    int8Type = llvm::Type::getInt8Ty(*context);
	int32Type = llvm::Type::getInt32Ty(*context);
	charPtrType = llvm::PointerType::get(int8Type, 0);

	IRFunctionSysDecl("puts", {charPtrType}, false);

	std::vector<llvm::Type *> Doubles(0,
									  llvm::Type::getDoubleTy(*context));
	llvm::FunctionType *FT = llvm::FunctionType::get(
		llvm::Type::getVoidTy(*context), Doubles, false);

	F = llvm::Function::Create(
		FT, llvm::Function::ExternalLinkage, "_main_", module.get());

	llvm::BasicBlock *BB = llvm::BasicBlock::Create(*context, "entry", F);
	builder->SetInsertPoint(BB);

	visitChildren(ctx);

	builder->CreateRet(nullptr);

	return std::any(nullptr);
}
std::any exprVisitorImpl::visitPrintExpr(exprParser::PrintExprContext *ctx){
    llvm::Value *value = std::any_cast<llvm::Value *>(visit(ctx->expr()));

	llvm::FunctionCallee putsFunc = module->getOrInsertFunction("puts", llvm::Type::getInt32Ty(*context), llvm::Type::getInt8PtrTy(*context));
	llvm::CallInst *result = builder->CreateCall(putsFunc, {builder->CreateGlobalStringPtr(value->getName())}, "callSystem");

	return std::any();
}
std::any exprVisitorImpl::visitAssign(exprParser::AssignContext *ctx){
    std::string id = std::any_cast<std::string>(ctx->ID()->getText());

	auto value = visit(ctx->expr());

	if (value.type() == typeid(llvm::Value *)) {
		llvm::AllocaInst *allocaInst = CreateEntryBlockAlloca(id);
		llvm::Value *valueToStore = std::any_cast<llvm::Value *>(value);
		builder->CreateStore(valueToStore, allocaInst);
		memory[id] = valueToStore;
	}

	return std::any(nullptr);
}
std::any exprVisitorImpl::visitBlank(exprParser::BlankContext *ctx){
    return visitChildren(ctx);
}
std::any exprVisitorImpl::visitMulDiv(exprParser::MulDivContext *ctx){
    llvm::Value *L = std::any_cast<llvm::Value *>(visit(ctx->expr(0)));
	llvm::Value *R = std::any_cast<llvm::Value *>(visit(ctx->expr(1)));
	if (ctx->op->getType() == exprParser::MUL) {
		return std::any(builder->CreateFMul(L, R, "mulTemp"));
	}
	else
	{
		return std::any(builder->CreateFDiv(L, R, "divTemp"));
	}
}
std::any exprVisitorImpl::visitAddSub(exprParser::AddSubContext *ctx){
    llvm::Value *L = std::any_cast<llvm::Value *>(visit(ctx->expr(0)));
	llvm::Value *R = std::any_cast<llvm::Value *>(visit(ctx->expr(1)));
	if (ctx->op->getType() == exprParser::ADD){
		return std::any(builder->CreateFAdd(L, R, "addTemp"));
	}
	else
	{
		return std::any(builder->CreateFSub(L, R, "subTemp"));
	}
}
std::any exprVisitorImpl::visitCond(exprParser::CondContext *ctx){
    llvm::Value *L = std::any_cast<llvm::Value *>(visit(ctx->expr(0)));
	llvm::Value *R = std::any_cast<llvm::Value *>(visit(ctx->expr(1)));

	switch(ctx->op->getType()){
		case exprParser::EQ:
        	return std::any(builder->CreateFCmpUEQ(L, R, "eqTemp"));
		case exprParser::NEQ:
			return std::any(builder->CreateFCmpUNE(L, R, "neqTemp"));
		case exprParser::GR:
			return std::any(builder->CreateFCmpUGT(L, R, "grTemp"));
		case exprParser::GRE:
			return std::any(builder->CreateFCmpUGE(L, R, "greTemp"));
		case exprParser::LS:
			return std::any(builder->CreateFCmpULT(L, R, "lsTemp"));
		case exprParser::LSE:
			return std::any(builder->CreateFCmpULE(L, R, "lseTemp"));
		default:
        	return std::any(nullptr);
	}
}
std::any exprVisitorImpl::visitNumber(exprParser::NumberContext *ctx){
    auto numVal = std::stod(ctx->NUMBER()->getText());
	llvm::Value *val = llvm::ConstantFP::get(*context, llvm::APFloat(numVal));
	
    return std::any(val);
}
std::any exprVisitorImpl::visitId(exprParser::IdContext *ctx){
    std::string idName = ctx->ID()->getText();

	if (memory.find(idName) != memory.end())
		return std::any(memory[idName]);

	return std::any(idName);
}
std::any exprVisitorImpl::visitIfElse(exprParser::IfElseContext *ctx){
    llvm::Value *cond = std::any_cast<llvm::Value *>(visit(ctx->expr(0)));

	llvm::Function *TheFunction = builder->GetInsertBlock()->getParent();
	llvm::BasicBlock *doBlock = llvm::BasicBlock::Create(*context, "doBB", TheFunction);
	llvm::BasicBlock *elseBlock = llvm::BasicBlock::Create(*context, "elseBB");
	llvm::BasicBlock *endBlock = llvm::BasicBlock::Create(*context, "mergeBB");

	builder->CreateCondBr(cond, doBlock, elseBlock);
	builder->SetInsertPoint(doBlock);
	visit(ctx->expr(1));
	builder->CreateBr(endBlock);

	doBlock = builder->GetInsertBlock();
	TheFunction->insert(TheFunction->end(), elseBlock);
	builder->SetInsertPoint(elseBlock);
	visit(ctx->expr(2));
	builder->CreateBr(endBlock);

	elseBlock = builder->GetInsertBlock();
	TheFunction->insert(TheFunction->end(), endBlock);
	builder->SetInsertPoint(endBlock);

	return std::any(nullptr);
}
std::any exprVisitorImpl::visitIf(exprParser::IfContext *ctx){
    llvm::Value *cond = std::any_cast<llvm::Value *>(visit(ctx->expr(0)));

	llvm::Function *TheFunction = builder->GetInsertBlock()->getParent();
	llvm::BasicBlock *doBlock = llvm::BasicBlock::Create(*context, "doBB", TheFunction);
	llvm::BasicBlock *endBlock = llvm::BasicBlock::Create(*context, "endBB");

	builder->CreateCondBr(cond, doBlock, endBlock);
	builder->SetInsertPoint(doBlock);
	visit(ctx->expr(1));

	builder->CreateBr(endBlock);
	TheFunction->insert(TheFunction->end(), endBlock);
	builder->SetInsertPoint(endBlock);

	return std::any(nullptr);
}