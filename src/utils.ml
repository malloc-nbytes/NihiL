let list_of_string s = s |> String.to_seq |> List.of_seq

let string_of_char c = String.make 1 c

let unwrap = function
  | Some k -> k
  | _ -> failwith "unwrap: tried to unwrap a None value"
