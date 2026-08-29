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

let rec parse_primary_expr p = assert false

and parse_unary_expr p =
  let lhs, p = parse_primary_expr p in
  assert false

and parse_multiplicative_expr p =
  let lhs, p = parse_unary_expr p in
  assert false

and parse_additive_expr p =
  let lhs, p = parse_multiplicative_expr p in
  assert false

and parse_equalitive_expr p =
  let lhs, p = parse_additive_expr p in
  assert false

and parse_logical_expr p =
  let lhs, p = parse_equalitive_expr p in
  assert false

and parse_assignment_expr p =
  let lhs, p = parse_logical_expr p in
  assert false

and parse_expr p = parse_assignment_expr p

and parse_type p = assert false

let rec parse_stmt p =
  match p.ts with
  | [] -> raise Err.Out_Of_Tokens
  | ({k = Keyword Export; _} :: {k = Keyword Proc; _} :: _) |
      {k = Keyword Proc; _} :: _ -> parse_stmt_proc p
  | {k = Keyword Let; _} :: _ -> parse_stmt_let p
  | {k = L_Curly; _} :: _ -> parse_stmt_blk p
  | _ -> parse_stmt_expr p

and parse_stmt_let p =
  let st, p = expect (Keyword Let) p in
  let loc = st.l in
  let id, p = expect Token.Identifier p in
  let _, p = expect Token.Colon p in
  let ty, p = parse_type p in
  let e, p = parse_expr p in
  let _, p = expect Token.Semicolon p in
  Stmt.Let {node = {loc}; id; ty; e}, p

and parse_stmt_expr p =
  let (e : Expr.t), p = parse_expr p in
  let _, p = expect Token.Semicolon p in
  Stmt.Expr {node = {loc = Expr.get_location e}; e}, p

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
