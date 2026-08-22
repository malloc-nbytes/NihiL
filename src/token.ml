type kind =
  | Eof
  | Identifier
  | Keyword
  | String_Literal
  | Integer_Literal
  | L_Paren
  | R_Paren
  | L_Curly
  | R_Curly
  | L_Sqr
  | R_Sqr
  | Plus
  | Minus
  | Asterisk
  | Forward_Slash
  | Equals
  | Semicolon
  | Colon

type t =
  { lx  : string
  ; k   : kind
  ; loc : Location.t
  }

let kind_to_string = function
  | Eof -> "Eof"
  | Identifier -> "Identifier"
  | Keyword -> "Keyword"
  | String_Literal -> "String_Literal"
  | Integer_Literal -> "Integer_Literal"
  | L_Paren -> "L_Paren"
  | R_Paren -> "R_Paren"
  | L_Curly -> "L_Curly"
  | R_Curly -> "R_Curly"
  | L_Sqr -> "L_Sqr"
  | R_Sqr -> "R_Sqr"
  | Plus -> "Plus"
  | Minus -> "Minus"
  | Asterisk -> "Asterisk"
  | Forward_Slash -> "Forward_Slash"
  | Equals -> "Equals"
  | Semicolon -> "Semicolon"
  | Colon -> "Colon"

let from lx k r c path =
  let loc = Location.from r c path in
  {lx; k; loc}

let to_string t =
  "Token: { lx = " ^ t.lx ^ ", k = " ^ (kind_to_string t.k) ^ ", loc = " ^ (Location.to_string t.loc) ^ " }"
