#ifndef __EXPR_VISITOR_IMPL__
#define __EXPR_VISITOR_IMPL__

#include "libs/exprBaseVisitor.h"
#include "libs/exprParser.h"
#include "libs/exprVisitor.h"

#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"

#include <any>
#include <llvm/ADT/StringRef.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/Instructions.h>
#include <llvm/IR/Type.h>
#include <llvm/Support/raw_ostream.h>
#include <map>
#include <memory>
#include <string>
#include <system_error>


class exprVisitorImpl : public exprBaseVisitor{
public:
    exprVisitorImpl() : context(std::make_unique<llvm::LLVMContext>()),
                        module(std::make_unique<llvm::Module>("EB", *context)),
		                builder(std::make_unique<llvm::IRBuilder<>>(*context)) {}
    
    void IRFunctionSysDecl(const char *nameFn, std::vector<llvm::Type *> argTy, bool isVar);
    virtual std::any visitProg(exprParser::ProgContext *ctx) override;
    virtual std::any visitPrintExpr(exprParser::PrintExprContext *ctx) override;
    virtual std::any visitAssign(exprParser::AssignContext *ctx) override;
    virtual std::any visitBlank(exprParser::BlankContext *ctx) override;
    virtual std::any visitMulDiv(exprParser::MulDivContext *ctx) override;
    virtual std::any visitAddSub(exprParser::AddSubContext *ctx) override;
    virtual std::any visitCond(exprParser::CondContext *ctx) override;
    virtual std::any visitNumber(exprParser::NumberContext *ctx) override;
    virtual std::any visitId(exprParser::IdContext *ctx) override;
    virtual std::any visitIfElse(exprParser::IfElseContext *ctx) override;
    virtual std::any visitIf(exprParser::IfContext *ctx) override;
    void test()
	{
		std::error_code error;
		llvm::raw_fd_stream outLL("test.ll", error);
		module->print(outLL, nullptr);
	}
private:
    std::map<std::string, std::any> memory;
	std::unique_ptr<llvm::LLVMContext> context;
	std::unique_ptr<llvm::Module> module;
	std::unique_ptr<llvm::IRBuilder<>> builder;

    llvm::Function *F;

    llvm::AllocaInst *CreateEntryBlockAlloca(llvm::StringRef varName)
	{
		llvm::IRBuilder<> TmpB(&F->getEntryBlock(), F->getEntryBlock().begin());
		return TmpB.CreateAlloca(llvm::Type::getDoubleTy(*context), nullptr,
								 varName);
	}


    llvm::Type *int8Type;
	llvm::Type *int32Type;
	llvm::Type *charPtrType;
	llvm::Type *typeDouble;
};

#endif