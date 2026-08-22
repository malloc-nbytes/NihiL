let kwd_if = "if"
let kwd_let = "let"
let kwd_proc = "proc"
let kwd_export = "export"

let kwds = [kwd_if; kwd_let; kwd_proc; kwd_export]

let iskwd s =
  let rec aux = function
    | [] -> false
    | hd :: _ when hd = s -> true
    | _ :: tl -> aux tl
  in
  aux kwds
