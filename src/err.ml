exception Expect of string * string
exception Out_Of_Tokens
exception Illegal_Statement of string

let fatal fmt =
  Printf.kfprintf (fun oc -> flush oc; exit 1) stderr fmt
