open Expr
open Stmt
open Token

type t =
  { ts : Token.t list
  ; global : bool
  }

let expect e p =
  match p.ts with
  | [] -> raise Err.Out_Of_Tokens
  | hd :: tl when hd.k = e -> hd, {p with ts = tl}
  | hd :: tl ->
     raise (Err.Expect (Token.kind_to_string e, Token.kind_to_string hd.k))

let expect' e p =
  match p.ts with
  | [] -> raise Err.Out_Of_Tokens
  | hd :: tl when hd.k = e -> {p with ts = tl}
  | hd :: tl ->
     raise (Err.Expect (Token.kind_to_string e, Token.kind_to_string hd.k))

let binary lhs op rhs =
  Expr.Binary
    { node = {loc = Expr.get_location lhs; ty = Type.Undefined}
    ; lhs
    ; op
    ; rhs
    }

let rec parse_primary_expr p =
  match p.ts with
  | ({k = Integer_Literal; _} as i) :: ts ->
     Integer {node = {loc = i.l; ty = Type.Undefined}; i}, {p with ts}
  | ({k = String_Literal; _} as s) :: ts ->
     String {node = {loc = s.l; ty = Type.Undefined}; s}, {p with ts}
  | ({k = Identifier; _} as id) :: ts ->
     Identifier {node = {loc = id.l; ty = Type.Undefined}; id}, {p with ts}
  | hd :: _ ->
     raise (Err.Expect ("primary expression", Token.kind_to_string hd.k))
  | [] -> raise Err.Out_Of_Tokens

and parse_unary_expr p =
  match p.ts with
  | ({k = (Bang | Minus); _} as op) :: ts ->
     let rhs, p = parse_unary_expr {p with ts} in
     Expr.Unary {node = {loc = op.l; ty = Type.Undefined}; op; rhs}, p
  | _ -> parse_primary_expr p

and parse_multiplicative_expr p =
  let lhs, p = parse_unary_expr p in
  let rec aux lhs p =
    match p.ts with
    | {k = (Asterisk | Forward_Slash | Percent); _} as op :: ts ->
       let rhs, p = parse_unary_expr {p with ts} in
       aux (binary lhs op rhs) p
    | _ -> lhs, p
  in
  aux lhs p

and parse_additive_expr p =
  let lhs, p = parse_multiplicative_expr p in
  let rec aux lhs p =
    match p.ts with
    | ({k = (Plus | Minus); _} as op) :: ts ->
       let rhs, p = parse_multiplicative_expr {p with ts} in
       aux (binary lhs op rhs) p
    | _ -> lhs, p
  in
  aux lhs p

and parse_equalitive_expr p =
  let lhs, p = parse_additive_expr p in
  let rec aux lhs p =
    match p.ts with
    | ({k = (Double_Equals | Bang_Equals | Greaterthan | Greaterthan_Equals |
             Lessthan | Lessthan_Equals); _} as op) :: ts ->
       let rhs, p = parse_additive_expr {p with ts} in
       aux (binary lhs op rhs) p
    | _ -> lhs, p
  in
  aux lhs p

and parse_logical_expr p =
  let lhs, p = parse_equalitive_expr p in
  let rec aux lhs p =
    match p.ts with
    | ({k = (Pipe | Double_Pipe | Ampersand | Double_Ampersand); _} as op) :: ts ->
       let rhs, p = parse_equalitive_expr {p with ts} in
       aux (binary lhs op rhs) p
    | _ -> lhs, p
  in
  aux lhs p

and parse_assignment_expr p =
  let lhs, p = parse_logical_expr p in
  match p.ts with
  | ({k = (Equals | Plus_Equals | Minus_Equals | Asterisk_Equals |
           Forward_Slash_Equals | Pipe_Equals | Ampersand_Equals |
           Uptick_Equals | Percent_Equals); _} as op) :: ts ->
     let rhs, p = parse_assignment_expr {p with ts} in
     binary lhs op rhs, p
  | _ -> lhs, p

and parse_expr p = parse_assignment_expr p

