type t =
  | Void
  | U8
  | I32
  | Ptr of ptr

and ptr =
  { inner : t
  }

let is_builtin s =
  match s with
  | "void" -> true
  | "u8" -> true
  | "i32" -> true
  | _ -> false

let size = function
  | Void -> 0
  | U8 -> 1
  | I32 -> 4
  | Ptr _ -> 8

let from_str = function
  | "void" -> Void
  | "u8" -> U8
  | "i32" -> I32
  | s -> failwith @@ Printf.sprintf "from_str: unknown type: `%s'" s
