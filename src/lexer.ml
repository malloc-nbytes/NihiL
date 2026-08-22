let isalpha = function
  | 'a' .. 'z' | 'A' .. 'Z' -> true
  | _ -> false

let isdigit = function
  | '0' .. '9' -> true
  | _ -> false

let consume_while s pred =
  let rec aux s acc =
    match s with
    | [] -> acc, []
    | hd :: tl when not @@ pred hd -> acc, tl
    | hd :: tl -> aux tl @@ acc ^ Utils.string_of_char hd
  in
  aux s ""

let rec dump = function
  | [] -> ()
  | hd :: tl ->
     let _ = print_endline @@ Token.to_string hd
     in dump tl

let from_file path src =
  let open Token in
  let rec aux r c = function
    | [] -> []
    | '\n' :: tl       -> []                                      @ aux (r+1) 1 tl
    | ('\t'|' ') :: tl -> []                                      @ aux r (c+1) tl
    | '(' :: tl        -> [Token.from "(" L_Paren r c path]       @ aux r (c+1) tl
    | ')' :: tl        -> [Token.from ")" R_Paren r c path]       @ aux r (c+1) tl
    | '[' :: tl        -> [Token.from "[" L_Sqr r c path]         @ aux r (c+1) tl
    | ']' :: tl        -> [Token.from "]" R_Sqr r c path]         @ aux r (c+1) tl
    | '{' :: tl        -> [Token.from "{" L_Curly r c path]       @ aux r (c+1) tl
    | '}' :: tl        -> [Token.from "}" R_Curly r c path]       @ aux r (c+1) tl
    | '+' :: tl        -> [Token.from "+" Plus r c path]          @ aux r (c+1) tl
    | '-' :: tl        -> [Token.from "-" Minus r c path]         @ aux r (c+1) tl
    | '*' :: tl        -> [Token.from "*" Asterisk r c path]      @ aux r (c+1) tl
    | '/' :: tl        -> [Token.from "/" Forward_Slash r c path] @ aux r (c+1) tl
    | '=' :: tl        -> [Token.from "=" Equals r c path]        @ aux r (c+1) tl
    | ';' :: tl        -> [Token.from ";" Semicolon r c path]     @ aux r (c+1) tl
    | ':' :: tl        -> [Token.from ":" Colon r c path]         @ aux r (c+1) tl
    | (hd :: tl) as lst when isalpha hd || hd = '_' ->
       let id, tl' = consume_while lst (fun c -> isalpha c || c = '_') in
       [Token.from id (if Kwds.iskwd id      then Keyword else
                       if Type.is_builtin id then TypeKeyword
                       else                       Identifier) r c path] @ aux r (c + String.length id) tl'
    | (hd :: tl) as lst when isdigit hd ->
       let num, tl' = consume_while lst isdigit in
       [Token.from num Integer_Literal r c path] @ aux r (c + String.length num) tl'
    | hd :: _ -> failwith @@ "unhandled lexer case: " ^ (Utils.string_of_char hd)
  in
  aux 1 1 @@ Utils.list_of_string src
