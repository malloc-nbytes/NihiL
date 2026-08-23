open Token

type t =
  { tokens : Token.t list
  ; stmts  : Stmt.t list
  }

let need k = function
  | {tokens = []; _} -> Err.fatal "need: hit EOF\n"
  | {tokens = hd :: tl; _} as p when hd.k = k -> hd, {p with tokens = tl}
  | {tokens = hd :: _} ->
     Err.fatal "%s need `%s' but got `%s'\n" (Location.fmt hd.loc) (Token.kind_to_string k) (Token.kind_to_string hd.k)

let need_loc k p = let t, p = need k p in t.loc, p

let expect k p = let _, p = need k p in p

let needkw s = function
  | {tokens = []; _} -> Err.fatal "needkw: hit EOF\n"
  | {tokens = hd :: tl; _} as p when hd.k = Keyword && hd.lx = s -> hd, {p with tokens = tl}
  | {tokens = hd :: _} ->
     Err.fatal "%s needed keyword `%s' but got `%s'\n" (Location.fmt hd.loc) s hd.lx

let expectkw s p = let _, p = needkw s p in p

let rec parse_primary_expr p =
  match p.tokens with
  | {k = Identifier; loc; _} as hd :: tl ->
     Expr.Identifier {node = {type_ = None; loc}; id = hd}, {p with tokens = tl}
  | {k = Integer_Literal; loc; _} as hd :: tl ->
     Expr.Integer {node = {type_ = None; loc}; i = hd}, {p with tokens = tl}
  | {k = String_Literal; loc; _} as hd :: tl ->
     Expr.String {node = {type_ = None; loc}; s = hd}, {p with tokens = tl}
  | hd :: _ ->
     Err.fatal "%s invalid expression near `%s'\n" (Location.fmt hd.loc) hd.lx
  | [] -> Err.fatal "invalid expression\n"

and parse_additive_expr p =
  let lhs, p = parse_primary_expr p in
  let rec aux lhs p =
    match p.tokens with
    | {k = (Plus|Minus); _} as hd :: tl ->
       let rhs, p = parse_primary_expr {p with tokens = tl} in
       let lhs = Expr.Binary {
                     node = {type_ = None; loc = Expr.get_loc lhs};
                     lhs; op = hd; rhs} in
       aux lhs p
    | _ -> lhs, p
  in aux lhs p

and parse_assignment_expr p =
  let lhs, p = parse_additive_expr p in
  match p.tokens with
  | {k = Equals; _} as hd :: tl ->
     let rhs, p = parse_assignment_expr {p with tokens = tl} in
     Expr.Binary {node = {type_ = None; loc = hd.loc}; lhs; op = hd; rhs}, p
  | _ -> lhs, p

and parse_expr p = parse_assignment_expr p

(* TODO: Add advanced types *)
let parse_type p =
  let base, p = need TypeKeyword p in
  Type.from_string base.lx, p

let rec parse_stmt_blk p =
  let loc, p = need_loc L_Curly p in
  let rec aux acc = function
    | {tokens = ({k = R_Curly; _}) :: tl; _} as p ->
       Stmt.Blk {node = {loc}; stmts = List.rev acc}, {p with tokens = tl}
    | _ -> let s, p = parse_stmt p in aux (s :: acc) p
  in aux [] p

and parse_stmt_proc p =
  let parse_proc_params p =
    let rec aux acc = function
      | {tokens = {k = R_Curly; _} :: tl} -> List.rev acc, p
      | _ as p ->
         let id, p = need Identifier p in
         let p = expect Colon p in
         let ty, p = parse_type p in
         match p.tokens with
         | {k = Comma; _} :: tl -> aux ((id, ty) :: acc) {p with tokens = tl}
         | _ -> let p = expect R_Curly p in List.rev acc, p
    in aux [] @@ expect L_Paren p
  in
  let export, loc1, p = match p.tokens with
    | {lx = "export"; loc; _} :: tl -> true, (Some loc), {p with tokens = tl}
    | _ -> false, None, p in
  let loc2, p   = needkw "proc" p in
  let loc       = match loc1 with | Some l -> l | _ -> loc2.loc in
  let id, p     = need Identifier p in
  let params, p = parse_proc_params p in
  let p         = expect Colon p in
  let rty, p    = parse_type p in
  let body, p   = parse_stmt_blk p in
  Stmt.Proc {node = {loc}; export; id; params; rty; body}, p

