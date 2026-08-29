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

let resolve_expr_binary
      (v : resolver_type)
      ({node; lhs; op = {lx = op; k}; rhs; _} as e : Expr.binary)
    : Expr.t * resolver_type =
  let lhs, v = accept_expr v lhs in
  let rhs, v = accept_expr v rhs in
  let lty, rty = Expr.get_type lhs, Expr.get_type rhs in
  if not @@ Type.check lty rty then
    raise @@ Err.Incompatible_Types (Type.to_string lty, Type.to_string rty)
  else if not @@ Type.check_binop lty k rty then
    raise @@ Err.Incompatible_Binop_Types (Type.to_string lty, op, Type.to_string rty)
  else
    Expr.Binary {
        e with
        node = {node with ty = Expr.get_type lhs}
      }, v

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

let resolve_stmt_expr (v : resolver_type) ({e; _} as s : Stmt.expr)
    : Stmt.t * resolver_type =
  let e, v = accept_expr v e in
  Stmt.Expr {s with e}, v

let resolve_stmt_block
      ({context = scope; _} as v : resolver_type)
      (s : Stmt.block)
  : Stmt.t * resolver_type =
  let rec aux acc v = function
    | [] -> List.rev acc, v
    | hd :: tl ->
       let s, v = accept_stmt v hd in
       aux (s :: acc) v tl
  in
  let stmts, v = aux [] {v with context = Scope.push scope} s.stmts in
  Stmt.Block {s with stmts}, {v with context = Scope.pop v.context}

let analyze stmts =
  let rec aux v = function
    | [] -> []
    | hd :: tl ->
       let s, v = accept_stmt v hd in
       s :: aux v tl
  in
  try
    List.rev @@
      aux (Vis.{
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
        }) stmts
  with
  | Err.Incompatible_Types (t, t') ->
     let _ = Printf.eprintf "types `%s' and `%s' are not compatible\n" t t' in
     failwith "semantic error"
  | Err.Identifier_Already_Defined id ->
     let _ = Printf.eprintf "identifier `%s' is already defined\n" id in
     failwith "semantic error"
  | Err.Identifier_Not_Defined id ->
     let _ = Printf.eprintf "identifier `%s' is not defined\n" id in
     failwith "semantic error"
  | Err.Incompatible_Binop_Types (l, op, r) ->
     let _ = Printf.eprintf "Unsupported binary operator `%s' on types `%s' and `%s'\n" op l r in
     failwith "semantic error"
