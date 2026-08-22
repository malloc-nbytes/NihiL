type ('context, 'result) visitor =
  { context : 'context

  ; expr_integer    : ('context, 'result) visitor -> Expr.integer    -> 'result
  ; expr_string     : ('context, 'result) visitor -> Expr.string     -> 'result
  ; expr_identifier : ('context, 'result) visitor -> Expr.identifier -> 'result
  ; expr_bin        : ('context, 'result) visitor -> Expr.binary     -> 'result

  ; stmt_expr    : ('context, 'result) visitor -> Stmt.expr    -> 'result
  ; stmt_vardecl : ('context, 'result) visitor -> Stmt.vardecl -> 'result
  ; stmt_proc    : ('context, 'result) visitor -> Stmt.proc    -> 'result
  }