and parse_stmt_vardecl p =
  let loc, p = need_loc Keyword p in
  let id, p  = need Identifier p in
  let p      = expect Colon p in
  let ty, p  = parse_type p in
  let p      = expect Equals p in
  let e, p   = parse_expr p in
  let p      = expect Semicolon p in
  Stmt.Vardecl {node = {loc}; id; ty; e}, p

and parse_stmt_if p =
  let loc, p   = needkw Kwds.kwd_if p in
  let cond, p  = parse_expr p in
  let then_, p = parse_stmt p in
  let else_, p = match p.tokens with
    | {lx = "else"} :: tl ->
       let else_, p = parse_stmt {p with tokens = tl} in
       Some else_, p
    | _ -> None, p in
  Stmt.If {node = {loc = loc.loc}; cond; then_; else_}, p

and parse_stmt_expr p =
  let e, p = parse_expr p in
  Stmt.Expr {node = {loc = Expr.get_loc e}; e}, p

and parse_stmt_kw p =
  match p.tokens with
  | [] -> Err.fatal "parse_stmt_kw: out of tokens\n"
  | {lx = "if"; _} :: _ -> parse_stmt_if p
  | {lx = "let"; _} :: _ -> parse_stmt_vardecl p
  | {lx = "proc"; _} :: _ -> parse_stmt_proc p
  | _ -> Err.fatal "parse_stmt_kw: invalid keyword\n"

and parse_stmt p =
  match p.tokens with
  | []                    -> Err.fatal "parse_stmt: out of tokens\n"
  | {k = Keyword; _} :: _ -> parse_stmt_kw p
  | {k = L_Curly; _} :: _ -> parse_stmt_blk p
  | _                     -> parse_stmt_expr p

let parse tokens : t =
  let p = {tokens; stmts = []} in
  let rec aux = function
    | {tokens = []; _} as p -> p
    | p ->
       let s, p = parse_stmt p in
       aux {p with stmts = p.stmts @ [s]}
  in aux p

let dump (p : t) : unit =
  let indent = ref 0 in
  let line s = print_endline @@ String.make (!indent * 2) ' ' ^ s in
  let with_indent f =
    indent := !indent + 1;
    f ();
    indent := !indent - 1
  in
  let rec v =
    { Visitor.context = indent
    ; expr_integer = (fun _ e -> line @@ "Integer " ^ e.i.lx)
    ; expr_string = (fun _ e -> line @@ "String " ^ e.s.lx)
    ; expr_identifier = (fun _ e -> line @@ "Identifier " ^ e.id.lx)
    ; expr_binary = (fun v e ->
      line @@ "Binary " ^ e.op.lx;
      with_indent @@ fun () ->
                     Visitor.accept_expr v e.lhs;
                     Visitor.accept_expr v e.rhs)
    ; stmt_expr = (fun v s -> Visitor.accept_expr v s.e)
    ; stmt_vardecl = (fun v s ->
      line @@ "Vardecl " ^ s.id.lx ^ ": " ^ Type.to_string s.ty;
      with_indent @@ fun () -> Visitor.accept_expr v s.e)
    ; stmt_proc = (fun v s ->
      let params = List.map (fun (id, ty) -> id.lx ^ ": " ^ Type.to_string ty) s.params in
      line @@ (if s.export then "Export " else "") ^ "Proc " ^ s.id.lx ^
                "(" ^ String.concat ", " params ^ "): " ^ Type.to_string s.rty;
      with_indent @@ fun () -> Visitor.accept_stmt v s.body)
    ; stmt_blk = (fun v s ->
      line "Block";
      with_indent @@ fun () -> List.iter (Visitor.accept_stmt v) s.stmts)
    ; stmt_if = (fun v s ->
      line "If";
      with_indent @@ fun () ->
        line "Condition";
        with_indent @@ fun () -> Visitor.accept_expr v s.cond;
        line "Then";
        with_indent @@ fun () -> Visitor.accept_stmt v s.then_)
    }
  in
  List.iter (Visitor.accept_stmt v) p.stmts
