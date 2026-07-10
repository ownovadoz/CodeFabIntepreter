#include "ScopeResolver.h"

#include "../AssemblerUnit/Parser/Expression.h"
#include "../AssemblerUnit/Parser/Statement.h"
#include "../AssemblerUnit/Tokenizer/Token.h"

#include <gmock/gmock.h>

#include <memory>
#include <variant>
#include <vector>

using std::make_unique;
using std::monostate;
using std::move;
using std::unique_ptr;
using std::vector;

namespace {
    vector<unique_ptr<Statement>> single(unique_ptr<Statement> statement) {
        vector<unique_ptr<Statement>> statements;
        statements.push_back(move(statement));

        return statements;
    }
}

TEST(ScopeResolverTest, TopLevelVariableIsTreatedAsGlobalAndNotResolved)
{
    Token name_token(TokenType::IDENTIFIER, "a", monostate{}, 1);
    auto var_decl = make_unique<VarDeclareStmt>(name_token);
    var_decl->setExpression(make_unique<LiteralExpr>(Token(TokenType::NUMBER, "1", 1.0, 1)));

    auto var_expr = make_unique<VariableExpr>(name_token);
    const VariableExpr* var_expr_ptr = var_expr.get();
    auto usage = make_unique<ExpressionStmt>(move(var_expr));

    vector<unique_ptr<Statement>> statements;
    statements.push_back(move(var_decl));
    statements.push_back(move(usage));

    ScopeResolver resolver;
    resolver.resolve(statements);

    EXPECT_EQ(resolver.getLocals().count(var_expr_ptr), 0u);
}

TEST(ScopeResolverTest, VariableInBlockResolvesToDistanceZero)
{
    Token name_token(TokenType::IDENTIFIER, "a", monostate{}, 1);
    auto var_decl = make_unique<VarDeclareStmt>(name_token);
    var_decl->setExpression(make_unique<LiteralExpr>(Token(TokenType::NUMBER, "1", 1.0, 1)));

    auto var_expr = make_unique<VariableExpr>(name_token);
    const VariableExpr* var_expr_ptr = var_expr.get();
    auto usage = make_unique<ExpressionStmt>(move(var_expr));

    auto block = make_unique<BlockStmt>();
    block->addStatement(move(var_decl));
    block->addStatement(move(usage));

    ScopeResolver resolver;
    resolver.resolve(single(move(block)));

    ASSERT_EQ(resolver.getLocals().count(var_expr_ptr), 1u);
    EXPECT_EQ(resolver.getLocals().at(var_expr_ptr), 0);
}

TEST(ScopeResolverTest, VariableInNestedBlockResolvesToOuterDistance)
{
    Token name_token(TokenType::IDENTIFIER, "a", monostate{}, 1);
    auto var_decl = make_unique<VarDeclareStmt>(name_token);
    var_decl->setExpression(make_unique<LiteralExpr>(Token(TokenType::NUMBER, "1", 1.0, 1)));

    auto var_expr = make_unique<VariableExpr>(name_token);
    const VariableExpr* var_expr_ptr = var_expr.get();

    auto inner_block = make_unique<BlockStmt>();
    inner_block->addStatement(make_unique<ExpressionStmt>(move(var_expr)));

    auto outer_block = make_unique<BlockStmt>();
    outer_block->addStatement(move(var_decl));
    outer_block->addStatement(move(inner_block));

    ScopeResolver resolver;
    resolver.resolve(single(move(outer_block)));

    ASSERT_EQ(resolver.getLocals().count(var_expr_ptr), 1u);
    EXPECT_EQ(resolver.getLocals().at(var_expr_ptr), 1);
}

// Interpreter::executeIfStmt는 분기 자체를 위한 Environment를 만들지 않으므로,
// 중괄호 없는 then 분기에서 선언한 변수는 if문을 둘러싼 스코프에 그대로 남는다.
TEST(ScopeResolverTest, IfBranchWithoutBlockDoesNotOpenNewScope)
{
    Token name_token(TokenType::IDENTIFIER, "a", monostate{}, 1);
    auto then_branch = make_unique<VarDeclareStmt>(name_token);
    then_branch->setExpression(make_unique<LiteralExpr>(Token(TokenType::NUMBER, "1", 1.0, 1)));

    auto if_stmt = make_unique<IfStmt>(make_unique<LiteralExpr>(Token(TokenType::TRUE, "true", true, 1)), move(then_branch), nullptr);

    auto var_expr = make_unique<VariableExpr>(name_token);
    const VariableExpr* var_expr_ptr = var_expr.get();

    auto block = make_unique<BlockStmt>();
    block->addStatement(move(if_stmt));
    block->addStatement(make_unique<ExpressionStmt>(move(var_expr)));

    ScopeResolver resolver;
    resolver.resolve(single(move(block)));

    ASSERT_EQ(resolver.getLocals().count(var_expr_ptr), 1u);
    EXPECT_EQ(resolver.getLocals().at(var_expr_ptr), 0);
}

