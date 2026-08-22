let kwds = ["if"; "proc"; "export"]

let iskwd s =
  let rec aux = function
    | [] -> false
    | hd :: _ when hd = s -> true
    | _ :: tl -> aux tl
  in
  aux kwds
