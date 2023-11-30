#include "libs/exprLexer.h"
#include "libs/exprParser.h"
#include "exprVisitorImpl.h"

#include <fstream>

int main(int argc, char **argv)
{
	if (argc <= 1)
		return -1;
	std::ifstream is;
	is.open(argv[1]);
	antlr4::ANTLRInputStream input(is);
	exprLexer lexer(&input);
	antlr4::CommonTokenStream tokens(&lexer);
	exprParser parser(&tokens);
	auto tree = parser.prog();
	exprVisitorImpl eval;
	eval.visitProg(tree);
	eval.test();
	// std::cout << tree->toStringTree(true) << std::endl;
}