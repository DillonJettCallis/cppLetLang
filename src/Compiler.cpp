//
// Created by Dillon on 2018-08-12.
//

#include "Compiler.h"
#include "Ast.h"
#include <memory>
#include <llvm/IR/Module.h>
#include <llvm/IR/LLVMContext.h>

class Compiler {

    llvm::LLVMContext con;
    llvm::Module mod;

public:
    Compiler() : mod(llvm::Module("main", con)) {

    }

    void compile(Expression *expression) {
        ExpressionKind kind = expression->kind;

        switch (kind) {
            case ExpressionKind::assignment: {
                auto ex = (Assignment *) expression;

                break;
            }
            case ExpressionKind::binaryOp: {
                auto ex = (BinaryOp *) expression;

                break;
            }
            case ExpressionKind::numberLiteral: {
                auto ex = (NumberLiteral *) expression;

                break;
            }
            default:
                throw std::runtime_error("Unknown Expression type");
        }
    }

};

