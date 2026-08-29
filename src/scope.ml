type 'v entry =
  { id : string
  ; value : 'v
  }

type 'v frame = 'v entry list

type 'v t = 'v frame list

let empty = [[]]

let rec get id = function
  | [] -> None
  | frame :: parents ->
     match List.find_opt (fun entry -> entry.id = id) frame with
     | Some entry -> Some entry.value
     | None -> get id parents

let contains id t = Option.is_some (get id t)

let push t = [] :: t

let pop = function
  | _ :: (_ :: _ as parents) -> parents
  | t -> t

let add id value = function
  | frame :: parents ->
     ({id; value} :: List.filter (fun entry -> entry.id <> id) frame) :: parents
  | [] -> [[{id; value}]]

let remove id = function
  | frame :: parents -> List.filter (fun entry -> entry.id <> id) frame :: parents
  | [] -> []
