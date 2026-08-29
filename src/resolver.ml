open Vis
open Stmt
open Expr

type resolver_type = (Type.t Scope.t, Expr.t, Stmt.t) Vis.t

let resolve_expr_integer
      (v : resolver_type)
      (e : Expr.integer)
    : Expr.t * ('c, 'e, 's) Vis.t =
  Expr.Integer {
      e with
      node = {e.node with ty = Type.I32}
    }, v

let resolve_expr_string
      (v : resolver_type)
      (e : Expr.string_)
    : Expr.t * resolver_type =
  Expr.String {
      e with
      node = {e.node with ty = Type.Ptr Type.U8}
    }, v

let resolve_expr_identifier
      ({context = scope; _} as v : resolver_type)
      ({node; id = {lx = id}; _} as e : Expr.identifier)
    : Expr.t * resolver_type =
  match Scope.get id scope with
  | None -> raise @@ Err.Identifier_Not_Defined id
  | Some ty -> Expr.Identifier {
                   e with
                   node = {node with ty = ty}
                 }, v

let resolve_expr_binary v e = assert false
let resolve_expr_unary v e = assert false

let resolve_stmt_proc
      ({context = scope; _} as v : resolver_type)
      ({id = {lx = id}; rty; params; blk; _} as s : Stmt.proc)
    : Stmt.t * resolver_type =
  if Scope.contains id scope then
    raise @@ Err.Identifier_Already_Defined id
  else
    let scope =
      Scope.add
        id
        (Type.Proc {rty; params = List.map snd params})
        scope
      |> Scope.push
    in
    let scope, blk, v =
      let scope = let rec aux s = function
                    | [] -> s
                    | hd :: tl ->
                       aux (Scope.add ((fst hd).Token.lx) (snd hd) s) tl
                  in
                  aux scope params
      in
      let blk, v = accept_stmt {v with context = scope} blk in
      Scope.pop scope, blk, v
    in
    Stmt.Proc {s with blk}, {v with context = scope}

let resolve_stmt_let
      ({context = scope; _} as v : resolver_type)
      ({id = {lx = id; _}; _} as s : Stmt.let_)
    : Stmt.t * resolver_type =
  if Scope.contains id scope then
    raise @@ Err.Identifier_Already_Defined id
  else
    let e, v = accept_expr v s.e in
    if not (Type.check s.ty (Expr.get_type e)) then
      raise @@ Err.Incompatible_Types (Type.to_string s.ty,
                                       Type.to_string (Expr.get_type e))
    else
      Stmt.Let {s with e}, {v with context = Scope.add id s.ty scope}

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
