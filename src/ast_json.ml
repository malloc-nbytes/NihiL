let quote s =
  let b = Buffer.create (String.length s + 2) in
  Buffer.add_char b '"';
  String.iter
    (function
      | '"' -> Buffer.add_string b "\\\""
      | '\\' -> Buffer.add_string b "\\\\"
      | '\b' -> Buffer.add_string b "\\b"
      | '\012' -> Buffer.add_string b "\\f"
      | '\n' -> Buffer.add_string b "\\n"
      | '\r' -> Buffer.add_string b "\\r"
      | '\t' -> Buffer.add_string b "\\t"
      | c when Char.code c < 0x20 ->
         Buffer.add_string b (Printf.sprintf "\\u%04x" (Char.code c))
      | c -> Buffer.add_char b c)
    s;
  Buffer.add_char b '"';
  Buffer.contents b

let location (loc : Location.t) =
  "{\"path\":" ^ quote loc.path ^ ",\"line\":" ^ string_of_int loc.r ^
  ",\"column\":" ^ string_of_int loc.c ^ "}"

let type_ ty = quote (Type.to_string ty)

let expr_object fields (node : Expr.node) =
  "{" ^ fields ^ ",\"location\":" ^ location node.loc ^ ",\"type\":" ^
  type_ node.ty ^ "}"

let rec expr = function
  | Expr.Integer e ->
     expr_object ("\"kind\":\"integer\",\"value\":" ^ quote e.i.lx) e.node
  | Expr.String e ->
     expr_object ("\"kind\":\"string\",\"value\":" ^ quote e.s.lx) e.node
  | Expr.Identifier e ->
     expr_object ("\"kind\":\"identifier\",\"name\":" ^ quote e.id.lx) e.node
  | Expr.Binary e ->
     expr_object
       ("\"kind\":\"binary\",\"operator\":" ^ quote e.op.lx ^
        ",\"left\":" ^ expr e.lhs ^ ",\"right\":" ^ expr e.rhs)
       e.node
  | Expr.Unary e ->
     expr_object
       ("\"kind\":\"unary\",\"operator\":" ^ quote e.op.lx ^
        ",\"operand\":" ^ expr e.rhs)
       e.node

let rec stmt = function
  | Stmt.Proc s ->
     let parameter ((id : Token.t), ty) =
       "{\"name\":" ^ quote id.lx ^ ",\"type\":" ^ type_ ty ^ "}"
     in
     "{\"kind\":\"procedure\",\"export\":" ^ string_of_bool s.export ^
     ",\"name\":" ^ quote s.id.lx ^ ",\"parameters\":[" ^
     String.concat "," (List.map parameter s.params) ^ "],\"return_type\":" ^
     type_ s.rty ^ ",\"body\":" ^ stmt s.blk ^ ",\"location\":" ^
     location s.node.loc ^ "}"
  | Stmt.Let s ->
     "{\"kind\":\"let\",\"name\":" ^ quote s.id.lx ^ ",\"type\":" ^
     type_ s.ty ^ ",\"value\":" ^ expr s.e ^ ",\"location\":" ^
     location s.node.loc ^ "}"
  | Stmt.Expr s ->
     "{\"kind\":\"expression\",\"expression\":" ^ expr s.e ^
     ",\"location\":" ^ location s.node.loc ^ "}"
  | Stmt.Block s ->
     "{\"kind\":\"block\",\"statements\":[" ^
     String.concat "," (List.map stmt s.stmts) ^ "],\"location\":" ^
     location s.node.loc ^ "}"

let to_string program =
  "{\"kind\":\"program\",\"statements\":[" ^
  String.concat "," (List.map stmt program) ^ "]}"

let print program = print_endline (to_string program)
let dump = print
