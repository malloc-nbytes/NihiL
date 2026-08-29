type t =
  | Void
  | I32
  | U32
  | U8
  | Ptr of t
  | Proc of {
      rty : t;
      params : t list;
    }
  | Undefined

let is_primitive = function
  | "void" | "i32" | "u32" | "u8" -> true
  | _ -> false

let rec to_string = function
  | Void -> "void"
  | I32 -> "i32"
  | U32 -> "u32"
  | U8 -> "u8"
  | Ptr i -> "Ptr(" ^ to_string i ^ ")"
  | Proc p ->
     let rec aux = function
       | [] -> ""
       | hd :: tl -> (to_string hd) ^ aux tl
     in "Proc(" ^ (aux p.params) ^ "): " ^ to_string p.rty
  | Undefined -> "undefined"

let rec check t t' =
  if t <> t' then false
  else match t, t' with
       | Ptr p, Ptr p' -> check p p'
       | _ -> true
