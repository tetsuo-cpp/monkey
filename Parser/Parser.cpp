#include "Parser.h"

#include <Token/Token.h>

#include <sstream>

namespace monkey::parser {

namespace {

const std::vector<std::pair<TokenType, Precedence>> Precedences = {
    {TokenType::EQ, Precedence::EQUALS},
    {TokenType::NOT_EQ, Precedence::EQUALS},
    {TokenType::LT, Precedence::LESSGREATER},
    {TokenType::GT, Precedence::LESSGREATER},
    {TokenType::PLUS, Precedence::SUM},
    {TokenType::MINUS, Precedence::SUM},
    {TokenType::SLASH, Precedence::PRODUCT},
    {TokenType::ASTERISK, Precedence::PRODUCT},
    {TokenType::LPAREN, Precedence::CALL},
    {TokenType::LBRACKET, Precedence::INDEX}};

} // namespace

Parser::Parser(lexer::Lexer &L) : L(L) {
  nextToken();
  nextToken();

  registerPrefix(TokenType::IDENT, [this]() { return parseIdentifier(); });
  registerPrefix(TokenType::INT, [this]() { return parseIntegerLiteral(); });
  registerPrefix(TokenType::BANG, [this]() { return parsePrefixExpression(); });
  registerPrefix(TokenType::MINUS,
                 [this]() { return parsePrefixExpression(); });
  registerPrefix(TokenType::TRUE, [this]() { return parseBoolean(); });
  registerPrefix(TokenType::FALSE, [this]() { return parseBoolean(); });
  registerPrefix(TokenType::STRING, [this]() { return parseStringLiteral(); });
  registerPrefix(TokenType::LPAREN,
                 [this]() { return parseGroupedExpression(); });
  registerPrefix(TokenType::IF, [this]() { return parseIfExpression(); });
  registerPrefix(TokenType::FUNCTION,
                 [this]() { return parseFunctionLiteral(); });
  registerPrefix(TokenType::LBRACKET, [this]() { return parseArrayLiteral(); });
  registerPrefix(TokenType::LBRACE, [this]() { return parseHashLiteral(); });
  registerInfix(TokenType::PLUS, [this](std::unique_ptr<ast::Expression> Left) {
    return parseInfixExpression(std::move(Left));
  });
  registerInfix(TokenType::MINUS,
                [this](std::unique_ptr<ast::Expression> Left) {
                  return parseInfixExpression(std::move(Left));
                });
  registerInfix(TokenType::SLASH,
                [this](std::unique_ptr<ast::Expression> Left) {
                  return parseInfixExpression(std::move(Left));
                });
  registerInfix(TokenType::ASTERISK,
                [this](std::unique_ptr<ast::Expression> Left) {
                  return parseInfixExpression(std::move(Left));
                });
  registerInfix(TokenType::EQ, [this](std::unique_ptr<ast::Expression> Left) {
    return parseInfixExpression(std::move(Left));
  });
  registerInfix(TokenType::NOT_EQ,
                [this](std::unique_ptr<ast::Expression> Left) {
                  return parseInfixExpression(std::move(Left));
                });
  registerInfix(TokenType::LT, [this](std::unique_ptr<ast::Expression> Left) {
    return parseInfixExpression(std::move(Left));
  });
  registerInfix(TokenType::GT, [this](std::unique_ptr<ast::Expression> Left) {
    return parseInfixExpression(std::move(Left));
  });
  registerInfix(TokenType::LPAREN,
                [this](std::unique_ptr<ast::Expression> Left) {
                  return parseCallExpression(std::move(Left));
                });
  registerInfix(TokenType::LBRACKET,
                [this](std::unique_ptr<ast::Expression> Left) {
                  return parseIndexExpression(std::move(Left));
                });
}

std::unique_ptr<ast::Program> Parser::parseProgram() {
  auto P = std::make_unique<ast::Program>();

  while (CurToken.Type != TokenType::END_OF_FILE) {
    auto S = parseStatement();
    if (S)
      P->Statements.push_back(std::move(S));

    nextToken();
  }

  return P;
}

std::unique_ptr<ast::Statement> Parser::parseStatement() {
  switch (CurToken.Type) {
  case TokenType::LET:
    return parseLetStatement();
  case TokenType::RETURN:
    return parseReturnStatement();
  default:
    return parseExpressionStatement();
  }
}

std::unique_ptr<ast::LetStatement> Parser::parseLetStatement() {
  auto LS = std::make_unique<ast::LetStatement>();
  LS->Tok = CurToken;

  if (!expectPeek(TokenType::IDENT))
    return nullptr;

  auto Name = std::make_unique<ast::Identifier>();
  Name->Tok = CurToken;
  Name->Value = CurToken.Literal;

  LS->Name = std::move(Name);

  if (!expectPeek(TokenType::ASSIGN))
    return nullptr;

  nextToken();
  LS->Value = parseExpression(Precedence::LOWEST);

  if (peekTokenIs(TokenType::SEMICOLON))
    nextToken();

  return LS;
}

std::unique_ptr<ast::ReturnStatement> Parser::parseReturnStatement() {
  auto RS = std::make_unique<ast::ReturnStatement>(CurToken, nullptr);

  nextToken();

  RS->ReturnValue = parseExpression(Precedence::LOWEST);

  if (peekTokenIs(TokenType::SEMICOLON))
    nextToken();

  return RS;
}

std::unique_ptr<ast::ExpressionStatement> Parser::parseExpressionStatement() {
  auto ES = std::make_unique<ast::ExpressionStatement>();
  ES->Tok = CurToken;
  ES->Expr = parseExpression(Precedence::LOWEST);

  if (peekTokenIs(TokenType::SEMICOLON))
    nextToken();

  return ES;
}

std::unique_ptr<ast::Expression> Parser::parseExpression(Precedence Prec) {
  auto FnIter = PrefixParseFns.find(CurToken.Type);
  if (FnIter == PrefixParseFns.end()) {
    noPrefixParseFnError(CurToken.Type);
    return nullptr;
  }

  auto LeftExp = FnIter->second();
  while (!peekTokenIs(TokenType::SEMICOLON) && Prec < peekPrecedence()) {
    auto InfixIter = InfixParseFns.find(PeekToken.Type);
    if (InfixIter == InfixParseFns.end())
      return LeftExp;

    nextToken();
    LeftExp = InfixIter->second(std::move(LeftExp));
  }

  return LeftExp;
}

std::unique_ptr<ast::Expression> Parser::parseIdentifier() {
  return std::make_unique<ast::Identifier>(CurToken, CurToken.Literal);
}

std::unique_ptr<ast::IntegerLiteral> Parser::parseIntegerLiteral() {
  try {
    int64_t Value = std::stoll(CurToken.Literal);
    auto IntLiteral = std::make_unique<ast::IntegerLiteral>(CurToken, Value);
    return IntLiteral;
  } catch (const std::invalid_argument &) {
    std::string Error("Could not parse " + CurToken.Literal + " as integer.");
    Errors.push_back(std::move(Error));
    return nullptr;
  }
}

std::unique_ptr<ast::Expression> Parser::parsePrefixExpression() {
  auto Prefix = std::make_unique<ast::PrefixExpression>();
  Prefix->Tok = CurToken;
  Prefix->Operator = CurToken.Literal;

  nextToken();

  Prefix->Right = parseExpression(Precedence::PREFIX);

  return Prefix;
}

std::unique_ptr<ast::Expression>
Parser::parseInfixExpression(std::unique_ptr<ast::Expression> Left) {
  auto Infix = std::make_unique<ast::InfixExpression>(
      CurToken, CurToken.Literal, std::move(Left), nullptr);

  auto Precedence = curPrecedence();
  nextToken();
  Infix->Right = parseExpression(Precedence);

  return Infix;
}

std::unique_ptr<ast::Expression> Parser::parseBoolean() {
  return std::make_unique<ast::Boolean>(CurToken, curTokenIs(TokenType::TRUE));
}

std::unique_ptr<ast::Expression> Parser::parseStringLiteral() {
  return std::make_unique<ast::String>(CurToken, CurToken.Literal);
}

std::unique_ptr<ast::Expression> Parser::parseGroupedExpression() {
  nextToken();

  auto Exp = parseExpression(Precedence::LOWEST);
  if (!expectPeek(TokenType::RPAREN))
    return nullptr;

  return Exp;
}

std::unique_ptr<ast::Expression> Parser::parseIfExpression() {
  auto IfE = std::make_unique<ast::IfExpression>();
  IfE->Tok = CurToken;

  if (!expectPeek(TokenType::LPAREN))
    return nullptr;

  nextToken();
  IfE->Condition = parseExpression(Precedence::LOWEST);

  if (!expectPeek(TokenType::RPAREN))
    return nullptr;

  if (!expectPeek(TokenType::LBRACE))
    return nullptr;

  IfE->Consequence = parseBlockStatement();

  if (peekTokenIs(TokenType::ELSE)) {
    nextToken();

    if (!expectPeek(TokenType::LBRACE))
      return nullptr;

    IfE->Alternative = parseBlockStatement();
  }

  return IfE;
}

std::unique_ptr<ast::BlockStatement> Parser::parseBlockStatement() {
  auto BlockTok = CurToken;
  std::vector<std::unique_ptr<ast::Statement>> Statements;

  nextToken();

  while (!curTokenIs(TokenType::RBRACE) &&
         !curTokenIs(TokenType::END_OF_FILE)) {
    auto Statement = parseStatement();
    if (Statement)
      Statements.push_back(std::move(Statement));

    nextToken();
  }

  return std::make_unique<ast::BlockStatement>(BlockTok, std::move(Statements));
}

std::unique_ptr<ast::Expression> Parser::parseFunctionLiteral() {
  auto FunctionTok = CurToken;

  if (!expectPeek(TokenType::LPAREN))
    return nullptr;

  auto Parameters = parseFunctionParameters();

  if (!expectPeek(TokenType::LBRACE))
    return nullptr;

  auto Body = parseBlockStatement();

  return std::make_unique<ast::FunctionLiteral>(
      FunctionTok, std::move(Parameters), std::move(Body));
}

std::vector<std::unique_ptr<ast::Identifier>>
Parser::parseFunctionParameters() {
  std::vector<std::unique_ptr<ast::Identifier>> Identifiers;

  if (peekTokenIs(TokenType::RPAREN)) {
    nextToken();
    return Identifiers;
  }

  nextToken();

  auto I = std::make_unique<ast::Identifier>(CurToken, CurToken.Literal);
  Identifiers.push_back(std::move(I));

  while (peekTokenIs(TokenType::COMMA)) {
    nextToken();
    nextToken();
    auto I = std::make_unique<ast::Identifier>(CurToken, CurToken.Literal);
    Identifiers.push_back(std::move(I));
  }

  if (!expectPeek(TokenType::RPAREN))
    return {};

  return Identifiers;
}

std::unique_ptr<ast::Expression>
Parser::parseCallExpression(std::unique_ptr<ast::Expression> Function) {
  auto FunctionTok = CurToken;
  return std::make_unique<ast::CallExpression>(
      FunctionTok, std::move(Function), parseExpressionList(TokenType::RPAREN));
}

std::unique_ptr<ast::Expression> Parser::parseArrayLiteral() {
  return std::make_unique<ast::ArrayLiteral>(
      CurToken, parseExpressionList(TokenType::RBRACKET));
}

std::vector<std::unique_ptr<ast::Expression>>
Parser::parseExpressionList(TokenType End) {
  std::vector<std::unique_ptr<ast::Expression>> List;

  if (peekTokenIs(End)) {
    nextToken();
    return List;
  }

  nextToken();
  List.push_back(parseExpression(Precedence::LOWEST));

  while (peekTokenIs(TokenType::COMMA)) {
    nextToken();
    nextToken();
    List.push_back(parseExpression(Precedence::LOWEST));
  }

  if (!expectPeek(End))
    return {};

  return List;
}

std::unique_ptr<ast::Expression>
Parser::parseIndexExpression(std::unique_ptr<ast::Expression> Left) {
  auto Exp = std::make_unique<ast::IndexExpression>(CurToken, std::move(Left),
                                                    nullptr);

  nextToken();
  Exp->Index = parseExpression(Precedence::LOWEST);
  if (!expectPeek(TokenType::RBRACKET))
    return nullptr;

  return Exp;
}

std::unique_ptr<ast::Expression> Parser::parseHashLiteral() {
  auto Hash = std::make_unique<ast::HashLiteral>(CurToken);

  while (!peekTokenIs(TokenType::RBRACE)) {
    nextToken();
    auto Key = parseExpression(Precedence::LOWEST);

    if (!expectPeek(TokenType::COLON))
      return nullptr;

    nextToken();
    auto Value = parseExpression(Precedence::LOWEST);

    Hash->Pairs[std::move(Key)] = std::move(Value);

    if (!peekTokenIs(TokenType::RBRACE) && !expectPeek(TokenType::COMMA))
      return nullptr;
  }

  if (!expectPeek(TokenType::RBRACE))
    return nullptr;

  return Hash;
}

void Parser::nextToken() {
  CurToken = PeekToken;
  PeekToken = L.nextToken();
}

bool Parser::curTokenIs(TokenType Type) const { return CurToken.Type == Type; }

bool Parser::peekTokenIs(TokenType Type) const {
  return PeekToken.Type == Type;
}

bool Parser::expectPeek(TokenType Type) {
  if (peekTokenIs(Type)) {
    nextToken();
    return true;
  }

  peekError(Type);
  return false;
}

void Parser::peekError(TokenType Type) {
  std::stringstream SS;
  SS << "expected next token to be " << tokenTypeToString(Type) << ", got "
     << tokenTypeToString(PeekToken.Type) << " instead";

  Errors.push_back(SS.str());
}

const std::vector<std::string> &Parser::errors() const { return Errors; }

void Parser::registerPrefix(TokenType Type, PrefixParseFn Prefix) {
  PrefixParseFns.emplace(Type, Prefix);
}

void Parser::registerInfix(TokenType Type, InfixParseFn Infix) {
  InfixParseFns.emplace(Type, Infix);
}

void Parser::noPrefixParseFnError(TokenType Type) {
  std::stringstream SS;
  SS << "no prefix parse function found for " << tokenTypeToString(Type)
     << " found";

  Errors.push_back(SS.str());
}

Precedence Parser::peekPrecedence() const {
  auto Iter = std::find_if(Precedences.begin(), Precedences.end(),
                           [this](const std::pair<TokenType, Precedence> &P) {
                             return P.first == PeekToken.Type;
                           });

  if (Iter != Precedences.end())
    return Iter->second;

  return Precedence::LOWEST;
}

Precedence Parser::curPrecedence() const {
  auto Iter = std::find_if(Precedences.begin(), Precedences.end(),
                           [this](const std::pair<TokenType, Precedence> &P) {
                             return P.first == CurToken.Type;
                           });

  if (Iter != Precedences.end())
    return Iter->second;

  return Precedence::LOWEST;
}

} // namespace monkey::parser
