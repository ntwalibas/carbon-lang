// Part of the Carbon Language project, under the Apache License v2.0 with LLVM
// Exceptions. See /LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#include "toolchain/semantics/semantics_context.h"
#include "toolchain/semantics/semantics_node.h"

namespace Carbon {

auto SemanticsHandleAddress(SemanticsContext& context,
                            ParseTree::Node parse_node) -> bool {
  return context.TODO(parse_node, "HandleAddress");
}

auto SemanticsHandleBreakStatement(SemanticsContext& context,
                                   ParseTree::Node parse_node) -> bool {
  return context.TODO(parse_node, "HandleBreakStatement");
}

auto SemanticsHandleBreakStatementStart(SemanticsContext& context,
                                        ParseTree::Node parse_node) -> bool {
  return context.TODO(parse_node, "HandleBreakStatementStart");
}

auto SemanticsHandleContinueStatement(SemanticsContext& context,
                                      ParseTree::Node parse_node) -> bool {
  return context.TODO(parse_node, "HandleContinueStatement");
}

auto SemanticsHandleContinueStatementStart(SemanticsContext& context,
                                           ParseTree::Node parse_node) -> bool {
  return context.TODO(parse_node, "HandleContinueStatementStart");
}

auto SemanticsHandleDeclaredName(SemanticsContext& context,
                                 ParseTree::Node parse_node) -> bool {
  // The parent is responsible for binding the name.
  context.node_stack().Push(parse_node);
  return true;
}

auto SemanticsHandleDeducedParameterList(SemanticsContext& context,
                                         ParseTree::Node parse_node) -> bool {
  return context.TODO(parse_node, "HandleDeducedParameterList");
}

auto SemanticsHandleDeducedParameterListStart(SemanticsContext& context,
                                              ParseTree::Node parse_node)
    -> bool {
  return context.TODO(parse_node, "HandleDeducedParameterListStart");
}

auto SemanticsHandleDesignatedName(SemanticsContext& context,
                                   ParseTree::Node parse_node) -> bool {
  auto name_str = context.parse_tree().GetNodeText(parse_node);
  auto name_id = context.semantics_ir().AddString(name_str);
  // The parent is responsible for binding the name.
  context.node_stack().Push(parse_node, name_id);
  return true;
}

auto SemanticsHandleDesignatorExpression(SemanticsContext& context,
                                         ParseTree::Node parse_node) -> bool {
  auto name_id = context.node_stack().Pop<SemanticsStringId>(
      ParseNodeKind::DesignatedName);

  auto base_id = context.node_stack().Pop<SemanticsNodeId>();
  auto base = context.semantics_ir().GetNode(base_id);
  auto base_type = context.semantics_ir().GetNode(
      context.semantics_ir().GetType(base.type_id()));

  switch (base_type.kind()) {
    case SemanticsNodeKind::StructType: {
      auto refs =
          context.semantics_ir().GetNodeBlock(base_type.GetAsStructType());
      // TODO: Do we need to optimize this with a lookup table for O(1)?
      for (int i = 0; i < static_cast<int>(refs.size()); ++i) {
        auto ref = context.semantics_ir().GetNode(refs[i]);
        if (name_id == ref.GetAsStructTypeField()) {
          context.AddNodeAndPush(
              parse_node,
              SemanticsNode::StructMemberAccess::Make(
                  parse_node, ref.type_id(), base_id, SemanticsMemberIndex(i)));
          return true;
        }
      }
      CARBON_DIAGNOSTIC(DesignatorExpressionNameNotFound, Error,
                        "Type `{0}` does not have a member `{1}`.", std::string,
                        llvm::StringRef);
      context.emitter().Emit(
          parse_node, DesignatorExpressionNameNotFound,
          context.semantics_ir().StringifyType(base.type_id()),
          context.semantics_ir().GetString(name_id));
      break;
    }
    default: {
      CARBON_DIAGNOSTIC(DesignatorExpressionUnsupported, Error,
                        "Type `{0}` does not support designator expressions.",
                        std::string);
      context.emitter().Emit(
          parse_node, DesignatorExpressionUnsupported,
          context.semantics_ir().StringifyType(base.type_id()));
      break;
    }
  }

  // Should only be reached on error.
  context.node_stack().Push(parse_node, SemanticsNodeId::BuiltinInvalidType);
  return true;
}

auto SemanticsHandleEmptyDeclaration(SemanticsContext& /*context*/,
                                     ParseTree::Node /*parse_node*/) -> bool {
  // Empty declarations have no actions associated.
  return true;
}

auto SemanticsHandleExpressionStatement(SemanticsContext& context,
                                        ParseTree::Node /*parse_node*/)
    -> bool {
  // Pop the expression without investigating its contents.
  // TODO: This will probably eventually need to do some "do not discard"
  // analysis.
  context.node_stack().PopAndDiscardId();
  return true;
}

auto SemanticsHandleFileEnd(SemanticsContext& /*context*/,
                            ParseTree::Node /*parse_node*/) -> bool {
  // Do nothing, no need to balance this node.
  return true;
}

auto SemanticsHandleForHeader(SemanticsContext& context,
                              ParseTree::Node parse_node) -> bool {
  return context.TODO(parse_node, "HandleForHeader");
}

auto SemanticsHandleForHeaderStart(SemanticsContext& context,
                                   ParseTree::Node parse_node) -> bool {
  return context.TODO(parse_node, "HandleForHeaderStart");
}

auto SemanticsHandleForIn(SemanticsContext& context, ParseTree::Node parse_node)
    -> bool {
  return context.TODO(parse_node, "HandleForIn");
}

auto SemanticsHandleForStatement(SemanticsContext& context,
                                 ParseTree::Node parse_node) -> bool {
  return context.TODO(parse_node, "HandleForStatement");
}

auto SemanticsHandleGenericPatternBinding(SemanticsContext& context,
                                          ParseTree::Node parse_node) -> bool {
  return context.TODO(parse_node, "GenericPatternBinding");
}

auto SemanticsHandleInfixOperator(SemanticsContext& context,
                                  ParseTree::Node parse_node) -> bool {
  auto rhs_id = context.node_stack().Pop<SemanticsNodeId>();
  auto lhs_id = context.node_stack().Pop<SemanticsNodeId>();

  // Figure out the operator for the token.
  auto token = context.parse_tree().node_token(parse_node);
  switch (auto token_kind = context.tokens().GetKind(token)) {
    case TokenKind::Plus:
      // TODO: This should search for a compatible interface. For now, it's a
      // very trivial check of validity on the operation.
      lhs_id = context.ImplicitAsRequired(
          parse_node, lhs_id, context.semantics_ir().GetNode(rhs_id).type_id());

      context.AddNodeAndPush(
          parse_node,
          SemanticsNode::BinaryOperatorAdd::Make(
              parse_node, context.semantics_ir().GetNode(lhs_id).type_id(),
              lhs_id, rhs_id));
      break;

    case TokenKind::And:
    case TokenKind::Or: {
      // The first operand is wrapped in a ShortCircuitOperand, which we
      // already handled by creating a RHS block and a resumption block, which
      // are the current block and its enclosing block.
      rhs_id = context.ImplicitAsBool(parse_node, rhs_id);

      // When the second operand is evaluated, the result of `and` and `or` is
      // its value.
      auto rhs_block_id = context.node_block_stack().PopForAdd();
      auto resume_block_id = context.node_block_stack().PeekForAdd();
      context.AddNodeToBlock(rhs_block_id,
                             SemanticsNode::BranchWithArg::Make(
                                 parse_node, resume_block_id, rhs_id));

      // Collect the result from either the first or second operand.
      context.AddNodeAndPush(
          parse_node,
          SemanticsNode::BlockArg::Make(
              parse_node, context.semantics_ir().GetNode(rhs_id).type_id(),
              resume_block_id));
      break;
    }

    default:
      return context.TODO(parse_node, llvm::formatv("Handle {0}", token_kind));
  }

  return true;
}

auto SemanticsHandleInvalidParse(SemanticsContext& context,
                                 ParseTree::Node parse_node) -> bool {
  return context.TODO(parse_node, "HandleInvalidParse");
}

auto SemanticsHandleLiteral(SemanticsContext& context,
                            ParseTree::Node parse_node) -> bool {
  auto token = context.parse_tree().node_token(parse_node);
  switch (auto token_kind = context.tokens().GetKind(token)) {
    case TokenKind::False:
    case TokenKind::True: {
      context.AddNodeAndPush(
          parse_node,
          SemanticsNode::BoolLiteral::Make(
              parse_node,
              context.CanonicalizeType(SemanticsNodeId::BuiltinBoolType),
              token_kind == TokenKind::True ? SemanticsBoolValue::True
                                            : SemanticsBoolValue::False));
      break;
    }
    case TokenKind::IntegerLiteral: {
      auto id = context.semantics_ir().AddIntegerLiteral(
          context.tokens().GetIntegerLiteral(token));
      context.AddNodeAndPush(
          parse_node,
          SemanticsNode::IntegerLiteral::Make(
              parse_node,
              context.CanonicalizeType(SemanticsNodeId::BuiltinIntegerType),
              id));
      break;
    }
    case TokenKind::RealLiteral: {
      auto token_value = context.tokens().GetRealLiteral(token);
      auto id = context.semantics_ir().AddRealLiteral(
          {.mantissa = token_value.Mantissa(),
           .exponent = token_value.Exponent(),
           .is_decimal = token_value.IsDecimal()});
      context.AddNodeAndPush(parse_node,
                             SemanticsNode::RealLiteral::Make(
                                 parse_node,
                                 context.CanonicalizeType(
                                     SemanticsNodeId::BuiltinFloatingPointType),
                                 id));
      break;
    }
    case TokenKind::StringLiteral: {
      auto id = context.semantics_ir().AddString(
          context.tokens().GetStringLiteral(token));
      context.AddNodeAndPush(
          parse_node,
          SemanticsNode::StringLiteral::Make(
              parse_node,
              context.CanonicalizeType(SemanticsNodeId::BuiltinStringType),
              id));
      break;
    }
    case TokenKind::Bool: {
      context.node_stack().Push(parse_node, SemanticsNodeId::BuiltinBoolType);
      break;
    }
    case TokenKind::IntegerTypeLiteral: {
      auto text = context.tokens().GetTokenText(token);
      if (text != "i32") {
        return context.TODO(parse_node, "Currently only i32 is allowed");
      }
      context.node_stack().Push(parse_node,
                                SemanticsNodeId::BuiltinIntegerType);
      break;
    }
    case TokenKind::FloatingPointTypeLiteral: {
      auto text = context.tokens().GetTokenText(token);
      if (text != "f64") {
        return context.TODO(parse_node, "Currently only f64 is allowed");
      }
      context.node_stack().Push(parse_node,
                                SemanticsNodeId::BuiltinFloatingPointType);
      break;
    }
    case TokenKind::StringTypeLiteral: {
      context.node_stack().Push(parse_node, SemanticsNodeId::BuiltinStringType);
      break;
    }
    default: {
      return context.TODO(parse_node, llvm::formatv("Handle {0}", token_kind));
    }
  }

  return true;
}

auto SemanticsHandleNameReference(SemanticsContext& context,
                                  ParseTree::Node parse_node) -> bool {
  auto name = context.parse_tree().GetNodeText(parse_node);
  context.node_stack().Push(parse_node, context.LookupName(parse_node, name));
  return true;
}

auto SemanticsHandleNamedConstraintDeclaration(SemanticsContext& context,
                                               ParseTree::Node parse_node)
    -> bool {
  return context.TODO(parse_node, "HandleNamedConstraintDeclaration");
}

auto SemanticsHandleNamedConstraintDefinition(SemanticsContext& context,
                                              ParseTree::Node parse_node)
    -> bool {
  return context.TODO(parse_node, "HandleNamedConstraintDefinition");
}

auto SemanticsHandleNamedConstraintDefinitionStart(SemanticsContext& context,
                                                   ParseTree::Node parse_node)
    -> bool {
  return context.TODO(parse_node, "HandleNamedConstraintDefinitionStart");
}

auto SemanticsHandleNamedConstraintIntroducer(SemanticsContext& context,
                                              ParseTree::Node parse_node)
    -> bool {
  return context.TODO(parse_node, "HandleNamedConstraintIntroducer");
}

auto SemanticsHandleParameterList(SemanticsContext& context,
                                  ParseTree::Node parse_node) -> bool {
  auto refs_id = context.ParamOrArgEnd(
      /*for_args=*/false, ParseNodeKind::ParameterListStart);
  // TODO: This contains the IR block for parameters. At present, it's just
  // loose, but it's not strictly required for parameter refs; we should either
  // stop constructing it completely or, if it turns out to be needed, store it.
  // Note, the underlying issue is that the LLVM IR has nowhere clear to emit,
  // so changing storage would require addressing that problem. For comparison
  // with function calls, the IR needs to be emitted prior to the call.
  context.node_block_stack().Pop();

  context.PopScope();
  context.node_stack().PopAndDiscardSoloParseNode(
      ParseNodeKind::ParameterListStart);
  context.node_stack().Push(parse_node, refs_id);
  return true;
}

auto SemanticsHandleParameterListComma(SemanticsContext& context,
                                       ParseTree::Node /*parse_node*/) -> bool {
  context.ParamOrArgComma(/*for_args=*/false);
  return true;
}

auto SemanticsHandleParameterListStart(SemanticsContext& context,
                                       ParseTree::Node parse_node) -> bool {
  context.PushScope();
  context.node_stack().Push(parse_node);
  context.node_block_stack().Push();
  context.ParamOrArgStart();
  return true;
}

auto SemanticsHandleParenExpression(SemanticsContext& context,
                                    ParseTree::Node parse_node) -> bool {
  auto value_id = context.node_stack().Pop<SemanticsNodeId>();
  context.node_stack().PopAndDiscardSoloParseNode(
      ParseNodeKind::ParenExpressionOrTupleLiteralStart);
  context.node_stack().Push(parse_node, value_id);
  return true;
}

auto SemanticsHandleParenExpressionOrTupleLiteralStart(
    SemanticsContext& context, ParseTree::Node parse_node) -> bool {
  context.node_stack().Push(parse_node);
  return true;
}

auto SemanticsHandlePatternBinding(SemanticsContext& context,
                                   ParseTree::Node parse_node) -> bool {
  auto [type_node, parsed_type_id] =
      context.node_stack().PopWithParseNode<SemanticsNodeId>();
  auto cast_type_id = context.ExpressionAsType(type_node, parsed_type_id);

  // Get the name.
  auto name_node =
      context.node_stack().PopForSoloParseNode(ParseNodeKind::DeclaredName);
  auto name_str = context.parse_tree().GetNodeText(name_node);
  auto name_id = context.semantics_ir().AddString(name_str);

  // Allocate storage, linked to the name for error locations.
  auto storage_id =
      context.AddNode(SemanticsNode::VarStorage::Make(name_node, cast_type_id));

  // Bind the name to storage.
  context.AddNodeAndPush(parse_node,
                         SemanticsNode::BindName::Make(name_node, cast_type_id,
                                                       name_id, storage_id));
  return true;
}

auto SemanticsHandlePostfixOperator(SemanticsContext& context,
                                    ParseTree::Node parse_node) -> bool {
  return context.TODO(parse_node, "HandlePostfixOperator");
}

auto SemanticsHandlePrefixOperator(SemanticsContext& context,
                                   ParseTree::Node parse_node) -> bool {
  auto value_id = context.node_stack().Pop<SemanticsNodeId>();

  // Figure out the operator for the token.
  auto token = context.parse_tree().node_token(parse_node);
  switch (auto token_kind = context.tokens().GetKind(token)) {
    case TokenKind::Not:
      value_id = context.ImplicitAsBool(parse_node, value_id);
      context.AddNodeAndPush(
          parse_node,
          SemanticsNode::UnaryOperatorNot::Make(
              parse_node, context.semantics_ir().GetNode(value_id).type_id(),
              value_id));
      break;

    default:
      return context.TODO(parse_node, llvm::formatv("Handle {0}", token_kind));
  }

  return true;
}

auto SemanticsHandleReturnStatement(SemanticsContext& context,
                                    ParseTree::Node parse_node) -> bool {
  CARBON_CHECK(!context.return_scope_stack().empty());
  const auto& fn_node =
      context.semantics_ir().GetNode(context.return_scope_stack().back());
  const auto callable =
      context.semantics_ir().GetFunction(fn_node.GetAsFunctionDeclaration());

  if (context.parse_tree().node_kind(context.node_stack().PeekParseNode()) ==
      ParseNodeKind::ReturnStatementStart) {
    context.node_stack().PopAndDiscardSoloParseNode(
        ParseNodeKind::ReturnStatementStart);

    if (callable.return_type_id.is_valid()) {
      // TODO: Add a note pointing at the return type's parse node.
      CARBON_DIAGNOSTIC(ReturnStatementMissingExpression, Error,
                        "Must return a {0}.", std::string);
      context.emitter()
          .Build(parse_node, ReturnStatementMissingExpression,
                 context.semantics_ir().StringifyType(callable.return_type_id))
          .Emit();
    }

    context.AddNode(SemanticsNode::Return::Make(parse_node));
  } else {
    auto arg = context.node_stack().Pop<SemanticsNodeId>();
    context.node_stack().PopAndDiscardSoloParseNode(
        ParseNodeKind::ReturnStatementStart);

    if (!callable.return_type_id.is_valid()) {
      CARBON_DIAGNOSTIC(
          ReturnStatementDisallowExpression, Error,
          "No return expression should be provided in this context.");
      CARBON_DIAGNOSTIC(ReturnStatementImplicitNote, Note,
                        "There was no return type provided.");
      context.emitter()
          .Build(parse_node, ReturnStatementDisallowExpression)
          .Note(fn_node.parse_node(), ReturnStatementImplicitNote)
          .Emit();
    } else {
      arg =
          context.ImplicitAsRequired(parse_node, arg, callable.return_type_id);
    }

    context.AddNode(SemanticsNode::ReturnExpression::Make(
        parse_node, context.semantics_ir().GetNode(arg).type_id(), arg));
  }
  return true;
}

auto SemanticsHandleReturnStatementStart(SemanticsContext& context,
                                         ParseTree::Node parse_node) -> bool {
  // No action, just a bracketing node.
  context.node_stack().Push(parse_node);
  return true;
}

auto SemanticsHandleReturnType(SemanticsContext& context,
                               ParseTree::Node parse_node) -> bool {
  // Propagate the type expression.
  auto [type_parse_node, type_node_id] =
      context.node_stack().PopWithParseNode<SemanticsNodeId>();
  auto cast_node_id = context.ExpressionAsType(type_parse_node, type_node_id);
  context.node_stack().Push(parse_node, cast_node_id);
  return true;
}

auto SemanticsHandleSelfTypeIdentifier(SemanticsContext& context,
                                       ParseTree::Node parse_node) -> bool {
  return context.TODO(parse_node, "HandleSelfTypeIdentifier");
}

auto SemanticsHandleSelfValueIdentifier(SemanticsContext& context,
                                        ParseTree::Node parse_node) -> bool {
  return context.TODO(parse_node, "HandleSelfValueIdentifier");
}

auto SemanticsHandleShortCircuitOperand(SemanticsContext& context,
                                        ParseTree::Node parse_node) -> bool {
  // Convert the condition to `bool`.
  auto cond_value_id = context.node_stack().Pop<SemanticsNodeId>();
  cond_value_id = context.ImplicitAsBool(parse_node, cond_value_id);
  auto bool_type_id = context.semantics_ir().GetNode(cond_value_id).type_id();

  // Compute the branch value: the condition for `and`, inverted for `or`.
  auto token = context.parse_tree().node_token(parse_node);
  SemanticsNodeId branch_value_id = SemanticsNodeId::Invalid;
  auto short_circuit_result_id = SemanticsNodeId::Invalid;
  switch (auto token_kind = context.tokens().GetKind(token)) {
    case TokenKind::And:
      branch_value_id = cond_value_id;
      short_circuit_result_id =
          context.AddNode(SemanticsNode::BoolLiteral::Make(
              parse_node, bool_type_id, SemanticsBoolValue::False));
      break;

    case TokenKind::Or:
      branch_value_id = context.AddNode(SemanticsNode::UnaryOperatorNot::Make(
          parse_node, bool_type_id, cond_value_id));
      short_circuit_result_id =
          context.AddNode(SemanticsNode::BoolLiteral::Make(
              parse_node, bool_type_id, SemanticsBoolValue::True));
      break;

    default:
      CARBON_FATAL() << "Unexpected short-circuiting operator " << parse_node;
  }

  // Create a block for the right-hand side and for the continuation.
  auto lhs_block_id = context.node_block_stack().PopForAdd();
  auto end_block_id = context.node_block_stack().PushForAdd();
  auto rhs_block_id = context.node_block_stack().PushForAdd();
  context.AddNodeToBlock(
      lhs_block_id,
      SemanticsNode::BranchIf::Make(parse_node, rhs_block_id, branch_value_id));
  context.AddNodeToBlock(
      lhs_block_id, SemanticsNode::BranchWithArg::Make(
                        parse_node, end_block_id, short_circuit_result_id));

  // Put the condition back on the stack for SemanticsHandleInfixOperator.
  context.node_stack().Push(parse_node, cond_value_id);
  return true;
}

auto SemanticsHandleTemplate(SemanticsContext& context,
                             ParseTree::Node parse_node) -> bool {
  return context.TODO(parse_node, "HandleTemplate");
}

auto SemanticsHandleTupleLiteral(SemanticsContext& context,
                                 ParseTree::Node parse_node) -> bool {
  return context.TODO(parse_node, "HandleTupleLiteral");
}

auto SemanticsHandleTupleLiteralComma(SemanticsContext& context,
                                      ParseTree::Node parse_node) -> bool {
  return context.TODO(parse_node, "HandleTupleLiteralComma");
}

auto SemanticsHandleVariableDeclaration(SemanticsContext& context,
                                        ParseTree::Node parse_node) -> bool {
  // Handle the optional initializer.
  auto expr_node_id = SemanticsNodeId::Invalid;
  bool has_init =
      context.parse_tree().node_kind(context.node_stack().PeekParseNode()) !=
      ParseNodeKind::PatternBinding;
  if (has_init) {
    expr_node_id = context.node_stack().Pop<SemanticsNodeId>();
    context.node_stack().PopAndDiscardSoloParseNode(
        ParseNodeKind::VariableInitializer);
  }

  // Get the storage and add it to name lookup.
  auto binding_id =
      context.node_stack().Pop<SemanticsNodeId>(ParseNodeKind::PatternBinding);
  auto binding = context.semantics_ir().GetNode(binding_id);
  auto [name_id, storage_id] = binding.GetAsBindName();
  context.AddNameToLookup(binding.parse_node(), name_id, storage_id);

  // If there was an initializer, assign it to storage.
  if (has_init) {
    auto cast_value_id = context.ImplicitAsRequired(
        parse_node, expr_node_id,
        context.semantics_ir().GetNode(storage_id).type_id());
    context.AddNode(SemanticsNode::Assign::Make(
        parse_node, context.semantics_ir().GetNode(cast_value_id).type_id(),
        storage_id, cast_value_id));
  }

  context.node_stack().PopAndDiscardSoloParseNode(
      ParseNodeKind::VariableIntroducer);

  return true;
}

auto SemanticsHandleVariableIntroducer(SemanticsContext& context,
                                       ParseTree::Node parse_node) -> bool {
  // No action, just a bracketing node.
  context.node_stack().Push(parse_node);
  return true;
}

auto SemanticsHandleVariableInitializer(SemanticsContext& context,
                                        ParseTree::Node parse_node) -> bool {
  // No action, just a bracketing node.
  context.node_stack().Push(parse_node);
  return true;
}

auto SemanticsHandleWhileCondition(SemanticsContext& context,
                                   ParseTree::Node parse_node) -> bool {
  return context.TODO(parse_node, "HandleWhileCondition");
}

auto SemanticsHandleWhileConditionStart(SemanticsContext& context,
                                        ParseTree::Node parse_node) -> bool {
  return context.TODO(parse_node, "HandleWhileConditionStart");
}

auto SemanticsHandleWhileStatement(SemanticsContext& context,
                                   ParseTree::Node parse_node) -> bool {
  return context.TODO(parse_node, "HandleWhileStatement");
}

}  // namespace Carbon
