module String_Map = Map.Make (String)

type variable =
  { id : Token.t
  ; ty : Type.t
  }

type scope =
  { layers : variable String_Map.t list
  }

let scope_push s =
  { layers = String_Map.empty :: s.layers
  }

let scope_pop s =
  match s.layers with
  | [] -> Err.fatal "scope_pop: scope is empty"
  | _ :: tl -> {layers = tl}

let name_resolution =
  let s = {layers = [String_Map.empty]} in
  ()

let type_check = failwith "unimplemented"


