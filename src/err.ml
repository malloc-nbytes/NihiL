exception Expect of string * string
exception Out_Of_Tokens
exception Illegal_Statement of string
exception Incompatible_Types of string * string
exception Identifier_Already_Defined of string
exception Identifier_Not_Defined of string

let fatal fmt =
  Printf.kfprintf (fun oc -> flush oc; exit 1) stderr fmt
