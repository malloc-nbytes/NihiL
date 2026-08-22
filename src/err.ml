let fatal fmt =
  Printf.kfprintf (fun oc -> flush oc; exit 1) stderr fmt