and parse_type p =
  let ty, p = expect Token.Type p in
  let ty = match ty.lx with
    | "void" -> Type.Void
    | "i32" -> Type.I32
    | "u32" -> Type.U32
    | "u8" -> Type.U8
    | _ -> raise (Err.Expect ("primitive type", ty.lx))
  in
  let rec pointers ty p =
    match p.ts with
    | {k = Asterisk; _} :: ts -> pointers (Type.Ptr ty) {p with ts}
    | _ -> ty, p
  in
  pointers ty p

let rec parse_stmt p =
  match p.ts with
  | [] -> raise Err.Out_Of_Tokens
  | ({k = Keyword Export; _} :: {k = Keyword Proc; _} :: _) |
      {k = Keyword Proc; _} :: _ -> parse_stmt_proc p
  | {k = Keyword Let; _} :: _ -> parse_stmt_let p
  | {k = Keyword If; _} :: _ -> parse_stmt_if p
  | {k = L_Curly; _} :: _ -> parse_stmt_blk p
  | _ -> parse_stmt_expr p

and parse_stmt_if p =
  let loc, p =
    let hd, p = expect (Keyword If) p in
    hd.l, p in
  let cond, p = parse_expr p in
  let then_, p = parse_stmt p in
  let else_, p = match p.ts with
    | {k = Keyword Else; _} :: tl ->
       let s, p = parse_stmt {p with ts = tl} in
       Some s, p
    | _ -> None, p
  in
  Stmt.If {node = {loc}; cond; then_; else_}, p

and parse_stmt_let p =
  let st, p = expect (Keyword Let) p in
  let loc = st.l in
  let id, p = expect Token.Identifier p in
  let p = expect' Token.Colon p in
  let ty, p = parse_type p in
  let p = expect' Token.Equals p in
  let e, p = parse_expr p in
  let p = expect' Token.Semicolon p in
  Stmt.Let {node = {loc}; id; ty; e}, p

and parse_stmt_expr p =
  let e, p = parse_expr p in
  let _, p = expect Semicolon p in
  Expr {node = {loc = Expr.get_location e}; e}, p

and parse_stmt_blk p =
  let rec aux acc p =
    match p.ts with
    | {k = R_Curly; _} :: tl -> List.rev acc, {p with ts = tl}
    | _ ->
       let s, p = parse_stmt p in
       aux (s :: acc) p
  in
  let loc, p =
    let hd, p = expect L_Curly p in
    hd.l, p in
  let stmts, p = aux [] p in
  Block {node = {loc}; stmts}, p

and parse_stmt_proc p =
  let rec aux acc p =
    match p.ts with
    | {k = R_Paren; _} :: tl -> List.rev acc, {p with ts = tl}
    | _ ->
       let id, p = expect Identifier p in
       let p = expect' Colon p in
       let ty, p = parse_type p in
       let p = match p.ts with
         | {k = Comma; _} :: tl -> {p with ts = tl}
         | _ -> let paren, p = expect R_Paren p in
                {p with ts = paren :: p.ts}
       in
       aux ((id, ty) :: acc) p
  in
  let export, p = match p.ts with
    | {k = Keyword Export; _} :: tl -> true, {p with ts = tl}
    | _ -> false, p in
  let loc, p =
    let hd, p = expect (Keyword Proc) p in
    hd.l, p in
  let id, p = expect Identifier p in
  let params, p = aux [] (expect' L_Paren p) in
  let p = expect' Colon p in
  let rty, p = parse_type p in
  let blk, p = parse_stmt p in
  Proc {node = {loc}; export; id; params; rty; blk}, p

let parse ts =
  let rec aux acc p =
    match p.ts with
    | [] -> acc, p
    | _ ->
       let s, p = parse_stmt p in
       aux (s :: acc) p
  in
  try
    let stmts, p = aux [] {ts; global = true} in
    List.rev stmts
  with
  | Err.Expect (e, g) ->
     Printf.eprintf "expected `%s' but got `%s'\n" e g;
     failwith "Parsing Error"
  | Err.Out_Of_Tokens ->
     Printf.eprintf "ran out of tokens";
     failwith "Parsing Error"
