type t =
  | Proc
  | Let
  | If
  | Else
  | Export

let of_string = function
  | "proc" -> Some Proc
  | "let" -> Some Let
  | "if" -> Some If
  | "else" -> Some Else
  | "export" -> Some Export
  | _ -> None

let to_string = function
  | Proc -> "proc"
  | Let -> "let"
  | If -> "if"
  | Else -> "else"
  | Export -> "export"
