type t =
  { r : int
  ; c : int
  ; path : string
  }

let from r c path = {r; c; path}

let to_string l =
  "Location: { path = " ^ l.path ^ ", r = " ^ (string_of_int l.r) ^ ", c = " ^ (string_of_int l.c) ^ " }"

let fmt l = Printf.sprintf "%s:%d:%d:" l.path l.r l.c
