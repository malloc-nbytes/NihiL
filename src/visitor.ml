type ('context, 'result) visitor =
  { context : 'context

  ; expr_integer    : ('context, 'result) visitor -> Expr.integer    -> 'result
  ; expr_string     : ('context, 'result) visitor -> Expr.string_    -> 'result
  ; expr_identifier : ('context, 'result) visitor -> Expr.identifier -> 'result
  ; expr_binary     : ('context, 'result) visitor -> Expr.binary     -> 'result

  ; stmt_expr    : ('context, 'result) visitor -> Stmt.expr    -> 'result
  ; stmt_vardecl : ('context, 'result) visitor -> Stmt.vardecl -> 'result
  ; stmt_proc    : ('context, 'result) visitor -> Stmt.proc    -> 'result
  ; stmt_blk     : ('context, 'result) visitor -> Stmt.blk     -> 'result
  }

let accept_expr (v : ('context, 'result) visitor) (expr : Expr.t) : 'result =
  match expr with
  | Expr.Integer    e -> v.expr_integer    v e
  | Expr.String     e -> v.expr_string     v e
  | Expr.Identifier e -> v.expr_identifier v e
  | Expr.Binary     e -> v.expr_binary     v e

let accept_stmt (v : ('context, 'result) visitor) (stmt : Stmt.t) : 'result =
  match stmt with
  | Stmt.Expr    s -> v.stmt_expr    v s
  | Stmt.Vardecl s -> v.stmt_vardecl v s
  | Stmt.Proc    s -> v.stmt_proc    v s
  | Stmt.Blk     s -> v.stmt_blk     v s
