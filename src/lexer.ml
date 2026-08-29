open Token

let isalpha c =
  match c with
  | 'a' .. 'z' | 'A' .. 'Z' -> true
  | _ -> false

let isdigit c =
  match c with
  | '0' .. '9' -> true
  | _ -> false

let isalnum c = isalpha c || isdigit c

let consume_while s pred =
  let rec aux acc s =
    match s with
    | [] -> acc, []
    | hd :: _  when not @@ pred hd -> acc, s
    | hd :: tl -> aux (hd :: acc) tl
  in
  let acc, rest = aux [] s in
  (List.rev acc |> List.to_seq |> String.of_seq), rest

let rec dump = function
  | [] -> ()
  | hd :: tl ->
     let _ = Printf.printf "%s\n" @@ Token.to_string hd in
     dump tl

let lex path src =
  let rec aux r c acc = function
    | [] -> List.rev acc
    | (' ' | '\t') :: tl -> aux r (c+1) acc tl
    | '\n' :: tl -> aux (r+1) 1 acc tl
    | '+' :: '=' :: tl -> aux r (c+1) ({lx = "+="; k = Plus_Equals; l = {r; c; path}} :: acc) tl
    | '-' :: '=' :: tl -> aux r (c+1) ({lx = "-="; k = Minus_Equals; l = {r; c; path}} :: acc) tl
    | '*' :: '=' :: tl -> aux r (c+1) ({lx = "*="; k = Asterisk_Equals; l = {r; c; path}} :: acc) tl
    | '/' :: '=' :: tl -> aux r (c+1) ({lx = "/="; k = Forward_Slash_Equals; l = {r; c; path}} :: acc) tl
    | '!' :: '=' :: tl -> aux r (c+1) ({lx = "!="; k = Bang_Equals; l = {r; c; path}} :: acc) tl
    | '>' :: '=' :: tl -> aux r (c+1) ({lx = ">="; k = Greaterthan_Equals; l = {r; c; path}} :: acc) tl
    | '<' :: '=' :: tl -> aux r (c+1) ({lx = "<="; k = Lessthan_Equals; l = {r; c; path}} :: acc) tl
    | '=' :: '=' :: tl -> aux r (c+1) ({lx = "=="; k = Double_Equals; l = {r; c; path}} :: acc) tl
    | '|' :: '|' :: tl -> aux r (c+1) ({lx = "||"; k = Double_Pipe; l = {r; c; path}} :: acc) tl
    | '&' :: '&' :: tl -> aux r (c+1) ({lx = "&&"; k = Double_Ampersand; l = {r; c; path}} :: acc) tl
    | '|' :: '=' :: tl -> aux r (c+1) ({lx = "|="; k = Pipe_Equals; l = {r; c; path}} :: acc) tl
    | '&' :: '=' :: tl -> aux r (c+1) ({lx = "&="; k = Ampersand_Equals; l = {r; c; path}} :: acc) tl
    | '^' :: '=' :: tl -> aux r (c+1) ({lx = "^="; k = Uptick_Equals; l = {r; c; path}} :: acc) tl
    | '%' :: '=' :: tl -> aux r (c+1) ({lx = "%="; k = Percent_Equals; l = {r; c; path}} :: acc) tl
    | '^' :: tl -> aux r (c+1) ({lx = "^"; k = Uptick; l = {r; c; path}} :: acc) tl
    | '%' :: tl -> aux r (c+1) ({lx = "%"; k = Percent; l = {r; c; path}} :: acc) tl
    | '>' :: tl -> aux r (c+1) ({lx = ">"; k = Greaterthan; l = {r; c; path}} :: acc) tl
    | '<' :: tl -> aux r (c+1) ({lx = "<"; k = Lessthan; l = {r; c; path}} :: acc) tl
    | '&' :: tl -> aux r (c+1) ({lx = "&"; k = Ampersand; l = {r; c; path}} :: acc) tl
    | '|' :: tl -> aux r (c+1) ({lx = "|"; k = Pipe; l = {r; c; path}} :: acc) tl
    | '!' :: tl -> aux r (c+1) ({lx = "!"; k = Bang; l = {r; c; path}} :: acc) tl
    | '=' :: tl -> aux r (c+1) ({lx = "="; k = Equals; l = {r; c; path}} :: acc) tl
    | '(' :: tl -> aux r (c+1) ({lx = "("; k = L_Paren; l = {r; c; path}} :: acc) tl
    | ')' :: tl -> aux r (c+1) ({lx = ")"; k = R_Paren; l = {r; c; path}} :: acc) tl
    | '{' :: tl -> aux r (c+1) ({lx = "{"; k = L_Curly; l = {r; c; path}} :: acc) tl
    | '}' :: tl -> aux r (c+1) ({lx = "}"; k = R_Curly; l = {r; c; path}} :: acc) tl
    | '[' :: tl -> aux r (c+1) ({lx = "["; k = L_Sqr; l = {r; c; path}} :: acc) tl
    | ']' :: tl -> aux r (c+1) ({lx = "]"; k = R_Sqr; l = {r; c; path}} :: acc) tl
    | ',' :: tl -> aux r (c+1) ({lx = ","; k = Comma; l = {r; c; path}} :: acc) tl
    | ':' :: tl -> aux r (c+1) ({lx = ":"; k = Colon; l = {r; c; path}} :: acc) tl
    | ';' :: tl -> aux r (c+1) ({lx = ";"; k = Semicolon; l = {r; c; path}} :: acc) tl
    | '+' :: tl -> aux r (c+1) ({lx = "+"; k = Plus; l = {r; c; path}} :: acc) tl
    | '-' :: tl -> aux r (c+1) ({lx = "-"; k = Minus; l = {r; c; path}} :: acc) tl
    | '*' :: tl -> aux r (c+1) ({lx = "*"; k = Asterisk; l = {r; c; path}} :: acc) tl
    | '/' :: tl -> aux r (c+1) ({lx = "/"; k = Forward_Slash; l = {r; c; path}} :: acc) tl
    | '"' :: tl ->
       let lx, tl = consume_while tl (fun c -> c <> '"') in
       aux r (c + 2 + String.length lx) ({lx; k = String_Literal; l = {r; c; path}} :: acc) tl
    | (hd :: tl) as l when isalpha hd || hd = '_' ->
       let lx, tl = consume_while l (fun c -> isalnum c || c = '_') in
       let k = match Keyword.of_string lx with
         | Some k -> Token.Keyword k
         | _ -> match Type.is_primitive lx with
                | true -> Token.Type
                | _ -> Identifier in
       aux r (c + String.length lx) ({lx; k; l = {r; c; path}} :: acc) tl
    | (hd :: tl) as l when isdigit hd ->
       let lx, tl = consume_while l isdigit in
       aux r (c + String.length lx) ({lx; k = Integer_Literal; l = {r; c; path}} :: acc) tl
    | hd :: _ -> failwith @@ "lexer error near `" ^ (String.make 1 hd) ^ "'"
  in aux 1 1 [] (String.to_seq src |> List.of_seq)
