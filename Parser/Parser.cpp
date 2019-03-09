#include "Parser.h"

#include <Token/Token.h>

#include <sstream>

namespace monkey {

namespace {

const std::vector<std::pair<TokenType, Precedence>> Precedences = {
    {TokenType::EQ, Precedence::EQUALS},
    {TokenType::NOT_EQ, Precedence::EQUALS},
    {TokenType::LT, Precedence::LESSGREATER},
    {TokenType::GT, Precedence::LESSGREATER},
    {TokenType::PLUS, Precedence::SUM},
    {TokenType::MINUS, Precedence::SUM},
    {TokenType::SLASH, Precedence::PRODUCT},
    {TokenType::ASTERISK, Precedence::PRODUCT}};

} // namespace

Parser::Parser(Lexer &L) : L(L) {
  nextToken();
  nextToken();

  registerPrefix(TokenType::IDENT, [this]() { return parseIdentifier(); });
  registerPrefix(TokenType::INT, [this]() { return parseIntegerLiteral(); });
  registerPrefix(TokenType::BANG, [this]() { return parsePrefixExpression(); });
  registerPrefix(TokenType::MINUS,
                 [this]() { return parsePrefixExpression(); });
  registerInfix(TokenType::PLUS, [this](std::unique_ptr<Expression> Left) {
    return parseInfixExpression(std::move(Left));
  });
  registerInfix(TokenType::MINUS, [this](std::unique_ptr<Expression> Left) {
    return parseInfixExpression(std::move(Left));
  });
  registerInfix(TokenType::SLASH, [this](std::unique_ptr<Expression> Left) {
    return parseInfixExpression(std::move(Left));
  });
  registerInfix(TokenType::ASTERISK, [this](std::unique_ptr<Expression> Left) {
    return parseInfixExpression(std::move(Left));
  });
  registerInfix(TokenType::EQ, [this](std::unique_ptr<Expression> Left) {
    return parseInfixExpression(std::move(Left));
  });
  registerInfix(TokenType::NOT_EQ, [this](std::unique_ptr<Expression> Left) {
    return parseInfixExpression(std::move(Left));
  });
  registerInfix(TokenType::LT, [this](std::unique_ptr<Expression> Left) {
    return parseInfixExpression(std::move(Left));
  });
  registerInfix(TokenType::GT, [this](std::unique_ptr<Expression> Left) {
    return parseInfixExpression(std::move(Left));
  });
}

std::unique_ptr<Program> Parser::parseProgram() {
  auto P = std::make_unique<Program>();

  while (CurToken.Type != TokenType::END_OF_FILE) {
    auto S = parseStatement();
    if (S) {
      P->Statements.push_back(std::move(S));
    }

    nextToken();
  }

  return P;
}

std::unique_ptr<Statement> Parser::parseStatement() {
  switch (CurToken.Type) {
  case TokenType::LET:
    return parseLetStatement();
  case TokenType::RETURN:
    return parseReturnStatement();
  default:
    return parseExpressionStatement();
  }
}

std::unique_ptr<LetStatement> Parser::parseLetStatement() {
  auto LS = std::make_unique<LetStatement>();
  LS->Tok = CurToken;

  if (!expectPeek(TokenType::IDENT)) {
    return nullptr;
  }

  auto Name = std::make_unique<Identifier>();
  Name->Tok = CurToken;
  Name->Value = CurToken.Literal;

  LS->Name = std::move(Name);

  if (!expectPeek(TokenType::ASSIGN)) {
    return nullptr;
  }

  // TODO: We're skipping the expressions until we encounter a semicolon.
  while (!curTokenIs(TokenType::SEMICOLON)) {
    nextToken();
  }

  nextToken();

  return LS;
}

std::unique_ptr<ReturnStatement> Parser::parseReturnStatement() {
  auto RS = std::make_unique<ReturnStatement>();
  RS->Tok = CurToken;

  nextToken();

  // TODO: We're skipping the expressions until we encounter a semicolon.
  while (!curTokenIs(TokenType::SEMICOLON)) {
    nextToken();
  }

  nextToken();

  return RS;
}

std::unique_ptr<ExpressionStatement> Parser::parseExpressionStatement() {
  auto ES = std::make_unique<ExpressionStatement>();
  ES->Tok = CurToken;
  ES->Expr = parseExpression(Precedence::LOWEST);

  if (peekTokenIs(TokenType::SEMICOLON)) {
    nextToken();
  }

  return ES;
}

std::unique_ptr<Expression> Parser::parseExpression(Precedence Prec) {
  auto FnIter = PrefixParseFns.find(CurToken.Type);
  if (FnIter == PrefixParseFns.end()) {
    noPrefixParseFnError(CurToken.Type);
    return nullptr;
  }

  auto LeftExp = FnIter->second();
  while (!peekTokenIs(TokenType::SEMICOLON) && Prec < peekPrecedence()) {
    auto InfixIter = InfixParseFns.find(PeekToken.Type);
    if (InfixIter == InfixParseFns.end()) {
      return LeftExp;
    }

    nextToken();
    LeftExp = InfixIter->second(std::move(LeftExp));
  }

  return LeftExp;
}

std::unique_ptr<Expression> Parser::parseIdentifier() {
  return std::make_unique<Identifier>(CurToken, CurToken.Literal);
}

std::unique_ptr<IntegerLiteral> Parser::parseIntegerLiteral() {
  try {
    int64_t Value = std::stoll(CurToken.Literal);
    auto IntLiteral = std::make_unique<IntegerLiteral>(CurToken, Value);
    return IntLiteral;
  } catch (const std::invalid_argument &) {
    std::string Error("Could not parse " + CurToken.Literal + " as integer.");
    Errors.push_back(std::move(Error));
    return nullptr;
  }
}

std::unique_ptr<Expression> Parser::parsePrefixExpression() {
  auto Prefix = std::make_unique<PrefixExpression>();
  Prefix->Tok = CurToken;
  Prefix->Operator = CurToken.Literal;

  nextToken();

  Prefix->Right = parseExpression(Precedence::PREFIX);

  return Prefix;
}

std::unique_ptr<Expression>
Parser::parseInfixExpression(std::unique_ptr<Expression> Left) {
  auto Infix = std::make_unique<InfixExpression>(CurToken, CurToken.Literal,
                                                 std::move(Left), nullptr);

  auto Precedence = curPrecedence();
  nextToken();
  Infix->Right = parseExpression(Precedence);

  return Infix;
}

void Parser::nextToken() {
  CurToken = PeekToken;
  PeekToken = L.nextToken();
}

bool Parser::curTokenIs(TokenType Type) const { return PeekToken.Type == Type; }

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

  if (Iter != Precedences.end()) {
    return Iter->second;
  }

  return Precedence::LOWEST;
}

Precedence Parser::curPrecedence() const {
  auto Iter = std::find_if(Precedences.begin(), Precedences.end(),
                           [this](const std::pair<TokenType, Precedence> &P) {
                             return P.first == CurToken.Type;
                           });

  if (Iter != Precedences.end()) {
    return Iter->second;
  }

  return Precedence::LOWEST;
}

} // namespace monkey
