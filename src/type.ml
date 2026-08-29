type t =
  | Void
  | I32
  | U32
  | U8
  | Ptr of t

let is_primitive = function
  | "void" | "i32" | "u32" | "u8" -> true
  | _ -> false

let rec to_string = function
  | Void -> "void"
  | I32 -> "i32"
  | U32 -> "u32"
  | U8 -> "u8"
  | Ptr i -> "Ptr(" ^ to_string i ^ ")"
