type kind =
  | Eof
  | Identifier
  | Keyword of Keyword.t
  | Type
  | Integer_Literal
  | String_Literal
  | L_Paren
  | R_Paren
  | L_Curly
  | R_Curly
  | L_Sqr
  | R_Sqr
  | Comma
  | Colon
  | Semicolon
  | Plus
  | Minus
  | Asterisk
  | Forward_Slash
  | Plus_Equals
  | Minus_Equals
  | Asterisk_Equals
  | Forward_Slash_Equals
  | Bang
  | Equals
  | Bang_Equals
  | Double_Equals
  | Greaterthan_Equals
  | Lessthan_Equals
  | Pipe
  | Double_Pipe
  | Ampersand
  | Double_Ampersand
  | Pipe_Equals
  | Ampersand_Equals
  | Lessthan
  | Greaterthan
  | Uptick
  | Uptick_Equals
  | Percent
  | Percent_Equals

type t =
  { lx : string
  ; k : kind
  ; l : Location.t
  }

let rec kind_to_string = function
  | Eof -> "Eof"
  | Identifier -> "Identifier"
  | Keyword k -> "Keyword(" ^ Keyword.to_string k ^ ")"
  | Type -> "Type"
  | Integer_Literal -> "Integer_Literal"
  | String_Literal -> "String_Literal"
  | L_Paren -> "L_Paren"
  | R_Paren -> "R_Paren"
  | L_Curly -> "L_Curly"
  | R_Curly -> "R_Curly"
  | L_Sqr -> "L_Sqr"
  | R_Sqr -> "R_Sqr"
  | Comma -> "Comma"
  | Colon -> "Colon"
  | Semicolon -> "Semicolon"
  | Plus -> "Plus"
  | Minus -> "Minus"
  | Asterisk -> "Asterisk"
  | Forward_Slash -> "Forward_Slash"
  | Plus_Equals -> "Plus_Equals"
  | Minus_Equals -> "Minus_Equals"
  | Asterisk_Equals -> "Asterisk_Equals"
  | Forward_Slash_Equals -> "Forward_Slash_Equals"
  | Bang -> "Bang"
  | Equals -> "Equals"
  | Bang_Equals -> "Bang_Equals"
  | Double_Equals -> "Double_Equals"
  | Greaterthan_Equals -> "Greaterthan_Equals"
  | Lessthan_Equals -> "Lessthan_Equals"
  | Pipe -> "Pipe"
  | Double_Pipe -> "Double_Pipe"
  | Ampersand -> "Ampersand"
  | Double_Ampersand -> "Double_Ampersand"
  | Pipe_Equals -> "Pipe_Equals"
  | Ampersand_Equals -> "Ampersand_Equals"
  | Lessthan -> "Lessthan"
  | Greaterthan -> "Greaterthan"
  | Uptick -> "Uptick"
  | Uptick_Equals -> "Uptick_Equals"
  | Percent -> "Percent"
  | Percent_Equals -> "Percent_Equals"



