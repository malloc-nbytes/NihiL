open Vis
open Stmt
open Expr

let resolve_expr_integer
      (v : (Type.t Scope.t, Expr.t, Stmt.t) Vis.t)
      (e : Expr.integer)
    : Expr.t * ('c, 'e, 's) Vis.t =
  Expr.Integer {
      e with
      node = {e.node with ty = Type.I32}
    }, v

let resolve_expr_string
      (v : (Type.t Scope.t, Expr.t, Stmt.t) Vis.t)
      (e : Expr.string_)
    : Expr.t * (Type.t Scope.t, Expr.t, Stmt.t) Vis.t =
  Expr.String {
      e with
      node = {e.node with ty = Type.Ptr Type.U8}
    }, v

let resolve_expr_identifier v e = assert false
let resolve_expr_binary v e = assert false
let resolve_expr_unary v e = assert false
let resolve_stmt_proc v e = assert false

let resolve_stmt_let
      (v : (Type.t Scope.t, Expr.t, Stmt.t) Vis.t)
      ({id = {lx = id; _}; _} as s : Stmt.let_)
    : Stmt.t * (Type.t Scope.t, Expr.t, Stmt.t) Vis.t =
  if Scope.contains id v.context then
    raise @@ Err.Identifier_Already_Defined id
  else
    let e, v = accept_expr v s.e in
    if not (Type.check s.ty (Expr.get_type e)) then
      raise @@ Err.Incompatible_Types (Type.to_string s.ty,
                                       Type.to_string (Expr.get_type e))
    else
      Stmt.Let {s with e}, {v with context = Scope.add id s.ty v.context}

let resolve_stmt_expr v s = assert false
let resolve_stmt_block v s = assert false

let analyze stmts =
  let v = (Vis.{
             context         = Scope.empty;
             expr_integer    = resolve_expr_integer;
             expr_string     = resolve_expr_string;
             expr_identifier = resolve_expr_identifier;
             expr_binary     = resolve_expr_binary;
             expr_unary      = resolve_expr_unary;
             stmt_proc       = resolve_stmt_proc;
             stmt_let        = resolve_stmt_let;
             stmt_expr       = resolve_stmt_expr;
             stmt_block      = resolve_stmt_block;
          }) in
  assert false