// executeForStmt는 for문 전체를 위한 Environment를 하나만 만들고 그 안에서 init을
// 정의하므로, 중괄호 없는 body는 그 스코프를 그대로 공유한다.
TEST(ScopeResolverTest, ForLoopBodyWithoutBlockSharesForScope)
{
    Token name_token(TokenType::IDENTIFIER, "i", monostate{}, 1);
    auto init = make_unique<VarDeclareStmt>(name_token);
    init->setExpression(make_unique<LiteralExpr>(Token(TokenType::NUMBER, "0", 0.0, 1)));

    auto var_expr = make_unique<VariableExpr>(name_token);
    const VariableExpr* var_expr_ptr = var_expr.get();
    auto body = make_unique<ExpressionStmt>(move(var_expr));

    auto for_stmt = make_unique<ForStmt>(move(init), nullptr, nullptr, move(body));

    ScopeResolver resolver;
    resolver.resolve(single(move(for_stmt)));

    ASSERT_EQ(resolver.getLocals().count(var_expr_ptr), 1u);
    EXPECT_EQ(resolver.getLocals().at(var_expr_ptr), 0);
}

// CodeFabFunction::call()은 파라미터와 몸통 문장들을 같은 Environment 하나에 담으므로
// 파라미터 참조는 별도의 블록 스코프를 거치지 않고 바로 거리 0에서 발견돼야 한다.
TEST(ScopeResolverTest, FunctionParamAndBodyShareSingleScope)
{
    Token param_token(TokenType::IDENTIFIER, "a", monostate{}, 1);
    auto var_expr = make_unique<VariableExpr>(param_token);
    const VariableExpr* var_expr_ptr = var_expr.get();

    auto body = make_unique<BlockStmt>();
    body->addStatement(make_unique<ExpressionStmt>(move(var_expr)));

    Token fn_name(TokenType::IDENTIFIER, "f", monostate{}, 1);
    auto fn_stmt = make_unique<FunctionStmt>(fn_name, vector<Token>{ param_token }, move(body));

    ScopeResolver resolver;
    resolver.resolve(single(move(fn_stmt)));

    ASSERT_EQ(resolver.getLocals().count(var_expr_ptr), 1u);
    EXPECT_EQ(resolver.getLocals().at(var_expr_ptr), 0);
}

// CodeFabFunction::bind()는 상속 여부와 무관하게 항상 "this" 하나짜리 Environment를
// 새로 만들어 call()이 만드는 파라미터/몸통 Environment를 감싸므로, 메서드 몸통에서
// this는 항상 거리 1에서 발견돼야 한다.
TEST(ScopeResolverTest, ThisInMethodBodyResolvesOneLevelAboveCallScope)
{
    Token this_keyword(TokenType::THIS, "this", monostate{}, 1);
    auto this_expr = make_unique<ThisExpr>(this_keyword);
    const ThisExpr* this_expr_ptr = this_expr.get();

    auto body = make_unique<BlockStmt>();
    body->addStatement(make_unique<ExpressionStmt>(move(this_expr)));

    Token method_name(TokenType::IDENTIFIER, "method", monostate{}, 1);
    auto method = make_unique<FunctionStmt>(method_name, vector<Token>{}, move(body));

    vector<unique_ptr<FunctionStmt>> methods;
    methods.push_back(move(method));

    Token class_name(TokenType::IDENTIFIER, "C", monostate{}, 1);
    auto class_stmt = make_unique<ClassStmt>(class_name, nullptr, move(methods));

    ScopeResolver resolver;
    resolver.resolve(single(move(class_stmt)));

    ASSERT_EQ(resolver.getLocals().count(this_expr_ptr), 1u);
    EXPECT_EQ(resolver.getLocals().at(this_expr_ptr), 1);
}

// executeClassStmt는 상속이 있을 때만 "super"를 담는 스코프를 만들고, 그 안쪽에
// bind()가 항상 만드는 "this" 스코프가 온다. 따라서 Super는 this보다 정확히
// 한 단계 더 먼 거리에서 발견돼야 하며, Interpreter::evaluateSuperExpr은 이
// 관계를 이용해 this 위치를 별도 조회 없이 distance - 1로 계산한다.
TEST(ScopeResolverTest, SuperResolvesOneLevelAboveThis)
{
    Token this_keyword(TokenType::THIS, "this", monostate{}, 1);
    auto this_expr = make_unique<ThisExpr>(this_keyword);
    const ThisExpr* this_expr_ptr = this_expr.get();

    Token super_keyword(TokenType::SUPER, "Super", monostate{}, 1);
    Token method_ref_token(TokenType::IDENTIFIER, "greet", monostate{}, 1);
    auto super_expr = make_unique<SuperExpr>(super_keyword, method_ref_token);
    const SuperExpr* super_expr_ptr = super_expr.get();

    auto body = make_unique<BlockStmt>();
    body->addStatement(make_unique<ExpressionStmt>(move(this_expr)));
    body->addStatement(make_unique<ExpressionStmt>(move(super_expr)));

    Token method_name(TokenType::IDENTIFIER, "method", monostate{}, 1);
    auto method = make_unique<FunctionStmt>(method_name, vector<Token>{}, move(body));

    vector<unique_ptr<FunctionStmt>> methods;
    methods.push_back(move(method));

    Token superclass_name(TokenType::IDENTIFIER, "Base", monostate{}, 1);
    auto superclass_ref = make_unique<VariableExpr>(superclass_name);

    Token class_name(TokenType::IDENTIFIER, "C", monostate{}, 1);
    auto class_stmt = make_unique<ClassStmt>(class_name, move(superclass_ref), move(methods));

    ScopeResolver resolver;
    resolver.resolve(single(move(class_stmt)));

    ASSERT_EQ(resolver.getLocals().count(this_expr_ptr), 1u);
    ASSERT_EQ(resolver.getLocals().count(super_expr_ptr), 1u);
    EXPECT_EQ(resolver.getLocals().at(super_expr_ptr) - 1, resolver.getLocals().at(this_expr_ptr));
}
